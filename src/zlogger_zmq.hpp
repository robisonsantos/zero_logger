#ifndef __ZLOGGER_ZMQ__
#define __ZLOGGER_ZMQ__

#include <boost/thread.hpp>
#include <string>
#include "zmq.hpp"
#include "sync_queue.hpp"


using namespace std;

class ZLoggerZMQ
{
    private:
        SynchronisedQueue<string> *requests;
        
    public:
        ZLoggerZMQ()
        {
            this->requests = new SynchronisedQueue<string>();
            cout << "ZLoggerZMQ instance created" << endl;            
        }

        ~ZLoggerZMQ()
        {
            delete this->requests;
        }

        void init()
        {
            cout << "ZLoggerZMQ initiating" << endl;
            boost::thread workerThread(boost::bind(&ZLoggerZMQ::worker, this));
            cout << "ZLoggerZMQ initiated" << endl;
        }

        string getNext()
        {
            string ret;
            if(!this->requests->tryDequeue(ret)) ret = "";
                
            cout << "ZLoggerZMQ::getNext returning: " << ret << endl;
            return ret;
        }

    private:
        void worker() 
        {
            cout << "ZLoggerZMQ::worker initiating: " << endl;
            
            SynchronisedQueue<string> *queue = this->requests;

            zmq::context_t context(1);
            zmq::socket_t server(context, ZMQ_PULL);
            server.bind("tcp://0.0.0.0:5555");

            cout << "ZLoggerZMQ bound to: tcp://0.0.0.0:5555" << endl;

            while(true)
            {
                zmq::message_t message;
                server.recv(&message);

                cout << "ZLoggerZMQ::worker got new message" << endl;
                queue->enqueue(std::string(static_cast<char*>(message.data()), message.size()));
                cout << "ZLoggerZMQ::worker message stored" << endl;
            }
        }
};

#endif