//
// Created by SHUO ZHAO on 10/15/17.
//
#include <Window.h>

Window::Window(char* file, int w_size):file_path(file), window_size(w_size),slots(w_size){

}

unsigned_short Window::cksum(u_short *buf, int count) {
    unsigned_long sum = 0;
    while (count--){
        sum += *buf++;
        if (sum & 0xFFFF0000) {
        }}
    return ~(sum & 0xFFFF);
}