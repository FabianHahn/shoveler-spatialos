#ifndef SHOVELER_CLIENT_CONNECT_H
#define SHOVELER_CLIENT_CONNECT_H

#include <improbable/c_schema.h>
#include <improbable/c_worker.h>

Worker_Connection *shovelerClientConnect(int argc, char **argv, Worker_ConnectionParameters *connectionParameters);

#endif
