//
// Created by SHUO ZHAO on 10/15/17.
//
#include "Slot.h"

Slot::Slot(){
}
Slot::Slot(int init_seq): seq_number(init_seq), resent_times(0){
    slot_buf = (char*) malloc(PACKET_SIZE);
}

Slot::~Slot(){
//    free(slot_buf);
}

unsigned short Slot::cksum(unsigned short *buf, int count) {
    unsigned long sum = 0;
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
    *((int *)slot_buf + 1) = htonl(seq_number);
    *((int *)slot_buf + 2) = htonl(ack_number);
    // checksum
    *((int *)slot_buf + 3) = htons(cksum(((unsigned short* )slot_buf), (HEADER_SIZE - CKSUM_SIZE) / 2)); // add checksum for header portion
    // add checksum for data portion. Packet size isconstant by padding extra 0
    *((short *)slot_buf + 7) = htons(cksum(((unsigned short* )((int *)slot_buf + 5)), (PACKET_SIZE - HEADER_SIZE) / 2));
}

void Slot::setLoadedStatus(short data_size_in, SlotType slot_type_in ) {
    data_length = data_size_in;
    slot_type = slot_type_in;
    slot_status = LOADED;
}
void Slot::setSentStatus(){
    struct timeval now;
    gettimeofday(&now,0);
    setSentTime(now);
    slot_status = SENT;
}
void Slot::setSentTime(struct timeval new_time){
    sent_time.tv_sec = new_time.tv_sec;
    sent_time.tv_usec = new_time.tv_usec;
}


