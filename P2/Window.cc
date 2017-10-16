//
// Created by SHUO ZHAO on 10/15/17.
//
#include <iostream>
#include <Window.h>

Window::Window(char* file, int w_size, int sock_in, struct sockaddr *si_other_in, socklen_t addr_len_in):file_path_name(file), window_size(w_size), sock
        (sock_in), si_other(si_other_in), addr_len(addr_len_in), min_seq_idx(0), is_complete(false){
    for (int i = 0; i < window_size; i++){
        slots.push_back(Slot(window_size, i));
    }
}

Window::~Window(){

}

vector <string> Window::split(const string &s, char delim) {
    stringstream ss(s);
    string item;
    vector <string> tokens;
    while (getline(ss, item, delim)) {
        tokens.push_back(item);
    }
    return tokens;
}

