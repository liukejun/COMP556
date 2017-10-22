#include "SenderWindow.h"


SenderWindow::SenderWindow(const char *file_path_name, int window_size, int sock, struct sockaddr *si_other, socklen_t addr_len) :
        Window(file_path_name, window_size, sock, si_other, addr_len) {
    // open the file
    in_file.open(file_path_name);
    if (!in_file) {
        cout << "Unable to open file" << file_path_name << endl;
        exit(-1);   // call system to stop
    }

    // get length of file
    in_file.seekg(0, in_file.end);
    file_length = in_file.tellg();
    in_file.seekg(0, in_file.beg);

    // load file name into first slot
    loadFileName();
}


SenderWindow::~SenderWindow(){
    in_file.close();
}


void SenderWindow::sendPendingPackets() {
    for (int s_i = min_seq_idx, count = 0; count < window_size && (!is_complete); count++, s_i = (s_i + 1) % window_size) {
        {
            Slot& cur_slot = slots[s_i];
            // load file
            if (cur_slot.slot_status == EMPTY) {
                in_file.read(cur_slot.slot_buf + HEADER_SIZE, PACKET_SIZE - HEADER_SIZE);
                streamsize size = in_file.gcount();
                cur_slot.setLoadedStatus(size, NORMAL);

                // check if file reaches the end
                if (in_file.eof()) {
                    cur_slot.slot_type = LAST;
                    window_status = ENDDING;
                }
                // set header in packet
                cur_slot.setHeader();
            }

            // reset packet that takes too much time
            struct timeval now;
            gettimeofday(&now, 0);
            long elapsed = (now.tv_sec - cur_slot.sent_time.tv_sec) * 1000000 + (now.tv_usec - cur_slot.sent_time.tv_usec);
            if (elapsed >= TIMEOUT) {
                cur_slot.slot_status = LOADED;
                if (window_status == ENDDING) {
                    cur_slot.resent_times++;
                    if (cur_slot.resent_times > LAST_PACKET_RESENT_TIME) {
                        is_complete = true;
                        cur_slot.slot_status = SENT;
                    }
                }
            }

            // send out packets
            if (cur_slot.slot_status == LOADED) {
                cout << "trying to send packet " << cur_slot.seq_number << endl;
                int sent = sendto(sock, cur_slot.slot_buf, PACKET_SIZE, 0, (struct sockaddr *) si_other, addr_len);
                if (sent == PACKET_SIZE) {
                    cur_slot.setSentStatus();
                } else {
                    cout << "Trying to send size of " << PACKET_SIZE << " but sent " << sent << " instead!" << endl;
                }
            }
        }
    }
}


void SenderWindow::loadFileName() {
    // load it to the first slot
    Slot& first_slot = slots[0];
    first_slot.slot_type = FIRST;

    // Save the file path in the packet data portion.
    first_slot.data_length = strlen(file_path_name) + 1;
    strcpy(first_slot.slot_buf + HEADER_SIZE, file_path_name);
    first_slot.slot_status = LOADED;
}

void SenderWindow::recievePacket() {
    //try to receive some data, this is a blocking call
    int recv_len = recvfrom(sock, recvBuf, PACKET_SIZE, 0, (struct sockaddr *) si_other, &addr_len); // recieve ack msg
    if (recv_len < 0) {
        cout << "sender recvfrom failed" << endl;
        exit(-1);
    }
    cout << "got an ack" << endl;
    if (recv_len == MIN_PACKET_SIZE){//ack packet size is set to the MIN_PACKET_SIZE
        if (checkPacket(recvBuf)){
            int ackNumber = ntohl(*(((int*)recvBuf) + 2));
            cout << "ack number: " << ackNumber <<endl;
            handleAck(ackNumber);
        } else {
            cout << "Check failed. Discard..." << endl;
        }
    } else {
        cout << "Trying to receive size of " << MIN_PACKET_SIZE << " but got " << recv_len << " instead!" << endl;
    }
}


void SenderWindow::handleAck(int ackNumber) {
    int s_i = min_seq_idx;
    for (int count = 0; count < window_size && slots[s_i].seq_number <= ackNumber; count++, s_i = (s_i + 1) % window_size) {
        Slot& cur_slot = slots[s_i];
        cur_slot.slot_status = EMPTY;
        updateSeqNumber(cur_slot);
    }

    min_seq_idx = s_i;
    cout << "packets seq number <= " << slots[min_seq_idx].seq_number << " has all be acked" << endl;
}


bool SenderWindow::checkPacket(char* recvBuf){
    return true;
//    // get all info
//    unsigned short header_cksum_in = *((short*)((int *)recvBuf + 3));
//
//    // check header
//    unsigned short header_cksum = cksum(((unsigned short* )recvBuf), (HEADER_SIZE - CKSUM_SIZE) / 2);
//
//    return (header_cksum_in == header_cksum);
}