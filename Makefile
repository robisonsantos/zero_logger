LOG4CPP_INC=/home/robison/Downloads/log4cpp/include
LOG4CPP_LIB=/home/robison/Downloads/log4cpp/lib

ZMQ_INC=/home/robison/Downloads/zmqpp/zmq/include
ZMQ_LIB=/home/robison/Downloads/zmqpp/zmq/lib 

GPP_FLAGS=-I/usr/include -I$(LOG4CPP_INC) -I$(ZMQ_INC) -L/usr/include -L$(LOG4CPP_LIB) -L$(ZMQ_LIB) -lboost_thread-mt -llog4cpp -lzmq

all:
	mkdir -p ./bin
	g++ src/zlogger_driver.cpp -I./src $(GPP_FLAGS) -o bin/zlogger 

clean:
	rm bin/zlogger

