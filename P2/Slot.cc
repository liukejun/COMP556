//
// Created by SHUO ZHAO on 10/15/17.
//
#include <Slot.h>
Slot::Slot(){
}
Slot::Slot(int win_size, int init_seq): window_size(win_size), seq_number(init_seq){
    slot_buf = (char*) alloc(PACKET_SIZE);
}

slot::~Slot(){
    free(slot_buf);
}

void Slot::updateSeqNumber(){
    seq_number += window_size;
}
unsigned_short Slot::cksum(u_short *buf, int count) {
    unsigned_long sum = 0;
    while (count--){
        sum += *buf++;
        if (sum & 0xFFFF0000) {
        }}
    return ~(sum & 0xFFFF);
}

void Slot::setHeader(){
    //data_type
    switch(slot_type){
        case(FIRST):
            *((char *)slot_buf) = 0;
            break;
        case(NORMAL):
            *((char *)slot_buf) = 1;
            break;
        case(LAST):
            *((char *)slot_buf) = 2;
            break;
    }
    // data_length
    *((char *)slot_buf + 1) = htons(data_length);
    // header_size
    *((int *)slot_buf + 1) = htonl(file_position);
    *((int *)slot_buf + 2) = htonl(seq_number);
    *((int *)slot_buf + 3) = htonl(ack_number);
    // checksum
    *((int *)slot_buf + 4) = htons(cksum((u_short*) slot_buf, data_length + header_size));
}


