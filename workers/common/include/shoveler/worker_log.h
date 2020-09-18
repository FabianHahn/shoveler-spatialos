#ifndef SHOVELER_WORKER_COMMON_WORKER_LOG_H
#define SHOVELER_WORKER_COMMON_WORKER_LOG_H

#include <improbable/c_worker.h>

void shovelerWorkerOnLogMessage(void *user_data, const Worker_LogData *message);

#endif
