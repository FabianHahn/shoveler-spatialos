#include <assert.h> // assert
#include <errno.h> // errno
#include <inttypes.h> // PRIu32 PRId64
#include <stdlib.h> // srand malloc free
#include <string.h> // strerror
#include <time.h> // time

#include <glib.h>
#include <improbable/c_schema.h>
#include <improbable/c_worker.h>
#include <shoveler/connect.h>
#include <shoveler/file.h>
#include <shoveler/image/png.h>
#include <shoveler/image.h>
#include <shoveler/log.h>
#include <shoveler/schema.h>
#include <shoveler/types.h>
#include <shoveler/worker_log.h>

static const long long int bootstrapEntityId = 1;

typedef struct {
	Worker_Connection *connection;
	bool disconnected;
} UpdaterContext;

static Worker_RequestId updateResource(UpdaterContext *context, int64_t entityId, const unsigned char *content, size_t contentSize);
static GString *getImageData(ShovelerImage *image);

int main(int argc, char **argv)
{
	srand(time(NULL));

	shovelerLogInit("shoveler-spatialos/", SHOVELER_LOG_LEVEL_INFO_UP, stdout);

	if (argc != 1 && argc != 2 && argc != 4) {
		shovelerLogError("Usage:\n\t%s\n\t%s <launcher link>\n\t%s <worker ID> <hostname> <port>", argv[0], argv[0]);
		return EXIT_FAILURE;
	}

	Worker_ComponentVtable componentVtable = {0};

	Worker_LogsinkParameters logsink;
	logsink.logsink_type = WORKER_LOGSINK_TYPE_CALLBACK;
	logsink.filter_parameters.categories = WORKER_LOG_CATEGORY_NETWORK_STATUS | WORKER_LOG_CATEGORY_LOGIN;
	logsink.filter_parameters.level = WORKER_LOG_LEVEL_INFO;
	logsink.filter_parameters.callback = NULL;
	logsink.filter_parameters.user_data = NULL;
	logsink.log_callback_parameters.log_callback = shovelerWorkerOnLogMessage;
	logsink.log_callback_parameters.user_data = NULL;

	Worker_ConnectionParameters connectionParameters = Worker_DefaultConnectionParameters();
	connectionParameters.worker_type = "ShovelerUpdater";
	connectionParameters.network.connection_type = WORKER_NETWORK_CONNECTION_TYPE_MODULAR_TCP;
	connectionParameters.network.modular_tcp.security_type = WORKER_NETWORK_SECURITY_TYPE_INSECURE;
	connectionParameters.default_component_vtable = &componentVtable;
	connectionParameters.logsink_count = 1;
	connectionParameters.logsinks = &logsink;
	connectionParameters.enable_logging_at_startup = true;

	Worker_Connection *connection = shovelerWorkerConnect(argc, argv, /* argumentOffset */ 0, &connectionParameters);
	assert(connection != NULL);
	if(!Worker_Connection_IsConnected(connection)) {
		shovelerLogError("Failed to connect to SpatialOS deployment: %s", Worker_Connection_GetConnectionStatusDetailString(connection));
		Worker_Connection_Destroy(connection);
		return EXIT_FAILURE;
	}
	shovelerLogInfo("Connected to SpatialOS deployment!");

	UpdaterContext context;
	context.connection = connection;
	context.disconnected = false;

	while(!context.disconnected) {
		Worker_OpList *opList = Worker_Connection_GetOpList(connection, /* timeout_millis */ 0);
		for(size_t i = 0; i < opList->op_count; ++i) {
			Worker_Op *op = &opList->ops[i];

			switch(op->op_type) {
				case WORKER_OP_TYPE_DISCONNECT:
					shovelerLogInfo("Disconnected from SpatialOS with code %d: %s", op->op.disconnect.connection_status_code, op->op.disconnect.reason);
					context.disconnected = true;
					break;
				case WORKER_OP_TYPE_METRICS:
					Worker_Connection_SendMetrics(connection, &op->op.metrics.metrics);
					break;
				default:
					// ignore
					break;
			}
		}
		Worker_OpList_Destroy(opList);

		shovelerLogInfo("Available commands:\n\timage <entity id> <filename>\n\tanimation <entity id> <shift amount> <filename>");

		char command[16];
		int ret = scanf("%15[^ \t\n]%*c", command);
		if (ret == EOF) {
			context.disconnected = true;
			break;
		}

		if (strncmp(command, "image", strlen("image")) == 0) {
			int64_t resourceEntityId;
			scanf("%"PRId64"%*c", &resourceEntityId);

			char filename[256];
			scanf("%255[^\n]%*c", filename);

			unsigned char *content;
			size_t contentSize;
			if(!shovelerFileRead(filename, &content, &contentSize)) {
				shovelerLogError("Failed to read resource data from file '%s' for resource update operation.", filename);
				context.disconnected = true;
				break;
			}

			Worker_RequestId updateResourceRequestId = updateResource(&context, resourceEntityId, content, contentSize);
			if (updateResourceRequestId < 0) {
				shovelerLogWarning("Failed to send update resource command.");
			} else {
				shovelerLogInfo("Sent update resource request %lld for entity %"PRId64" with contents of '%s'.", updateResourceRequestId, resourceEntityId, filename);
			}

			free(content);
		} else if (strncmp(command, "animation", strlen("animation")) == 0) {
			int64_t resourceEntityId;
			scanf("%"PRId64"%*c", &resourceEntityId);

			int shiftAmount;
			scanf("%d%*c", &shiftAmount);

			char filename[256];
			scanf("%255[^\n]%*c", filename);

			ShovelerImage *characterPngImage = shovelerImagePngReadFile(filename);
			if(characterPngImage == NULL) {
				shovelerLogError("Failed to read character png image data from file '%s' for resource animation update operation.", filename);
				context.disconnected = true;
				break;
			}

			ShovelerImage *characterAnimationTilesetImage = shovelerImageCreateAnimationTileset(characterPngImage, shiftAmount);
			GString *characterAnimationTilesetPngData = getImageData(characterAnimationTilesetImage);

			Worker_RequestId updateResourceRequestId = updateResource(&context, resourceEntityId, (unsigned char *) characterAnimationTilesetPngData->str, characterAnimationTilesetPngData->len);
			if (updateResourceRequestId < 0) {
				shovelerLogWarning("Failed to send update resource command.");
			} else {
				shovelerLogInfo("Sent update resource request %lld for entity %"PRId64" with contents of '%s'.", updateResourceRequestId, resourceEntityId, filename);
			}

			g_string_free(characterAnimationTilesetPngData, true);
			shovelerImageFree(characterPngImage);
			shovelerImageFree(characterAnimationTilesetImage);
		}
	}
	shovelerLogInfo("Exiting main loop, goodbye.");

	Worker_Connection_Destroy(connection);
	shovelerLogTerminate();

	return EXIT_SUCCESS;
}

static Worker_RequestId updateResource(UpdaterContext *context, int64_t entityId, const unsigned char *content, size_t contentSize)
{
	Worker_CommandRequest updateResourceCommandRequest;
	memset(&updateResourceCommandRequest, 0, sizeof(Worker_CommandRequest));
	updateResourceCommandRequest.component_id = shovelerWorkerSchemaComponentIdBootstrap;
	updateResourceCommandRequest.command_index = shovelerWorkerSchemaBootstrapCommandIdUpdateResource;
	updateResourceCommandRequest.schema_type = Schema_CreateCommandRequest();

	Schema_Object *updateResourceRequest = Schema_GetCommandRequestObject(updateResourceCommandRequest.schema_type);
	Schema_AddEntityId(updateResourceRequest, shovelerWorkerSchemaUpdateResourceRequestFieldIdResource, entityId);
	uint8_t *contentBuffer = Schema_AllocateBuffer(updateResourceRequest, contentSize);
	memcpy(contentBuffer, content, contentSize);
	Schema_AddBytes(updateResourceRequest, shovelerWorkerSchemaUpdateResourceRequestFieldIdContent, contentBuffer, contentSize);

	return Worker_Connection_SendCommandRequest(
		context->connection,
		bootstrapEntityId,
		&updateResourceCommandRequest,
		/* timeout_millis */ NULL,
		/* command_parameters */ NULL);
}

static GString *getImageData(ShovelerImage *image)
{
	const char *tempImageFilename = "temp.png";
	shovelerImagePngWriteFile(image, tempImageFilename);

	unsigned char *contents;
	size_t contentsSize;
	shovelerFileRead(tempImageFilename, &contents, &contentsSize);

	GString *data = g_string_new("");
	g_string_append_len(data, (gchar *) contents, contentsSize);
	free(contents);

	return data;
}
