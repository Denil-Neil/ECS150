#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

using namespace std;

#define BUFFER_SIZE 4096

int main(int argc, char* argv[]) {
    if (argc < 2) {
        write(STDOUT_FILENO, "wzip: file1 [file2 ...]\n", 24);
        return 1;
    }

    char previous = '\0';
    int count = 0;
    bool isFirstCharacter = true;
    char buffer[BUFFER_SIZE];

    for (int fileIndex = 1; fileIndex < argc; ++fileIndex) {
        int fileDescriptor = open(argv[fileIndex], O_RDONLY);
        if (fileDescriptor < 0) {
            write(STDERR_FILENO, "wzip: cannot open file\n", 23);
            return 1;
        }

        ssize_t bytesRead;
        while ((bytesRead = read(fileDescriptor, buffer, sizeof(buffer))) > 0) {
            for (ssize_t i = 0; i < bytesRead; ++i) {
                char current = buffer[i];

                if (isFirstCharacter) {
                    previous = current;
                    count = 1;
                    isFirstCharacter = false;
                } else if (current == previous) {
                    count++;
                } else {
                    // Write the run-length encoded data
                    write(STDOUT_FILENO, &count, sizeof(count));
                    write(STDOUT_FILENO, &previous, 1);

                    previous = current;
                    count = 1;
                }
            }
        }

        close(fileDescriptor);
    }

    if (count > 0) {
        write(STDOUT_FILENO, &count, sizeof(count));
        write(STDOUT_FILENO, &previous, 1);
    }

    return 0;
}
