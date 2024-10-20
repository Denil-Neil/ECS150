#include <iostream>
#include <sstream>
#include <deque>
#include <string>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <cstring>
#include <algorithm>
#include <fstream>
#include <vector>

using namespace std;

const string SHELL_PROMPT = "wish> ";
const char error_message[30] = "An error has occurred\n";

// Function to print the error message
void printError() {
    write(STDERR_FILENO, error_message, strlen(error_message));
}

int main(int argc, char* argv[]) {
    deque<string> executablePaths;
    executablePaths.push_back("/bin");

    if (argc > 2) {
        printError();
        exit(1);
    }

    istream* input = &cin;
    ifstream batchInput;
    bool interactiveMode = true;

    if (argc == 2) {
        batchInput.open(argv[1]);
        if (!batchInput) {
            printError();
            exit(1);
        }
        input = &batchInput;
        interactiveMode = false;
    }

    string inputLine;
    vector<pid_t> backgroundJobs;  // To track background processes
    
    while (true) {
        if (interactiveMode) {
            cout << SHELL_PROMPT;
        }

        if (!getline(*input, inputLine)) {
            if (interactiveMode) {
                cout << endl;
            }
            break;
        }

        inputLine.erase(0, inputLine.find_first_not_of(" \t"));
        inputLine.erase(inputLine.find_last_not_of(" \t") + 1);

        if (inputLine.empty()) continue;

        size_t pos = 0, found;
        deque<string> commands;

        // Splitting by '&' to handle multiple commands
        while ((found = inputLine.find_first_of('&', pos)) != string::npos) {
            commands.push_back(inputLine.substr(pos, found - pos));
            pos = found + 1;
        }
        if (pos < inputLine.size()) {
            commands.push_back(inputLine.substr(pos));
        }

        for (const string& singleCommand : commands) {
            deque<string> args;
            string currentArg;
            bool insideQuotes = false;

            for (char currentChar : singleCommand) {
                if (currentChar == '"') {
                    insideQuotes = !insideQuotes;
                } else if (currentChar == ' ' && !insideQuotes) {
                    if (!currentArg.empty()) {
                        args.push_back(currentArg);
                        currentArg.clear();
                    }
                } else if (currentChar == '>' && !insideQuotes) {
                    if (!currentArg.empty()) {
                        args.push_back(currentArg);
                        currentArg.clear();
                    }
                    args.push_back(">");
                } else {
                    currentArg += currentChar;
                }
            }
            if (!currentArg.empty()) {
                args.push_back(currentArg);
            }

            // Error handling for redirection
            if (args.empty()) continue;
            if (args[0] == ">" || (args.size() > 1 && args[0].empty() && args[1] == ">")) {
                printError();
                continue;
            }

            // Handling built-in commands: exit, cd, and path
            if (args[0] == "exit") {
                if (args.size() != 1) {
                    printError();
                } else {
                    // Wait for all background processes to complete before exiting
                    for (pid_t job : backgroundJobs) {
                        waitpid(job, nullptr, 0);
                    }
                    exit(0);
                }
            } else if (args[0] == "cd") {
                if (args.size() != 2 || chdir(args[1].c_str()) != 0) {
                    printError();
                }
            } else if (args[0] == "path") {
                executablePaths.clear();
                executablePaths.insert(executablePaths.end(), args.begin() + 1, args.end());
            } else if (!executablePaths.empty()) {
                deque<string> cmdArgs = args;
                auto redirectPos = find(cmdArgs.begin(), cmdArgs.end(), ">");
                int outputFile = -1;

                if (redirectPos != cmdArgs.end()) {
                    if (distance(redirectPos, cmdArgs.end()) != 2) {
                        printError();
                        continue;
                    }

                    string outputFileName = *(redirectPos + 1);
                    outputFile = open(outputFileName.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
                    if (outputFile == -1) {
                        printError();
                        continue;
                    }
                    cmdArgs.erase(redirectPos, cmdArgs.end());
                }

                pid_t processID = fork();
                if (processID == 0) {
                    // Child process
                    vector<char*> execArgs;
                    for (const string& arg : cmdArgs) {
                        execArgs.push_back(const_cast<char*>(arg.c_str()));
                    }
                    execArgs.push_back(nullptr);  // Null-terminate the argument list

                    if (outputFile != -1) {
                        dup2(outputFile, STDOUT_FILENO);
                        dup2(outputFile, STDERR_FILENO);
                        close(outputFile);
                    }

                    for (const string& execPath : executablePaths) {
                        string fullExecPath = execPath + "/" + cmdArgs[0];
                        if (access(fullExecPath.c_str(), X_OK) == 0) {
                            execv(fullExecPath.c_str(), execArgs.data());  // Using vector<char*> with data()
                            exit(0);
                        }
                    }
                    printError();
                    exit(1);
                } else if (processID > 0) {
                    if (outputFile != -1) {
                        close(outputFile);
                    }

                    // Check if the command is supposed to run in the background
                    if (inputLine.back() == '&') {
                        backgroundJobs.push_back(processID);  // Track background jobs
                    } else {
                        // If not a background process, wait for the process to complete
                        int status;
                        waitpid(processID, &status, 0);
                    }
                } else {
                    printError();
                }
            } else {
                printError();
            }
        }
    }

    // Ensure all background jobs complete before exiting
    for (pid_t job : backgroundJobs) {
        waitpid(job, nullptr, 0);
    }

    return 0;
}