all: startCollector.cpp base.h logger.cpp util.cpp
	g++ util.cpp logger.cpp startCollector.cpp -std=c++14 -g -o startCollector
clean:
	rm startCollector
