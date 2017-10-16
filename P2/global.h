//
// Created by SHUO ZHAO on 10/15/17.
//

#ifndef COMP_556_NETWORK_GLOBAL_H
#define COMP_556_NETWORK_GLOBAL_H
// define all global constans
#define MIN_PACKET_SIZE 64  // in bytes
#define PACKET_SIZE 1500
#define HEADER_SIZE 20
#define CKSUM_SIZE 4
#define WINDOW_SIZE 15
#define TIMEOUT  5000
#define  LAST_PACKET_RESENT_TIME 20

enum SlotType{FIRST, NORMAL, LAST};
enum SlotStatus{EMPTY, LOADED, SENT};
enum WindowStatus{NORMAL, LAST};
#endif //COMP_556_NETWORK_GLOBAL_H
