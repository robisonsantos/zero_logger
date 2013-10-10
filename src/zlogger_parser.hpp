#ifndef __BT_LOGGER_PARSER__
#define __BT_LOGGER_PARSER__

#include <string>
#include <iostream>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>

#define CONFIG_TYPE "config"
#define LOG_TYPE    "log"

//JSON protocol 
// Config:
#define TYPE_FIELD          "type"
#define FILE_NAME_FIELD     "fileName"
#define FILE_LOCATION_FIELD "fileLocation"
#define PRIORITY_FIELD      "priority"
#define MAX_FILESIZE_FIELD  "maxFileSize"
#define MAX_BACKUP_FIELD    "maxBackupIndex"

// Log
#define LOG_LEVEL_FIELD    "logLevel"
#define LOG_MESSAGE_FIELD  "message"
#define TIMESTAMP_FIELD    "timestamp"

// Default values
#define DEFAULT_PRIORITY   "INFO"
#define DEFAULT_MAX_BACKUP 10
#define DEFAULT_FILESIZE   10 * 1024 * 1024 // 10M


using namespace std;

// Simple enum to categorize one request
enum BTLoggerRequestType {CONFIG, LOG, INVALID};


// Simple struct to hold a parsed request.
struct BTLoggerRequest
{
    BTLoggerRequestType type;
    string fileName;
    string fileLocation;
    string logLevel;
    string logMessage;
    string priority;
    string timestamp;
    size_t maxFileSize;
    unsigned int maxBackupIndex;
};

// Main class responsible for parsing requests
class BTLoggerParser 
{
    public:
        BTLoggerParser() {}
        ~BTLoggerParser() {}

        BTLoggerRequest* parse(string request)
        {
            cout << "BTLoggerParser parsing: " << request << endl;

            BTLoggerRequest *parsedRequest;
            parsedRequest = new BTLoggerRequest();

            // Parsing json
            try {
                stringstream requeststream(request);
                boost::property_tree::ptree json;
                boost::property_tree::read_json(requeststream, json);

                string type = json.get<string>(TYPE_FIELD);
                
                if(type == CONFIG_TYPE )
                {
                    cout << "BTLoggerParser it's a config request: " << request << endl;

                    parsedRequest->type           = CONFIG;
                    parsedRequest->fileName       = json.get<string>(FILE_NAME_FIELD);
                    parsedRequest->fileLocation   = json.get<string>(FILE_LOCATION_FIELD);
                    parsedRequest->priority       = json.get<string>(PRIORITY_FIELD, DEFAULT_PRIORITY);
                    parsedRequest->maxFileSize    = json.get<size_t>(MAX_FILESIZE_FIELD, DEFAULT_FILESIZE);
                    parsedRequest->maxBackupIndex = json.get<unsigned int>(MAX_BACKUP_FIELD, DEFAULT_MAX_BACKUP);
                } 
                else if(type == LOG_TYPE)
                {
                    cout << "BTLoggerParser it's a log request: " << request << endl;

                    parsedRequest->type        = LOG;
                    parsedRequest->fileName    = json.get<string>(FILE_NAME_FIELD);
                    cout << "Got fileName from: " << request << endl;

                    parsedRequest->logLevel    = json.get<string>(LOG_LEVEL_FIELD);
                    cout << "Got loglevel from: " << request << endl;

                    parsedRequest->logMessage  = json.get<string>(LOG_MESSAGE_FIELD);
                    cout << "Got logMessage from: " << request << endl;

                    parsedRequest->timestamp   = json.get<string>(TIMESTAMP_FIELD);
                    cout << "Got timestamp from: " << request << endl;
                }
                else
                {
                    cout << "BTLoggerParser unknown request: " << request << endl;
                    parsedRequest->type = INVALID;
                }
            }
            catch(...)
            {
                cout << "BTLoggerParser error during parsing: " << request << endl;
                parsedRequest->type = INVALID;
            }

            return parsedRequest;
        }
};

#endif