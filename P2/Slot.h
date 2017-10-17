//
// Created by SHUO ZHAO on 10/15/17.
//

#ifndef COMP_556_NETWORK_SLOT_H
#define COMP_556_NETWORK_SLOT_H

#include "global.h"
class Slot{
public:
    Slot();
    Slot(int win_size, int init_seq);
    ~Slot();
    void setHeader();
    unsigned_short cksum(u_short *buf, int count);
    void setLoadedStatus(short data_size_in, SlotType slot_type_in, SlotStatus slot_status_in, int file_position_in) ;
    void setSentStatus();
    void setSentTime(struct timeval new_time);
    void updateSeqNumber();


    SlotType slot_type;// enum SlotType{FIRST, NORMAL, LAST};
    SlotStatus slot_status; //enum SlotStatus{EMPTY, LOADED, SENT};
    struct timeval sent_time;
    int seq_number;
    int ack_number;
    short data_length;
    int ack_number;
    char* slot_buf;
    int window_size; // number of slots in a window
    int resent_time;
    int sent_size;
};
#endif //COMP_556_NETWORK_SLOT_H
