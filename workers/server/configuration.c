#include "configuration.h"

bool shovelerServerGetWorkerConfiguration(Worker_Connection *connection, ShovelerServerConfiguration *outputServerConfiguration)
{
	outputServerConfiguration->gameType = SHOVELER_WORKER_GAME_TYPE_LIGHTS;

	shovelerWorkerConfigurationParseGameTypeFlag(connection, "game_type", &outputServerConfiguration->gameType);

	return true;
}
