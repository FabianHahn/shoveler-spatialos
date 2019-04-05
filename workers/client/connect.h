#ifndef SHOVELER_CLIENT_CONNECT_H
#define SHOVELER_CLIENT_CONNECT_H

#include <improbable/worker.h>

worker::Option<worker::Connection> connect(int argc, char **argv, worker::ConnectionParameters connectionParameters, const worker::ComponentRegistry& components);

#endif
