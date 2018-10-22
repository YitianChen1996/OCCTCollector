#include <iostream>
#include <string>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <fstream>
#include <map>

using namespace std;

string requestURL = "http://binghamtonupublic.etaspot.net/service.php?service=get_vehicles&includeETAData=0&orderedETAArray=0&token=TESTING";

string tempFileName = "data.txt", curFileName;

unsigned long lineCount = 0, fileCount = 0, MAX_LOG_PER_FILE = 100000;
bool firstWriteToThisFile, firstWrite;

void parseUsefulContent(const string &content) {
	size_t contentLen = content.length(), pos = 0;
	// For each log, parse begin at routeID, end at receiveTime
	while (true) {
		map<string, string> myMap;
		myMap.clear();
		size_t keyBegin, keyEnd, valueBegin, valueEnd;
		pos = content.find("{\"routeID\"", pos);
		if (pos >= contentLen) { break; }
		// At the beginning of this loop, pos = '{' (first key) or pos = ','
		while (true) {
			keyBegin = pos + 1;
			pos = content.find(":", pos + 1);
			keyEnd = pos - 1;
			valueBegin = pos + 1;
			pos = content.find(",", valueBegin);
			valueEnd = pos - 1;
			string key = content.substr(keyBegin, keyEnd - keyBegin + 1);
			string value = content.substr(valueBegin, valueEnd - valueBegin + 1);
			myMap.insert(make_pair(key, value));
			if (key == "\"receiveTime\"") { break; }
		}
		string writeToFile;
		writeToFile += (myMap.find("\"receiveTime\"")->second) + string(",");
		writeToFile += (myMap.find("\"equipmentID\"")->second) + string(",");
		writeToFile += (myMap.find("\"lat\"")->second) + string(",");
		writeToFile += (myMap.find("\"lng\"")->second) + string(",");
		writeToFile += (myMap.find("\"nextStopID\"")->second) + string(",");
		writeToFile += (myMap.find("\"routeID\"")->second) + string(",");
		writeToFile += myMap.find("\"inService\"")->second;
		if (lineCount > MAX_LOG_PER_FILE) {
			fileCount++;
			lineCount = 0;
			firstWriteToThisFile = true;
		} else {
			lineCount++;
		}
		if (firstWrite || firstWriteToThisFile) {
			curFileName = "record" + myMap.find("\"receiveTime\"")->second + ".csv";
			firstWrite = false;
		}
		ofstream outputFile(curFileName, ios::app);
		if (firstWriteToThisFile) {
			outputFile << "\"timestamps\",\"busNum\",\"lat\",\"lng\",\"nextStopID\",\"routeID\",\"inService\"" << endl;
			firstWriteToThisFile = false;
		}
		outputFile << writeToFile << endl;
		outputFile.close();
	}
}

int main() {
	firstWriteToThisFile = true;
	firstWrite = true;
	while (true) {
		pid_t childPid = fork();
		if (childPid == -1) { cout << "fork child error" << endl; }
		else if (childPid == 0) { // this is child process
			char **childArgv;
			childArgv = (char**) malloc(5 * sizeof(char*));
			childArgv[0] = (char*) malloc(6 * sizeof(char)); // for wget;
			childArgv[1] = (char*) malloc(4 * sizeof(char)); // for "-O"
			childArgv[2] = (char*) malloc(20 * sizeof(char)); // for output file name
			childArgv[3] = (char*) malloc(100 * sizeof(char)); // for the url
			childArgv[4] = NULL; // request by execvp call
			strcpy(childArgv[0], "wget");
			strcpy(childArgv[1], "-O");
			strcpy(childArgv[2], tempFileName.c_str());
			strcpy(childArgv[3], requestURL.c_str());
			execvp("wget", childArgv);
			cout << "[ERROR], you shouldn't arrive at here" << endl;
			exit(0);
		}

		while (true) {
			pid_t waitPid = wait(NULL);
			if (waitPid == -1) {
				if (errno == ECHILD) {
					break;
				} else {
					cout << "something error in waiting for child" << endl;
				}
			}
		}
		cout << "wget done, begin parse the file" << endl;
		
		string content;
		ifstream inputFile(tempFileName);
		inputFile >> content;
		inputFile.close();
		parseUsefulContent(content);
		sleep(10);
	}
    return 0;   
}
