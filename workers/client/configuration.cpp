#include <sstream>
#include <string>
#include <shoveler/game.h>

#include "configuration.h"

extern "C" {
#include <shoveler/log.h>
}

using worker::Connection;
using worker::Option;

static void parseFloatFlag(Connection& connection, std::string flagName, float& target);
static void parseVector3Flag(Connection& connection, std::string flagName, ShovelerVector3& target);
static void parseBoolFlag(Connection& connection, std::string flagName, bool& target);
static void parseCoordinateMappingFlag(Connection& connection, std::string flagName, ShovelerCoordinateMapping& target);

ClientConfiguration getClientConfiguration(Connection& connection)
{
	ClientConfiguration clientConfiguration;
	clientConfiguration.controllerSettings.frame.position = shovelerVector3(0, 0, 0);
	clientConfiguration.controllerSettings.frame.direction = shovelerVector3(0, 0, 1);
	clientConfiguration.controllerSettings.frame.up = shovelerVector3(0, 1, 0);
	clientConfiguration.controllerSettings.moveFactor = 2.0f;
	clientConfiguration.controllerSettings.tiltFactor = 0.0005f;
	clientConfiguration.controllerSettings.boundingBoxSize2 = 0.0f;
	clientConfiguration.controllerSettings.boundingBoxSize3 = 0.0f;
	clientConfiguration.positionMappingX = SHOVELER_COORDINATE_MAPPING_POSITIVE_X;
	clientConfiguration.positionMappingY = SHOVELER_COORDINATE_MAPPING_POSITIVE_Y;
	clientConfiguration.positionMappingZ = SHOVELER_COORDINATE_MAPPING_POSITIVE_Z;
	clientConfiguration.hidePlayerClientEntityModel = true;

	parseVector3Flag(connection, "controller_frame_position", clientConfiguration.controllerSettings.frame.position);
	parseVector3Flag(connection, "controller_frame_direction", clientConfiguration.controllerSettings.frame.direction);
	parseVector3Flag(connection, "controller_frame_up", clientConfiguration.controllerSettings.frame.up);
	parseFloatFlag(connection, "controller_move_factor", clientConfiguration.controllerSettings.moveFactor);
	parseFloatFlag(connection, "controller_tilt_factor", clientConfiguration.controllerSettings.tiltFactor);
	parseFloatFlag(connection, "controller_bounding_box_2d_size", clientConfiguration.controllerSettings.boundingBoxSize2);
	parseFloatFlag(connection, "controller_bounding_box_3d_size", clientConfiguration.controllerSettings.boundingBoxSize3);
	parseCoordinateMappingFlag(connection, "position_mapping_x", clientConfiguration.positionMappingX);
	parseCoordinateMappingFlag(connection, "position_mapping_y", clientConfiguration.positionMappingY);
	parseCoordinateMappingFlag(connection, "position_mapping_z", clientConfiguration.positionMappingZ);
	parseBoolFlag(connection, "hide_player_client_entity_model", clientConfiguration.hidePlayerClientEntityModel);

	return clientConfiguration;
}

static void parseFloatFlag(Connection& connection, std::string flagName, float& target)
{
	const Option<std::string> &flagOption = connection.GetWorkerFlag(flagName);
	if(!flagOption) {
		return;
	}

	std::istringstream flagStream(*flagOption);
	float value;
	if(!(flagStream >> value)) {
		return;
	}

	target = value;
	shovelerLogInfo("Parsed client configuration flag '%s' with value %f.", flagName.c_str(), value);
}

static void parseVector3Flag(Connection& connection, std::string flagName, ShovelerVector3& target)
{
	const Option<std::string> &flagOption = connection.GetWorkerFlag(flagName);
	if(!flagOption) {
		return;
	}

	std::istringstream flagStream(*flagOption);
	float x, y, z;
	if(!(flagStream >> x >> y >> z)) {
		return;
	}

	target = shovelerVector3(x, y, z);
	shovelerLogInfo("Parsed client configuration flag '%s' with value (%f, %f, %f).", flagName.c_str(), x, y, z);
}

static void parseBoolFlag(Connection& connection, std::string flagName, bool& target)
{
	const Option<std::string> &flagOption = connection.GetWorkerFlag(flagName);
	if(!flagOption) {
		return;
	}

	std::istringstream flagStream(*flagOption);
	bool value;
	if(!(flagStream >> std::boolalpha >> value)) {
		return;
	}

	target = value;
	shovelerLogInfo("Parsed client configuration flag '%s' with value %s.", flagName.c_str(), value ? "true" : "false");
}

static void parseCoordinateMappingFlag(Connection& connection, std::string flagName, ShovelerCoordinateMapping& target)
{
	const Option<std::string> &flagOption = connection.GetWorkerFlag(flagName);
	if(!flagOption) {
		return;
	}

	std::string value = *flagOption;
	if(value == "+x") {
		target = SHOVELER_COORDINATE_MAPPING_POSITIVE_X;
	} else if(value == "+y") {
		target = SHOVELER_COORDINATE_MAPPING_POSITIVE_Y;
	} else if(value == "+z") {
		target = SHOVELER_COORDINATE_MAPPING_POSITIVE_Z;
	} else if(value == "-x") {
		target = SHOVELER_COORDINATE_MAPPING_NEGATIVE_X;
	} else if(value == "-y") {
		target = SHOVELER_COORDINATE_MAPPING_NEGATIVE_Y;
	} else if(value == "-z") {
		target = SHOVELER_COORDINATE_MAPPING_NEGATIVE_Z;
	} else {
		return;
	}

	shovelerLogInfo("Parsed client configuration flag '%s' with value %s.", flagName.c_str(), value.c_str());
}
