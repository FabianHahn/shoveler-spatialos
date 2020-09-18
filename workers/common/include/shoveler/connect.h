#ifndef SHOVELER_WORKER_COMMON_CONNECT_H
#define SHOVELER_WORKER_COMMON_CONNECT_H

#include <improbable/c_schema.h>
#include <improbable/c_worker.h>

Worker_Connection *shovelerWorkerConnect(int argc, char **argv, int argumentOffset, Worker_ConnectionParameters *connectionParameters);

#endif
