//
// Created by SHUO ZHAO on 10/15/17.
//

#ifndef COMP_556_NETWORK_GLOBAL_H
#define COMP_556_NETWORK_GLOBAL_H
// define all global constans
#define MIN_PACKET_SIZE 64  // in bytes
#define PACKET_SIZE 1500
#define HEADER_SIZE 16
#define CKSUM_SIZE 4
#define WINDOW_SIZE 5
#define TIMEOUT  5000
#define LAST_PACKET_RESENT_TIME 20

enum SlotType {FIRST, NORMAL, LAST};
enum SlotStatus{EMPTY, LOADED, SENT, WROTE};
enum WindowStatus{WORKING, ENDDING};
#endif //COMP_556_NETWORK_GLOBAL_H
