#ifndef __BTLOGGER_ZMQ__
#define __BTLOGGER_ZMQ__

#include <boost/thread.hpp>
#include <string>
#include "zmq.hpp"
#include "sync_queue.hpp"


using namespace std;

class BTLoggerZMQ
{
    private:
        SynchronisedQueue<string> *requests;
        
    public:
        BTLoggerZMQ()
        {
            this->requests = new SynchronisedQueue<string>();
            cout << "BTLoggerZMQ instance created" << endl;            
        }

        ~BTLoggerZMQ()
        {
            delete this->requests;
        }

        void init()
        {
            cout << "BTLoggerZMQ initiating" << endl;
            boost::thread workerThread(boost::bind(&worker, this->requests));
            cout << "BTLoggerZMQ initiated" << endl;
        }

        string getNext()
        {
            string ret;
            if(!this->requests->tryDequeue(ret)) ret = "";
                
            cout << "BTLoggerZMQ::getNext returning: " << ret << endl;
            return ret;
        }

    private:
        static void worker(SynchronisedQueue<string> *queue) 
        {
            cout << "BTLoggerZMQ::worker initiating: " << endl;

            zmq::context_t context(1);
            zmq::socket_t server(context, ZMQ_PULL);
            server.bind("tcp://0.0.0.0:5555");

            cout << "BTLoggerZMQ bound to: tcp://0.0.0.0:5555" << endl;

            while(true)
            {
                zmq::message_t message;
                server.recv(&message);

                cout << "BTLoggerZMQ::worker got new message" << endl;
                queue->enqueue(std::string(static_cast<char*>(message.data()), message.size()));
                cout << "BTLoggerZMQ::worker message stored" << endl;
            }
        }
};

#endif