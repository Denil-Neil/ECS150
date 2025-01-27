#nclude <iostream>
#include <string>
#include <string.h>

#include <unistd.h>

using namespace std;

int main(int argc, char *argv[]){
    int a = 1234;
    char *b = new char[128];
    strcpy(b, "hello world");

    int ret = fork();

    if (ret < 0){
        cerr << "Process did not fork correctly" << endl;
    }
    else if (ret == 0){
        // Child
        cout << "I am the child and this is my pid: " << getpid() << endl;
        cout << "a = " << a << " b = " << b << endl;
    }
    else{
        cout << "I am the parent and this is my pid: " << getpid() << endl;
        cout << "a = " << a << " b = " << b << endl;
    }
	

    return  0;
}
