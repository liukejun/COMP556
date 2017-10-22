//
// Created by SHUO ZHAO on 10/15/17.
//

#ifndef COMP_556_NETWORK_SLOT_H
#define COMP_556_NETWORK_SLOT_H

#include "global.h"
#include <sys/socket.h>
#include <string>
#include <sstream>
#include <iostream>
#include <sys/time.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>

class Slot{
public:
    Slot();
    Slot(int init_seq);
    ~Slot();
    void setHeader();
    unsigned short cksum(unsigned short *buf, int count); // use 1's complement to calculate checksum
    void setLoadedStatus(short data_size_in, SlotType slot_type_in) ;
    void setSentStatus();
    void setSentTime(struct timeval new_time);

    SlotType slot_type;// enum SlotType{FIRST, NORMAL, LAST};
    SlotStatus slot_status; //enum SlotStatus{EMPTY, LOADED, SENT};
    struct timeval sent_time; // used for keeping track of
    int seq_number;
    int ack_number;
    short data_length;
    char* slot_buf;
    int resent_times;
    int sent_size;
};
#endif //COMP_556_NETWORK_SLOT_H
