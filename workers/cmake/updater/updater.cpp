#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <map>
#include <unordered_set>

#include <improbable/standard_library.h>
#include <improbable/view.h>
#include <improbable/worker.h>
#include <shoveler.h>

#include "../client/connect.h"

extern "C" {
#include <glib.h>

#include <shoveler/image/png.h>
#include <shoveler/file.h>
#include <shoveler/image.h>
#include <shoveler/log.h>
}

using shoveler::Bootstrap;
using worker::Connection;
using worker::ConnectionParameters;
using worker::EntityId;
using worker::OutgoingCommandRequest;
using worker::RequestId;

using UpdateResource = Bootstrap::Commands::UpdateResource;

static GString *getImageData(ShovelerImage *image);

int main(int argc, char **argv) {
	srand(time(NULL));

	shovelerLogInit("shoveler-spatialos/workers/cmake/", SHOVELER_LOG_LEVEL_INFO_UP, stdout);

	if (argc != 1 && argc != 2 && argc != 4) {
		shovelerLogError("Usage:\n\t%s\n\t%s <launcher link>\n\t%s <worker ID> <hostname> <port>", argv[0], argv[0]);
		return EXIT_FAILURE;
	}


	const worker::ComponentRegistry& components = worker::Components<
			Bootstrap>{};

	worker::ConnectionParameters parameters;
	parameters.WorkerType = "ShovelerAssetUpdater";
	parameters.Network.ConnectionType = worker::NetworkConnectionType::kTcp;

	worker::Option<Connection> connectionOption = connect(argc, argv, parameters, components);
	if(!connectionOption || !connectionOption->IsConnected()) {
		return EXIT_FAILURE;
	}
	Connection& connection = *connectionOption;
	shovelerLogInfo("Connected to SpatialOS deployment!");

	worker::Dispatcher dispatcher{components};
	bool disconnected = false;
	bool requestsSent = false;
	EntityId bootstrapEntityId = 0;

	dispatcher.OnDisconnect([&](const worker::DisconnectOp& op) {
		shovelerLogError("Disconnected from SpatialOS: %s", op.Reason.c_str());
		disconnected = true;
	});

	dispatcher.OnMetrics([&](const worker::MetricsOp& op) {
		auto metrics = op.Metrics;
		connection.SendMetrics(metrics);
	});

	dispatcher.OnLogMessage([&](const worker::LogMessageOp& op) {
		switch(op.Level) {
			case worker::LogLevel::kDebug:
				shovelerLogTrace(op.Message.c_str());
				break;
			case worker::LogLevel::kInfo:
				shovelerLogInfo(op.Message.c_str());
				break;
			case worker::LogLevel::kWarn:
				shovelerLogWarning(op.Message.c_str());
				break;
			case worker::LogLevel::kError:
				shovelerLogError(op.Message.c_str());
				break;
			case worker::LogLevel::kFatal:
				shovelerLogError(op.Message.c_str());
				std::terminate();
			default:
				break;
		}
	});

	worker::query::EntityQuery bootstrapEntityQuery{worker::query::ComponentConstraint{Bootstrap::ComponentId}, worker::query::SnapshotResultType{{{Bootstrap::ComponentId}}}};
	RequestId<worker::EntityQueryRequest> bootstrapQueryRequestId = connection.SendEntityQueryRequest(bootstrapEntityQuery, {});

	std::unordered_set<RequestId<OutgoingCommandRequest<UpdateResource>>> updateRequests;

	dispatcher.OnEntityQueryResponse([&](const worker::EntityQueryResponseOp& op) {
		shovelerLogInfo("Received entity query response for request %u with status code %d.", op.RequestId.Id, op.StatusCode);
		if(op.RequestId == bootstrapQueryRequestId && op.Result.size() > 0) {
			bootstrapEntityId = op.Result.begin()->first;
			shovelerLogInfo("Received bootstrap query response containing entity %lld.", bootstrapEntityId);
			shovelerLogInfo("Available commands:\n\tresource <entity id> <filename>\n\tresource_animation <entity id> <filename> <shift amount>");

			std::string operation;
			while(std::cin >> operation) {
				if(operation == "resource") {
					EntityId resourceEntityId;
					if(!(std::cin >> resourceEntityId)) {
						shovelerLogError("Unexpected end of input when reading resource entity ID for resource update operation.");
						disconnected = true;
						break;
					}

					std::string filename;
					if(!(std::cin >> filename)) {
						shovelerLogError("Unexpected end of input when reading filename for resource update operation.");
						disconnected = true;
						break;
					}

					unsigned char *contents;
					size_t contentsSize;
					if(!shovelerFileRead(filename.c_str(), &contents, &contentsSize)) {
						shovelerLogError("Failed to read resource data from file '%s' for resource update operation.", filename.c_str());
						disconnected = true;
						break;
					}
					std::string contentsString{(char *) contents, contentsSize};
					free(contents);

					updateRequests.insert(connection.SendCommandRequest<UpdateResource>(bootstrapEntityId, {resourceEntityId, "image/png", contentsString}, {}));
					shovelerLogInfo("Sent resource update request for entity %lld (%zu bytes).", resourceEntityId, contentsString.size());
				} else if(operation == "resource_animation") {
					EntityId resourceEntityId;
					if(!(std::cin >> resourceEntityId)) {
						shovelerLogError("Unexpected end of input when reading resource entity ID for resource animation update operation.");
						disconnected = true;
						break;
					}

					std::string filename;
					if(!(std::cin >> filename)) {
						shovelerLogError("Unexpected end of input when reading filename for resource animation update operation.");
						disconnected = true;
						break;
					}

					int characterShiftAmount;
					if(!(std::cin >> characterShiftAmount)) {
						shovelerLogError("Unexpected end of input when reading character shift amount for resource animation update operation.");
						disconnected = true;
						break;
					}

					ShovelerImage *characterPngImage = shovelerImagePngReadFile(filename.c_str());
					if(characterPngImage == NULL) {
						shovelerLogError("Failed to read character png image data from file '%s' for resource animation update operation.", filename.c_str());
						disconnected = true;
						break;
					}

					ShovelerImage *characterAnimationTilesetImage = shovelerImageCreateAnimationTileset(characterPngImage, characterShiftAmount);
					GString *characterAnimationTilesetPngData = getImageData(characterAnimationTilesetImage);
					std::string contentsString{characterAnimationTilesetPngData->str, characterAnimationTilesetPngData->len};
					g_string_free(characterAnimationTilesetPngData, true);
					shovelerImageFree(characterPngImage);
					shovelerImageFree(characterAnimationTilesetImage);

					updateRequests.insert(connection.SendCommandRequest<UpdateResource>(bootstrapEntityId, {resourceEntityId, "image/png", contentsString}, {}));
					shovelerLogInfo("Sent resource animation update request for entity %lld (%zu bytes).", resourceEntityId, contentsString.size());
				} else {
					shovelerLogError("Unsupported update operation '%s'.", operation.c_str());
					disconnected = true;
					break;
				}
			}
			requestsSent = true;
		}
	});

	dispatcher.OnCommandResponse<UpdateResource>([&](const worker::CommandResponseOp<UpdateResource>& op) {
		shovelerLogInfo("Received update resource command response %u with status code %d: %s", op.RequestId.Id, op.StatusCode, op.Message.c_str());
		updateRequests.erase(op.RequestId);

		if(updateRequests.empty()) {
			shovelerLogInfo("Received responses for all command requests, terminating.");
			disconnected = true;
		}
	});

	while(!disconnected) {
		dispatcher.Process(connection.GetOpList(0));
	}
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
