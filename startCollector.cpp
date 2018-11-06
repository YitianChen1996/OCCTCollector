#include "base.h"

int main(int argn, char* argv[]) {
	GlobalVariables globals;
	globals.firstWriteToThisFile = true;
	globals.firstWrite = true;
	while (true) {
		pid_t childPid = fork();
		if (childPid == -1) { LOG_INFO("fork child error"); }
		else if (childPid == 0) { // this is child process
			char **childArgv;
			childArgv = (char**) malloc(5 * sizeof(char*));
			childArgv[0] = (char*) malloc(6 * sizeof(char)); // for wget;
			childArgv[1] = (char*) malloc(4 * sizeof(char)); // for "-O"
			childArgv[2] = (char*) malloc(20 * sizeof(char)); // for output file name
			childArgv[3] = (char*) malloc(100 * sizeof(char)); // for the url
			childArgv[4] = NULL; // required by execvp call
			strcpy(childArgv[0], "wget");
			strcpy(childArgv[1], "-O");
			strcpy(childArgv[2], globals.tempFileName.c_str());
			strcpy(childArgv[3], globals.requestURL.c_str());
			execvp("wget", childArgv);
			LOG_INFO("[ERROR], you shouldn't arrive at here"); //why??
			for (int i = 0; i <= 4; i++) free(childArgv[i]);
			free(childArgv);
			exit(0);
		}

		while (true) {
			pid_t waitPid = wait(NULL);
			if (waitPid == -1) {
				if (errno == ECHILD) {
					break;
				} else {
					LOG_INFO("something error in waiting for child");
				}
			}
		}
		LOG_INFO("wget done, begin parse the file");
		
		string content;
		ifstream inputFile(globals.tempFileName);
		inputFile >> content;
		inputFile.close();
		parseUsefulContent(content, globals, argn == 2 && argv[1][0] == 'r');
		sleep(10);
	}
    return 0;   
}
