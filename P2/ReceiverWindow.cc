//
// Created by SHUO ZHAO on 10/15/17.
//
#include <ReceiverWindow.h>


SenderWindow::SenderWindow(char *file_path_name, int window_size, int sock, struct sockaddr *si_other, socklen_t addr_len) :
        SuperClass(file_path_name, window_size, sock, sockaddr * si_other, addr_len) {
    ack_buf = (char*) alloc(MIN_PACKET_SIZE);
    //    /* get path and file name */
//    vector <string> file_path_vec = split(file_path, '/');
//    int file_path_size = (int) file_path_vec.size();
//    file_name = file_path_vec[file_path_size - 1];
//    file_path = file_path.substr(0, file_path.length() - fileName.length());
}


vector <string> ReceiverWindow::split(const string &s, char delim) {
    stringstream ss(s);
    string item;
    vector <string> tokens;
    while (getline(ss, item, delim)) {
        tokens.push_back(item);
    }
    return tokens;
}

int ReceiverWindow::createFile(string file) {
    if (file.length() == 0) {
        cout << "Path and file name is empty" << "\n";
        return -1;
    } else {
        vector <string> pathFile = split(file, ' ');
        string path = pathFile[0];
        string name = pathFile[1] + ".recv";
        int status = mkdir(path.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
        ofstream file(path + name);
        string data("data to write to file");
        file << data;
        return 0;
    }
}

void ReceiverWindow::receivePacket() {
    int recv_len = recvfrom(sock, recvBuf, PACKET_SIZE, 0, (struct sockaddr *) si_other, addr_len);
    if (recv_len < 0) {
        cout << "receiver recvfrom failed" << endl;
        exit(-1);
    }
    if (recv_len < PACKET_SIZE) {//ack packet size is set to the MIN_PACKET_SIZE
        // just discard it.
        cout << "Trying to receive size of " << PACKET_SIZE << " but got " << recv_len << " instead!" << endl;

    } else {
        if (!checkPacket(recvBuf)) {
            cout << "Corrupt file. Discard" << endl;
        } else {
            int seq_number_in = ntohl(*(((int *) recvBuf) + 2));
            // immediately ack a packet that already been saved into file.
            if (seq_number_in < slots[min_seq_idx].seq_number) {
                sentAck(seq_number_in);
                return;
            }
            //
            savePacketToFile(seq_number_in);
            writeFile();
            sentAcks();
        }
    }
}

// find the corresponding slot index.
int ReceiverWindow::getSlotIdx(int seq_number_in){
    for (int s_i = min_seq_idx, count = 0; count < window_size; count++, s_i = (s_i + 1) % window_size) {
        if (slots[s_i].seq_number == seq_number_in){
            return s_i;
        }
    }
    return -1;
}

void ReceiverWindow::savePacketToFile(seq_number_in){
    int s_i = getSlotIdx(seq_number_in);
    Slot cur_slot = slots[s_i];
//    memcpy( void* dest, const void* src, std::size_t count );
    memcpy( cur_slot.slot_buf, recvBuf, PACKET_SIZE);

}
void ReceiverWindow::sendAck(int seq_number_in) {
    // make ack msg
    makeAck(seq_number_in);
    //send out ack msg
    sendto(sock, ack_buf, MIN_PACKET_SIZE, 0, (struct sockaddr *) si_other, addr_len);
}

void makeAck(int ackNum){
    *((char *)ack_buf + 1) = htons(MIN_PACKET_SIZE - HEADER_SIZE);
    *((int *)ack_buf + 3) = htonl(ack_number);
}

bool ReceiverWindow::checkPacket(char *recvBuf) {
    // get all info
    unsigned_short header_cksum_in = *((short *) ((int *) slot_buf + 4));
    unsigned_short data_cksum_in = *(((short *) slot_buf + 9));

    // check header
    unsigned_short header_cksum = cksum(((u_short *) recvBuf), (HEADER_SIZE - CKSUM_SIZE) / 2);

    // check data
    unsigned_short data_cksum = cksum((u_short * )((int *) slot_buf + 5), (PACKET_SIZE - HEADER_SIZE) / 2);

    return (header_cksum_in == header_cksum) && (data_cksum_in == data_cksum);
}