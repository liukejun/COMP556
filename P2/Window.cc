//
// Created by SHUO ZHAO on 10/15/17.
//
#include <iostream>
#include "Window.h"

Window::Window(const char* file, int w_size, int sock_in, struct sockaddr *si_other_in, socklen_t addr_len_in, int file_path_name_length_in)
        :file_path_name(file), window_size
        (w_size), sock (sock_in), si_other(si_other_in), addr_len(addr_len_in), min_seq_idx(0), is_complete(false), file_path_name_length
        (file_path_name_length_in){
    for (int i = 0; i < window_size; i++){
        slots.push_back(Slot(i));
    }
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

void Window::updateSeqNumber(Slot* slot){
    slot->seq_number += window_size;
}

unsigned short Window::cksum(unsigned short *buf, int count) {
    unsigned long sum = 0;
    while (count--){
        sum += *buf++;
        if (sum & 0xFFFF0000) {
        }}
    return ~(sum & 0xFFFF);
}

