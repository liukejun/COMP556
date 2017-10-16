//
// Created by SHUO ZHAO on 10/15/17.
//
#include <ReceiverWindow.h>

vector<string> ReceiverWindow::split(const string &s, char delim) {
    stringstream ss(s);
    string item;
    vector<string> tokens;
    while (getline(ss, item, delim)) {
        tokens.push_back(item);
    }
    return tokens;
}

int ReceiverWindow::createFile (string file) {
    if (file.length() == 0) {
        cout << "Path and file name is empty" << "\n";
        return -1;
    } else {
        vector<string> pathFile = split(file, ' ');
        string path = pathFile[0];
        string name = pathFile[1] + ".recv";
        int status = mkdir(path.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
        ofstream file(path+name);
        string data("data to write to file");
        file << data;
        return 0;
    }
}