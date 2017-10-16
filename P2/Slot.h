//
// Created by SHUO ZHAO on 10/15/17.
//

#ifndef COMP_556_NETWORK_SLOT_H
#define COMP_556_NETWORK_SLOT_H

#include <global.h>
class Slot{
public:
    Slot();
    ~Slot();
    SlotType slot_type;
    SlotStatus slot_status;
    struct timeval sent_time;
    int seq_number;
    int ack_number;
    short data_length;
    int file_position;
    int ack_number;
    char* slot_buf;
    int window_size; // number of slots in a window

    void setHeader();
    unsigned_short cksum(u_short *buf, int count);
};
#endif //COMP_556_NETWORK_SLOT_H
