#ifndef __ZLOGGER_RECEIVER__
#define __ZLOGGER_RECEIVER__

#include <string>
#include "zlogger_parser.hpp"
#include "zlogger.hpp"
#include "zlogger_zmq.hpp"

using namespace std;

class ZLoggerReceiver
{
    private:
        ZLoggerZMQ* zmqHandler;
        ZLoggerParser* parser;
        ZLogger* logger;

    public:
        ZLoggerReceiver(ZLoggerZMQ& zmqHandler, ZLoggerParser& parser, ZLogger& logger) 
        {
            this->zmqHandler = &zmqHandler;
            this->parser = &parser;
            this->logger = &logger;
        }

        ~ZLoggerReceiver() {}

        void start()
        {
            cout << "ZLoggerReceiver starting..." << endl;
            this->zmqHandler->init();
            cout << "ZLoggerReceiver zmqHandler intialized" << endl;

            ZLoggerRequest *req;
            while(true)
            {
                string request = this->zmqHandler->getNext();
                cout << "ZLoggerReceiver got " << request << " from zeroMq" << endl;
                if((req = this->parser->parse(request)))
                    cout << "ZLoggerReceiver logging message" << endl;
                    this->logger->log(req);  // assync processing
            }
        }
};

#endif
