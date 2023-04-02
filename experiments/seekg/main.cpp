// Here we will write a simple C++ program that will read a file and print bytes 1024-2047 from it using seekg

#include <iostream>
#include <fstream>
#include <string>

using namespace std;

int main(int argc, char *argv[]) {

    if (argc < 2) {
        cerr << "No file given" << endl;
        return 1;
    }

    ifstream file(argv[1], ios::in | ios::binary);
    if (!file) {
        cout << "Cannot open file." << endl;
        return 1;
    }


    file.seekg(1024, ios::beg);
    char buffer[1025];

    for (int i = 0; i < 1024; i++) {
        // This is just to make sure that all of the buffer is filled by the file
        buffer[i] = 'i';
    }

    file.read(buffer, 1024);

    buffer[1024] = '\0';
    cout << buffer << endl;

    return 0;
}   