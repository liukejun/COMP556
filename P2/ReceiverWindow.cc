#include "ReceiverWindow.h"

ReceiverWindow::ReceiverWindow(const char* file_path_name, int window_size, int sock, struct sockaddr *si_other, socklen_t addr_len) :
        Window(file_path_name, window_size, sock, si_other, addr_len) {
    ack_buf = (char *) malloc(MIN_PACKET_SIZE);
}


ReceiverWindow::~ReceiverWindow(){
    out_file.close();
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


void ReceiverWindow::receivePacket() {
    int recv_len = recvfrom(sock, (void *) recvBuf, PACKET_SIZE, 0, (struct sockaddr *)si_other, &addr_len);
//    int recv_len = recv(sock, (void *) recvBuf, PACKET_SIZE, 0);
    if (recv_len <= 0) {
        cout << "receiver recvfrom failed" << endl;
        return;
    }

    if (recv_len < PACKET_SIZE) {
        // just discard it.
        cout << "Trying to receive size of " << PACKET_SIZE << " but got " << recv_len << " instead!" << endl;
    } else {
        if (!checkPacket(recvBuf)) {
            cout << "Check failed. Discard" << endl;
        } else {

            int seq_number_in = ntohl(*(((int *) recvBuf) + 1));
            cout << "Got a new valid packet " << seq_number_in<< endl;
            // immediately ack a packet that already been saved into file.
            if (seq_number_in < slots[min_seq_idx].seq_number) {
                cout << seq_number_in << "< " << slots[min_seq_idx].seq_number << ". an ack is sent right away" << endl;
                sendAck(seq_number_in);
                return;
            }
            //
            savePacketToSlot(seq_number_in);
            writeFile();
        }
    }
}


void ReceiverWindow::savePacketToSlot(int seq_number_in) {
    int s_i = getSlotIdx(seq_number_in);
    cout << "getSlotIdx seq_number_in: " << seq_number_in << " s_i: " << s_i  <<endl;
    if (s_i == -1) {
        cout << "could not find the idx" << endl;
        return;
    }
    Slot& cur_slot = slots[s_i];

    // memcpy( void* dest, const void* src, std::size_t count );
    memcpy(cur_slot.slot_buf, recvBuf, PACKET_SIZE);
    cur_slot.slot_status = LOADED;
    cur_slot.printBuf();
}


void ReceiverWindow::writeFile() {
    int last_seq = -1;
//    cout << "min_seq_idx: " << min_seq_idx << endl;
//    cout << "is_complete: " << is_complete << endl;
//
    for (int s_i = min_seq_idx, count = 0; count < window_size && (!is_complete) && slots[s_i].slot_status == LOADED;
         count++, s_i = (s_i + 1) % window_size) {

        Slot& cur_slot = slots[s_i];

        // write in to file
        char data_type = *((char *) cur_slot.slot_buf);
        short data_size = ntohs(*((short *)(cur_slot.slot_buf + 1)));
        cout << "data_type: " << data_type << "data_size: " << data_size << endl;
        if (data_type == '0') {// file name packet
            createFile(s_i);
            int ackNumber = ntohl(*(((int*)cur_slot.slot_buf) + 2));
            sendAck(ackNumber);
        } else if (!file_name.empty()){
            out_file.write(cur_slot.slot_buf + HEADER_SIZE, data_size);
            out_file.flush();
            if (data_type == '2') // last packet
            {
                is_complete = true;
            }
        } else{
            return;
        }
        updateSeqNumber(&cur_slot);
        cur_slot.slot_status = EMPTY;
        last_seq = cur_slot.seq_number;
    }

    if (last_seq > 0) {
        sendAck(last_seq);
    }
}


void ReceiverWindow::createFile(int s_i) {
    cout << "creating file" << endl;
    Slot& slot = slots[s_i];
    short data_length = ntohs(* ((short*)(recvBuf + 1)));
    string file_path_name_s (slot.slot_buf + HEADER_SIZE, data_length);
//    memcpy(file_path_name, slot.slot_buf + HEADER_SIZE, data_length);

    // get path and file name
    string path_name(file_path_name_s);
    vector <string> file_path_vec = split(path_name, '/');
    int file_path_size = (int) file_path_vec.size();
    file_name = file_path_vec[file_path_size - 1];
    file_path = file_path.substr(0, file_path.length() - file_name.length());
    cout << "file path: " + file_path + " name: " << file_name << endl;

    string path = file_path;
    string name = file_name + ".recv";

    out_file.open((path + name).c_str(),std::fstream::out);
    if(!out_file.is_open()){
        //if fail to open the file, exit
        cout << "could not open new file" << endl;
        exit(-1);
    }
    cout << "created file " << path << " "<< name <<endl;
}


// find the corresponding slot index.
int ReceiverWindow::getSlotIdx(int seq_number_in) {
    for (int s_i = min_seq_idx, count = 0; count < window_size; count++, s_i = (s_i + 1) % window_size) {
        if (slots[s_i].seq_number == seq_number_in) {
            return s_i;
        }
    }
    return -1;
}


void ReceiverWindow::sendAck(int seq_number_in) {
    cout << "sedning ack for " << seq_number_in << endl;
    // make ack msg
    makeAck(seq_number_in);
    //send out ack msg
    sendto(sock, ack_buf, MIN_PACKET_SIZE, 0, si_other, addr_len);
}


void ReceiverWindow::makeAck(int ackNum) {
    *((char *) ack_buf + 1) = htons(MIN_PACKET_SIZE - HEADER_SIZE);
    *((int *) ack_buf + 3) = htonl(ackNum);
}


bool ReceiverWindow::checkPacket(char *recvBuf) {
    return true;
//    // get all info
//    unsigned short header_cksum_in = *((short *) ((int *) recvBuf + 4));
//    unsigned short data_cksum_in = *(((short *) recvBuf + 9));
//
//    // check header
//    unsigned short header_cksum = cksum(((u_short *) recvBuf), (HEADER_SIZE - CKSUM_SIZE) / 2);
//
//    // check data
//    unsigned short data_cksum = cksum((u_short *)((int *) recvBuf + 5), (PACKET_SIZE - HEADER_SIZE) / 2);
//
//    return (header_cksum_in == header_cksum) && (data_cksum_in == data_cksum);
}