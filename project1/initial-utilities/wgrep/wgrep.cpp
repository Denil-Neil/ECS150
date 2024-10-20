#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

using namespace std;

#define BUFFER_SIZE 4096

int main(int argc, char* argv[]) {
    if (argc < 2) {
        write(STDOUT_FILENO, "wgrep: searchterm [file ...]\n", 29);
        return 1;
    }

    const char* keyword = argv[1];

    if (argc == 2) {
        char buffer[BUFFER_SIZE];
        string currentTextLine;
        ssize_t bytesRead;

        while ((bytesRead = read(STDIN_FILENO, buffer, sizeof(buffer))) > 0) {
            for (ssize_t i = 0; i < bytesRead; ++i) {
                if (buffer[i] == '\n') {
                    if (currentTextLine.find(keyword) != string::npos) {
                        write(STDOUT_FILENO, currentTextLine.c_str(), currentTextLine.size());
                        write(STDOUT_FILENO, "\n", 1);
                    }
                    currentTextLine.clear();
                } else {
                    currentTextLine += buffer[i];
                }
            }
        }

        if (!currentTextLine.empty() && currentTextLine.find(keyword) != string::npos) {
            write(STDOUT_FILENO, currentTextLine.c_str(), currentTextLine.size());
            write(STDOUT_FILENO, "\n", 1);
        }
    } else {
        for (int fileIndex = 2; fileIndex < argc; ++fileIndex) {
            int fileDescriptor = open(argv[fileIndex], O_RDONLY);
            if (fileDescriptor < 0) {
                write(STDOUT_FILENO, "wgrep: cannot open file\n", 24);
                return 1;
            }

            char buffer[BUFFER_SIZE];
            string currentTextLine;
            ssize_t bytesRead;

            while ((bytesRead = read(fileDescriptor, buffer, sizeof(buffer))) > 0) {
                for (ssize_t i = 0; i < bytesRead; ++i) {
                    if (buffer[i] == '\n') {
                        if (currentTextLine.find(keyword) != string::npos) {
                            write(STDOUT_FILENO, currentTextLine.c_str(), currentTextLine.size());
                            write(STDOUT_FILENO, "\n", 1);
                        }
                        currentTextLine.clear();
                    } else {
                        currentTextLine += buffer[i];
                    }
                }
            }

            if (!currentTextLine.empty() && currentTextLine.find(keyword) != string::npos) {
                write(STDOUT_FILENO, currentTextLine.c_str(), currentTextLine.size());
                write(STDOUT_FILENO, "\n", 1);
            }

            close(fileDescriptor);
        }
    }

    return 0;
}
