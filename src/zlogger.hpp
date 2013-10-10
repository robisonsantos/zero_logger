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

using namespace std;

/*
 * For each config request a new thread is spawned.
 * Each thread will handle one log file, and will
 * consume from one queue.
 */
class ZLogger
{
    private:
        map<string, SynchronisedQueue<ZLoggerRequest*>* > loggingRequestMap;
        queue<SynchronisedQueue<ZLoggerRequest*>* > taskQueue;

    public:
        ZLogger()
        {
            // TODO: create worker threads
            // How to create worker threads and keep the logging sequence???
        }

        ~ZLogger(){}

        void log(ZLoggerRequest* request)
        {
            if(request->type == CONFIG)
            {
                // Start a new thread with the config information
                // Should we first verify the log handler does not exist?
                // If a log handler exist, then the loggingRequestMap 
                // contains a key for the file.
                map<string, SynchronisedQueue<ZLoggerRequest*>* >::iterator it;
                it = loggingRequestMap.find(request->fileName);
                if(it == loggingRequestMap.end()) {
                    SynchronisedQueue<ZLoggerRequest*> *queue = new SynchronisedQueue<ZLoggerRequest*>();
                    loggingRequestMap[request->fileName] = queue;   
                    boost::thread workerThread(boost::bind(&worker, queue, request));
                }
                else
                {
                    cout << "ZLogger: log handler for " << request->fileName << " already configured" << endl;
                }
            } 
            else
            {
                // Enqueue a request to the corresponding queue if that file
                // has an entry into the map.
                map<string, SynchronisedQueue<ZLoggerRequest*>* >::iterator it;
                it = loggingRequestMap.find(request->fileName);
                if(it != loggingRequestMap.end()) {
                    it->second->enqueue(request);
                }
            }
        }

    private:
        static void worker(SynchronisedQueue<ZLoggerRequest*> *queue, ZLoggerRequest* config)
        {
            // Configure a new log handler
            try
            {

                log4cpp::Category &category = configureNewLogger(config->fileLocation, 
                                                                 config->fileLocation + "/" + config->fileName, 
                                                                 config->maxFileSize, 
                                                                 config->maxBackupIndex, 
                                                                 config->priority);
                delete config;

                // Keep logging stuff
                // TODO: when to stop the loop?
                while(true) 
                {
                    ZLoggerRequest *loggingRequest;
                    if(queue->tryDequeue(loggingRequest) && loggingRequest->type == LOG) 
                    {
                        // Do the logging
                        boost::to_upper(loggingRequest->logLevel);
                        if( loggingRequest->logLevel == "DEBUG" && category.isDebugEnabled())
                        {
                            category.debug("%s - [DEBUG] %s", loggingRequest->timestamp.c_str(), loggingRequest->logMessage.c_str());
                        } 
                        else if(loggingRequest->logLevel == "INFO" && category.isInfoEnabled())
                        {
                            category.info("%s - [INFO] %s", loggingRequest->timestamp.c_str(), loggingRequest->logMessage.c_str());
                        }
                        else if(loggingRequest->logLevel == "NOTICE" && category.isNoticeEnabled())
                        {
                            category.notice("%s - [NOTICE] %s", loggingRequest->timestamp.c_str(), loggingRequest->logMessage.c_str());
                        }
                        else if(loggingRequest->logLevel == "WARN" && category.isWarnEnabled())
                        {
                            category.warn("%s - [WARN] %s", loggingRequest->timestamp.c_str(), loggingRequest->logMessage.c_str());
                        }
                        else if(loggingRequest->logLevel == "ERROR" && category.isErrorEnabled())
                        {
                            category.error("%s - [ERROR] %s", loggingRequest->timestamp.c_str(), loggingRequest->logMessage.c_str());
                        }
                        else if(loggingRequest->logLevel == "CRIT" && category.isCritEnabled())
                        {
                            category.crit("%s - [CRIT] %s", loggingRequest->timestamp.c_str(), loggingRequest->logMessage.c_str());
                        }
                        else if(loggingRequest->logLevel == "ALERT" && category.isAlertEnabled())
                        {
                            category.alert("%s - [ALERT] %s", loggingRequest->timestamp.c_str(), loggingRequest->logMessage.c_str());
                        }
                        else if(loggingRequest->logLevel == "FATAL" && category.isFatalEnabled())
                        {
                            category.fatal("%s - [FATAL] %s", loggingRequest->timestamp.c_str(), loggingRequest->logMessage.c_str());
                        }

                        // free the memory
                        delete loggingRequest;
                    }
                }
            }  
            catch(...)
            {
                // Nothing to do here!
            } 
        }

        static log4cpp::Category& configureNewLogger(string fileName, string filePath, size_t maxFileSize, unsigned int maxBackupIndex, string priority) 
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