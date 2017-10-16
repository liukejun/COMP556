//
// Created by SHUO ZHAO on 10/15/17.
//
#include <ReceiverWindow.h>


SenderWindow::SenderWindow(char *file_path_name, int window_size, int sock, struct sockaddr *si_other, socklen_t addr_len) :
        SuperClass(file_path_name, window_size, sock, sockaddr * si_other, addr_len) {

    //    /* get path and file name */
//    vector <string> file_path_vec = split(file_path, '/');
//    int file_path_size = (int) file_path_vec.size();
//    file_name = file_path_vec[file_path_size - 1];
//    file_path = file_path.substr(0, file_path.length() - fileName.length());
}


vector <string> ReceiverWindow::split(const string &s, char delim) {
    stringstream ss(s);
    string item;
    vector <string> tokens;
    while (getline(ss, item, delim)) {
        tokens.push_back(item);
    }
    return tokens;
}

int ReceiverWindow::createFile(string file) {
    if (file.length() == 0) {
        cout << "Path and file name is empty" << "\n";
        return -1;
    } else {
        vector <string> pathFile = split(file, ' ');
        string path = pathFile[0];
        string name = pathFile[1] + ".recv";
        int status = mkdir(path.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
        ofstream file(path + name);
        string data("data to write to file");
        file << data;
        return 0;
    }
}