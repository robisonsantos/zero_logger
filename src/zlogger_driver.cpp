#include "zlogger_receiver.hpp"

int main()
{
    BTLoggerReceiver receiver;
    receiver.start();

    return 0;
}
