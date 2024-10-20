#include <iostream>
 
#include <fcntl.h>
#include <stdlib.h>
 
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

#include <sstream>

using namespace std;

int main(int argc, char *argv[]) {
    if (argc == 1) {
        return 0;
    }

    for (int i = 1; i < argc; i++) {
        int fileDescriptor = open(argv[i], O_RDONLY);
        if (fileDescriptor < 0) {
            cout << "wcat: cannot open file" << endl;
            exit(1);
        }

        char buffer[4096];
        ssize_t bytesRead;
        while ((bytesRead = read(fileDescriptor, buffer, sizeof(buffer))) > 0) {
            if (write(STDOUT_FILENO, buffer, bytesRead) != bytesRead) {
                close(fileDescriptor);
                return 1;
            }
        }

        close(fileDescriptor);
    }

    return 0;
}