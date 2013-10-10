#ifndef __BT_LOGGER__
#define __BT_LOGGER__

#include <map>
#include <queue>
#include <boost/thread.hpp>
#include <boost/algorithm/string.hpp>

#include "log4cpp/Category.hh"
#include "log4cpp/Appender.hh"
#include "log4cpp/RollingFileAppender.hh"
#include "log4cpp/Priority.hh"
#include "log4cpp/PatternLayout.hh"

#include "sync_queue.hpp"
#include "zlogger_parser.hpp" // To get access to ZLoggerRequest and ZLoggerRequestType

// Log a simple message as it is and break line
#define LOG_LAYOUT_PATTERN "%m%n"
#define DEFAULT_WORKER_THREADS 10

using namespace std;

struct Task
{
    log4cpp::Category* category;
    ZLoggerRequest* request;
};


/*
 * For each config request a new thread is spawned.
 * Each thread will handle one log file, and will
 * consume from one queue.
 */
class ZLogger
{
    private:
        map<string, log4cpp::Category* > categoryMap;
        map<string, boost::thread* > threadsMap;
        map<boost::thread*, SynchronisedQueue<Task*>* > loggingTaskMap;
        queue<boost::thread* > threadsQueue;

    public:
        ZLogger(int workerThreads = DEFAULT_WORKER_THREADS)
        {
            // TODO: create worker threads
            for(int i = 0; i < workerThreads; i++)
            {
                // create threads and populate queues and maps
                SynchronisedQueue<Task*>* queue = new SynchronisedQueue<Task*>();
                boost::thread *workerThread = new boost::thread(&ZLogger::worker, this, queue);
                this->threadsQueue.push(workerThread);
                this->loggingTaskMap[workerThread] = queue;
            }
        }

        ~ZLogger() 
        { 
            //TODO: stop all threads
            //TODO: delete all threads
            //TODO: delete all queues
        }

        void log(ZLoggerRequest *request)
        {
            if(request->type == CONFIG)
            {
                log4cpp::Category &category = configureNewLogger(request->fileLocation, 
                                                                 request->fileLocation + "/" + request->fileName, 
                                                                 request->maxFileSize, 
                                                                 request->maxBackupIndex, 
                                                                 request->priority);
                this->categoryMap[request->fileName] = &category;
                delete request;
            } 
            else if(request->type == LOG)
            {
                // Create a new task if we have a category for this request
                map<string, log4cpp::Category* >::iterator itCategory;
                itCategory = this->categoryMap.find(request->fileName);
                if(itCategory != this->categoryMap.end())
                {
                    // Create a new task
                    Task *task;
                    task->category = itCategory->second;
                    task->request = request;

                    // Check if a thread handling this file already exists
                    boost::thread* workerThread;

                    map<string, boost::thread* >::iterator itThreads;
                    itThreads = this->threadsMap.find(request->fileName);
                    if(itThreads == this->threadsMap.end())
                    {
                        // I dont have a thread for this file yet.
                        // Grab one from the pool 
                        workerThread = nextThread();
                        this->threadsMap[request->fileName] = workerThread;
                    }
                    else
                    {
                        workerThread = itThreads->second;
                    }
                    
                    // Now, I sure have a thread for this file, let's use it
                    this->loggingTaskMap[workerThread]->enqueue(task);
                }

            }
        }

    private:
        void worker(SynchronisedQueue<Task*> *queue)
        {
            cout << "Thread: " << boost::this_thread::get_id() << " initiated and listening queue " << queue << endl;
            while(true) 
            {
                Task *task;
                if(queue->tryDequeue(task) && task->request->type == LOG) 
                {
                    // Do the logging
                    boost::to_upper(task->request->logLevel);
                    if( task->request->logLevel == "DEBUG" && task->category->isDebugEnabled())
                    {
                        task->category->debug("%s - [DEBUG] %s", task->request->timestamp.c_str(), task->request->logMessage.c_str());
                    } 
                    else if(task->request->logLevel == "INFO" && task->category->isInfoEnabled())
                    {
                        task->category->info("%s - [INFO] %s", task->request->timestamp.c_str(), task->request->logMessage.c_str());
                    }
                    else if(task->request->logLevel == "NOTICE" && task->category->isNoticeEnabled())
                    {
                        task->category->notice("%s - [NOTICE] %s", task->request->timestamp.c_str(), task->request->logMessage.c_str());
                    }
                    else if(task->request->logLevel == "WARN" && task->category->isWarnEnabled())
                    {
                        task->category->warn("%s - [WARN] %s", task->request->timestamp.c_str(), task->request->logMessage.c_str());
                    }
                    else if(task->request->logLevel == "ERROR" && task->category->isErrorEnabled())
                    {
                        task->category->error("%s - [ERROR] %s", task->request->timestamp.c_str(), task->request->logMessage.c_str());
                    }
                    else if(task->request->logLevel == "CRIT" && task->category->isCritEnabled())
                    {
                        task->category->crit("%s - [CRIT] %s", task->request->timestamp.c_str(), task->request->logMessage.c_str());
                    }
                    else if(task->request->logLevel == "ALERT" && task->category->isAlertEnabled())
                    {
                        task->category->alert("%s - [ALERT] %s", task->request->timestamp.c_str(), task->request->logMessage.c_str());
                    }
                    else if(task->request->logLevel == "FATAL" && task->category->isFatalEnabled())
                    {
                        task->category->fatal("%s - [FATAL] %s", task->request->timestamp.c_str(), task->request->logMessage.c_str());
                    }

                    // free the memory
                    //delete task->request;
                    //delete task;
                }
            }
        }

        /*********************************************************************/
        boost::thread* nextThread()
        {
            boost::thread *next = this->threadsQueue.front();
            this->threadsQueue.pop();
            this->threadsQueue.push(next);
            return next;
        }

        /*********************************************************************/
        log4cpp::Category& configureNewLogger(string fileName, 
                                              string filePath, 
                                              size_t maxFileSize, 
                                              unsigned int maxBackupIndex, 
                                              string priority) 
        {
            // Create a new category
            log4cpp::Category &category = log4cpp::Category::getInstance(fileName);

            // Create RollingFileAppender
            log4cpp::Appender *rfileAppender = new log4cpp::RollingFileAppender(fileName,
                                                                                filePath,
                                                                                maxFileSize,
                                                                                maxBackupIndex);
            if (rfileAppender != NULL)
            {
                // Create PatternLayout
                log4cpp::PatternLayout *layout = new log4cpp::PatternLayout();
                
                if (layout != NULL)
                {
                    layout->setConversionPattern(LOG_LAYOUT_PATTERN);
                    rfileAppender->setLayout(layout);

                    category.setAdditivity(true);

                    try
                    {
                        category.setPriority(log4cpp::Priority::getPriorityValue(priority));
                    }
                    catch(std::invalid_argument &ia)
                    {
                        cerr << "Invalid Priority: "  << priority << endl;
                        category.setPriority(log4cpp::Priority::INFO);
                    }

                    // Bind RollingFileAppender to Category
                    category.addAppender(rfileAppender);
                    
                }
            }
            return category;
        }
};

#endif