#include "zlogger_receiver.hpp"

int main()
{
    ZLoggerZMQ zmq;
    ZLoggerParser parser;
    ZLogger logger;

    ZLoggerReceiver receiver(zmq, parser, logger);
    receiver.start();

    return 0;
}
