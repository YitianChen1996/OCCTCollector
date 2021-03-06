#include "base.h"

void parseUsefulContent(const string &content, GlobalVariables &globals, bool RR) {
	size_t contentLen = content.length(), pos = 0;
	unsigned char is_loop;
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
		is_loop = processRecord(myMap, globals, RR);
		string writeToFile;
		writeToFile += (myMap.find("\"receiveTime\"")->second) + string(",");
		writeToFile += (myMap.find("\"equipmentID\"")->second) + string(",");
		writeToFile += (myMap.find("\"lat\"")->second) + string(",");
		writeToFile += (myMap.find("\"lng\"")->second) + string(",");
		writeToFile += (myMap.find("\"nextStopID\"")->second) + string(",");
		writeToFile += (myMap.find("\"routeID\"")->second) + string(",");
		writeToFile += myMap.find("\"inService\"")->second + string(",");
		writeToFile += '0' + is_loop;
		if (globals.lineCount > globals.MAX_LOG_PER_FILE) {
			globals.fileCount++;
			globals.lineCount = 0;
			globals.firstWriteToThisFile = true;
		} else {
			globals.lineCount++;
		}
		if (globals.firstWrite || globals.firstWriteToThisFile) {
			globals.curFileName = "record" + myMap.find("\"receiveTime\"")->second + ".csv";
			globals.firstWrite = false;
		}
		ofstream outputFile(globals.curFileName, ios::app);
		if (globals.firstWriteToThisFile) {
			outputFile << "\"timestamps\",\"busNum\",\"lat\",\"lng\",\"nextStopID\",\"routeID\",\"inService\",\"loopCompleted\"" << endl;
			globals.firstWriteToThisFile = false;
		}
		outputFile << writeToFile << endl;
		outputFile.close();
	}
}

unsigned char processRecord(const map<string, string> &myMap, GlobalVariables &globals, bool RR) {
    int routeID = stoi(myMap.find("\"routeID\"")->second, nullptr, 10);
    int inService = stoi(myMap.find("\"inService\"")->second, nullptr, 10);
    static int sf_bus[7] = {0};
    const int buses[7] = {380, 381, 747, 748, 762, 763, 777};
    int i; char strcmd[128];
    // If the route is not the campus shuttle's route or it is not in service, do not process.
    if (routeID != globals.CAMPUS_SHUTTLE_ROUTEID || inService == 0) {
        return 0;
    }
    string busNumStr = myMap.find("\"equipmentID\"")->second;
    busNumStr = busNumStr.substr(1, busNumStr.length() - 2); // remove the quota.
    int busNum = stoi(busNumStr, nullptr, 10);
    int nextStopID = stoi(myMap.find("\"nextStopID\"")->second, nullptr, 10);
    if (globals.busToStopsMap.find(busNum) == globals.busToStopsMap.end()) {
        deque<int> newQueue;
        newQueue.push_back(nextStopID);
        newQueue.push_back(nextStopID);
        globals.busToStopsMap.insert(make_pair(busNum, newQueue));
    } else {
        globals.busToStopsMap[busNum].pop_front();
        globals.busToStopsMap[busNum].push_back(nextStopID);
    }
    if (globals.busToStopsMap[busNum].front() == globals.LOOP1STOP && globals.busToStopsMap[busNum].back() == globals.LOOP2STOP) {
        /* Completed loop detected */
        for (i = 0; i < 7; i++) if (buses[i] == busNum) break;
        if (i == 7) return 0; //unknown bus number
        if (RR) {  //SF round robin
            sf_bus[i] = (sf_bus[i] + 1) % 6;
            sprintf(strcmd, "echo %d | ncat 128.226.123.247 111%02d", sf_bus[i] + 7, busNum % 100);
            system(strcmd);
        }
        LOG_INFO("Bus " + to_string(busNum) + " has finished one loop");
        return 1;
    }
    return 0;
}
