//
// Created by SHUO ZHAO on 10/15/17.
//

#ifndef COMP_556_NETWORK_WINDOW_H
#define COMP_556_NETWORK_WINDOW_H
#include <vector>
#include <global.h>
#include <Slot.h>
using namespace std;
class Window{
public:
//    Window(int window_size);
    Window(char* file_path, int window_size);
    int window_size;
    char* file_path;
    bool is_complete;
    unsigned_short cksum(u_short *buf, int count);
    vector<Slot> slots;

};
#endif //COMP_556_NETWORK_WINDOW_H
