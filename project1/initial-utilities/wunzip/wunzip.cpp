#include <fcntl.h>   
#include <unistd.h>   
#include <iostream>
#include <string.h>   
#include <errno.h>    

using namespace std;

int main(int argc, char* argv[]) {
    if (argc < 2) {
        const char* message = "wunzip: file1 [file2 ...]\n";
        write(STDOUT_FILENO, message, strlen(message));
        return 1;
    }

    for (int fileIndex = 1; fileIndex < argc; ++fileIndex) {
        int fd = open(argv[fileIndex], O_RDONLY);
        if (fd < 0) {
            const char* errorMsg = "wunzip: cannot open file ";
            write(STDERR_FILENO, errorMsg, strlen(errorMsg));
            write(STDERR_FILENO, argv[fileIndex], strlen(argv[fileIndex]));
            write(STDERR_FILENO, "\n", 1);
            return 1;
        }

        char currentChar;
        int repetitionCount;

        while (read(fd, &repetitionCount, sizeof(repetitionCount)) == sizeof(repetitionCount)) {
            if (read(fd, &currentChar, sizeof(currentChar)) == sizeof(currentChar)) {
                for (int counter = 0; counter < repetitionCount; ++counter) {
                    write(STDOUT_FILENO, &currentChar, sizeof(currentChar));
                }
            }
        }

        close(fd);
    }

    return 0;
}
