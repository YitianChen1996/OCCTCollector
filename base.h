#include <iostream>
#include <string>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <fstream>
#include <map>
#include <deque>
#include <unordered_map>

using namespace std;

struct GlobalVariables {
    const string requestURL = "http://binghamtonupublic.etaspot.net/service.php?service=get_vehicles&includeETAData=0&orderedETAArray=0&token=TESTING";
    const int CAMPUS_SHUTTLE_ROUTEID = 8;
    string tempFileName = "data.txt", curFileName;


    //These global variables are used to control the file writing.
    unsigned long lineCount = 0, fileCount = 0, MAX_LOG_PER_FILE = 100000;
    bool firstWriteToThisFile, firstWrite;

    //These global variables are used to take care of the "Loop"
    unordered_map<int, deque<int>> busToStopsMap; 
    const int LOOP1STOP = 263;
    const int LOOP2STOP = 264;
};

unsigned char processRecord(const map<string, string> &myMap, GlobalVariables &globals, bool RR);

void parseUsefulContent(const string &content, GlobalVariables &globals, bool RR);

//logger
void LOG_INFO(const string &s);
