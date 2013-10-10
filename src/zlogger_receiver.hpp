#ifndef __BTLOGGER_RECEIVER__
#define __BTLOGGER_RECEIVER__

#include <string>
#include "zlogger_parser.hpp"
#include "zlogger.hpp"
#include "zlogger_zmq.hpp"

using namespace std;

class BTLoggerReceiver
{
    private:
        BTLoggerZMQ zmqHandler;
        BTLoggerParser parser;
        BTLogger logger;

    public:
        BTLoggerReceiver() {}
        ~BTLoggerReceiver() {}

        void start()
        {
            cout << "BTLoggerReceiver starting..." << endl;
            this->zmqHandler.init();
            cout << "BTLoggerReceiver zmqHandler intialized" << endl;

            BTLoggerRequest *req;
            while(true)
            {
                string request = this->zmqHandler.getNext();
                cout << "BTLoggerReceiver got " << request << " from zeroMq" << endl;
                if((req = this->parser.parse(request)))
                    cout << "BTLoggerReceiver logging message" << endl;
                    this->logger.log(req);  // assync processing
            }
        }
};

#endif