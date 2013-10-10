/*
* Thanks to Novalis :
* http://stackoverflow.com/questions/10139251/shared-queue-c
*/

#ifndef __SYNC_QUEUE__
#define __SYNC_QUEUE__

#include <queue>
#include <boost/thread.hpp>  

template <typename T>
class SynchronisedQueue
{
    private:
        std::queue<T> m_queue;              // Use STL queue to store data
        boost::mutex m_mutex;               // The mutex to synchronise on
        boost::condition_variable m_cond;   // The condition to wait for

        bool requestToEnd;
        bool enqueueData;

    public:

        SynchronisedQueue()
        {
            requestToEnd = false;  
            enqueueData = true;
        }

        void enqueue(const T& data)
        {
            boost::unique_lock<boost::mutex> lock(m_mutex);

            if(enqueueData)
            {
                m_queue.push(data);
                m_cond.notify_one();
            }

        } 

        bool tryDequeue(T& result)
        {
            boost::unique_lock<boost::mutex> lock(m_mutex);

            while (m_queue.empty() && (! requestToEnd)) 
            { 
                m_cond.wait(lock);
            }

            if( requestToEnd )
            {
                doEndActions();
                return false;
            }

            result= m_queue.front(); m_queue.pop();

            return true;
        }

        void stopQueue()
        {
            boost::unique_lock<boost::mutex> lock(m_mutex);
            requestToEnd =  true;
            m_cond.notify_one();
        }

        int size()
        {
            boost::unique_lock<boost::mutex> lock(m_mutex);
            return m_queue.size();

        }

    private:

        void doEndActions()
        {
            enqueueData = false;

            while (!m_queue.empty())  
            {
                m_queue.pop();
            }
        }
};

#endif
