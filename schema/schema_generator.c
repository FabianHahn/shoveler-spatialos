#include <shoveler/component.h>
#include <shoveler/file.h>
#include <shoveler/global.h>
#include <shoveler/log.h>
#include <shoveler/view.h>
#include <ctype.h> // toupper
#include <stdlib.h>
#include "../workers/client/schema.h"

static const char *configurationOptionTypeToString(ShovelerComponentConfigurationOptionType type)
{
	switch (type) {
		case SHOVELER_COMPONENT_CONFIGURATION_OPTION_TYPE_ENTITY_ID:
			return "EntityId";
		case SHOVELER_COMPONENT_CONFIGURATION_OPTION_TYPE_ENTITY_ID_ARRAY:
			return "list<EntityId>";
		case SHOVELER_COMPONENT_CONFIGURATION_OPTION_TYPE_FLOAT:
			return "float";
		case SHOVELER_COMPONENT_CONFIGURATION_OPTION_TYPE_BOOL:
			return "bool";
		case SHOVELER_COMPONENT_CONFIGURATION_OPTION_TYPE_INT:
			return "int32";
		case SHOVELER_COMPONENT_CONFIGURATION_OPTION_TYPE_STRING:
			return "string";
		case SHOVELER_COMPONENT_CONFIGURATION_OPTION_TYPE_VECTOR2:
			return "Vector2";
		case SHOVELER_COMPONENT_CONFIGURATION_OPTION_TYPE_VECTOR3:
			return "Vector3";
		case SHOVELER_COMPONENT_CONFIGURATION_OPTION_TYPE_VECTOR4:
			return "Vector4";
		case SHOVELER_COMPONENT_CONFIGURATION_OPTION_TYPE_BYTES:
			return "bytes";
		default:
			return "(unknown)";
	}
}

static GString *toUpperCamelCase(const char *input) {
	GString *result = g_string_new("");
	bool atStart = true;
	for (const char *c = input; *c != '\0'; c++) {
		if (atStart) {
			g_string_append_c(result, toupper(*c));
			atStart = false;
		} else if (*c == '_') {
			atStart = true;
		} else {
			g_string_append_c(result, *c);
		}
	}

	return result;
}

int main(int argc, char **argv) {
	shovelerLogInit("shoveler-spatialos/", SHOVELER_LOG_LEVEL_INFO_UP, stdout);
	shovelerGlobalInit();

	if (argc != 2) {
		shovelerLogError("Usage:\n\t%s <schema output file>", argv[0]);
		return EXIT_FAILURE;
	}
	const char *filename = argv[1];

	ShovelerView *view = shovelerViewCreate(/* updateAuthoritativeComponent */ NULL, /* user_data */ NULL);
	shovelerClientRegisterViewComponentTypes(view);

	GString *schema = g_string_new(
		"package shoveler;\n"
		"\n"
		"import \"improbable/standard_library.schema\";\n"
		"\n"
		"type Vector2 {\n"
		"\tfloat x = 1;\n"
		"\tfloat y = 2;\n"
		"}\n"
		"\n"
		"type Vector3 {\n"
		"\tfloat x = 1;\n"
		"\tfloat y = 2;\n"
		"\tfloat z = 3;\n"
		"}\n"
		"\n"
		"type Vector4 {\n"
		"\tfloat x = 1;\n"
		"\tfloat y = 2;\n"
		"\tfloat z = 3;\n"
		"\tfloat w = 4;\n"
		"}\n");

	GHashTableIter iter;
	const char *typeName;
	ShovelerComponentType *componentType;
	g_hash_table_iter_init(&iter, view->componentTypes);
	while (g_hash_table_iter_next(&iter, (gpointer *) &typeName, (gpointer *) &componentType)) {
		int componentId = shovelerClientResolveComponentSchemaId(componentType->id);
		GString *componentName = toUpperCamelCase(componentType->id);
		g_string_append_printf(schema, "component %s {\n\tid = %d;\n", componentName->str, componentId);
		g_string_free(componentName, true);

		for (int i = 0; i < componentType->numConfigurationOptions; i++) {
			ShovelerComponentTypeConfigurationOption *configurationOption = &componentType->configurationOptions[i];
			g_string_append_printf(schema, "\t%s%s%s %s = %d;\n",
				configurationOption->isOptional ? "option<" : "",
				configurationOptionTypeToString(configurationOption->type),
				configurationOption->isOptional ? ">" : "",
				configurationOption->name,
				i + 1);
		}

		g_string_append(schema, "}\n\n");
	}

	// Server components
	g_string_append(schema,
		"type ChunkRegion {\n"
		"    int32 min_x = 1;\n"
		"    int32 min_z = 2;\n"
		"    int32 size_x = 3;\n"
		"    int32 size_z = 4;\n"
		"}\n"
		"\n"
		"type CreateClientEntityRequest {\n"
		"    option<ChunkRegion> starting_chunk_region = 1;\n"
		"}\n"
		"\n"
		"type CreateClientEntityResponse {\n"
		"\n"
		"}\n"
		"\n"
		"type ClientSpawnCubeRequest {\n"
		"\tEntityId client = 1;\n"
		"\tVector3 position = 2;\n"
		"\tVector3 direction = 3;\n"
		"\tVector3 rotation = 4;\n"
		"}\n"
		"\n"
		"type ClientSpawnCubeResponse {\n"
		"\n"
		"}\n"
		"\n"
		"type DigHoleRequest {\n"
		"\tEntityId client = 1;\n"
		"\tVector3 position = 2;\n"
		"}\n"
		"\n"
		"type DigHoleResponse {\n"
		"\n"
		"}\n"
		"\n"
		"type UpdateResourceRequest {\n"
		"\tEntityId resource = 1;\n"
		"\tbytes content = 2;\n"
		"}\n"
		"\n"
		"type UpdateResourceResponse {\n"
		"\n"
		"}\n"
  		"\n"
		"/** Bootstrap component authoritative on the server worker that acts as client-facing server API. */\n"
		"component Bootstrap {\n"
		"\tid = 1334;\n"
		"\t/** Requests spawning of a client entity for the calling worker. */\n"
		"\tcommand CreateClientEntityResponse create_client_entity(CreateClientEntityRequest);\n"
		"\t/** Requests spawning of a cube entity at the calling worker's current position. */\n"
		"\tcommand ClientSpawnCubeResponse client_spawn_cube(ClientSpawnCubeRequest);\n"
		"\t/** Requests digging a hole at the player's current position. */\n"
		"\tcommand DigHoleResponse dig_hole(DigHoleRequest);\n"
		"\t/** Requests updating a resource. */\n"
		"\tcommand UpdateResourceResponse update_resource(UpdateResourceRequest);\n"
		"}\n"
		"\n"
		"/** Server-side metadata associated with each connected client. */\n"
		"component ClientInfo {\n"
		"\tid = 133742;\n"
		"\tEntityId worker_entity_id = 1;\n"
		"\tfloat color_hue = 2;\n"
		"\tfloat color_saturation = 3;\n"
		"}\n"
		"\n"
		"/** Ping component regularly updated by client. */\n"
		"component ClientHeartbeatPing {\n"
		"\tid = 13351;\n"
		"\t/** Unix timestamp with the last ping time the client sent. */\n"
		"\tint64 last_updated_time = 1;\n"
		"}\n"
		"\n"
		"/** Pong component that reflects client pings from server back to client. */\n"
		"component ClientHeartbeatPong {\n"
		"\tid = 13352;\n"
		"\t/** Unix timestamp with the last pong time the server sent. */\n"
		"\tint64 last_updated_time = 1;\n"
		"}\n");

	// Component set definitions
	g_string_append(schema,
		"\n"
		"component_set ServerBootstrapAuthority {\n"
		"    id = 1337;\n"
		"    components = [\n"
		"        Bootstrap\n"
		"    ];\n"
		"}\n"
		"\n"
		"component_set ServerAssetAuthority {\n"
		"    id = 1338;\n"
		"    components = [\n"
		"        Resource,\n"
		"        TilemapTiles\n"
		"    ];\n"
		"}\n"
		"\n"
		"component_set ServerPlayerAuthority {\n"
		"    id = 1339;\n"
		"    components = [\n"
		"        ClientHeartbeatPong,\n"
		"        ClientInfo\n"
		"    ];\n"
		"}\n"
		"\n"
		"component_set ClientPlayerAuthority {\n"
		"    id = 1340;\n"
		"    components = [\n"
		"        improbable.Position,\n"
		"        improbable.Interest,\n"
		"        Client,\n"
		"        ClientHeartbeatPing,\n"
		"        Position\n"
		"    ];\n"
		"}\n"
		"\n"
		"component_set ClientPlayerSpatialInterest {\n"
		"    id = 1341;\n"
		"    components = [\n"
		"        Light,\n"
		"        Model,\n"
		"        Sprite,\n"
		"        TilemapTiles\n"
		"    ];\n"
		"}");

	shovelerViewFree(view);

	bool written = shovelerFileWriteString(filename, schema->str);
	g_string_free(schema, true);
	if (!written) {
		shovelerLogError("Failed to write generated schema.");
		return EXIT_FAILURE;
	}

	shovelerLogInfo("Successfully wrote generated schema.");

	shovelerGlobalUninit();
	shovelerLogTerminate();

	return EXIT_SUCCESS;
}
