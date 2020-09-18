#include "shoveler/worker_log.h"

#include <improbable/c_worker.h>
#include <shoveler/log.h>

void shovelerWorkerOnLogMessage(void *user_data, const Worker_LogData *message)
{
	switch(message->log_level) {
		case WORKER_LOG_LEVEL_DEBUG:
			shovelerLogTrace("[Worker SDK] %s", message->content);
			break;
		case WORKER_LOG_LEVEL_INFO:
			shovelerLogInfo("[Worker SDK] %s", message->content);
			break;
		case WORKER_LOG_LEVEL_WARN:
			shovelerLogWarning("[Worker SDK] %s", message->content);
			break;
		case WORKER_LOG_LEVEL_ERROR:
			shovelerLogError("[Worker SDK] %s", message->content);
			break;
		case WORKER_LOG_LEVEL_FATAL:
			shovelerLogError("[Worker SDK] [FATAL] %s", message->content);
			break;
	}
}
