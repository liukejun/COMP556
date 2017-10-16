//
// Created by SHUO ZHAO on 10/15/17.
//

SenderWindow::SenderWindow(char *file_path_name, int window_size, int sock, struct sockaddr *si_other, socklen_t addr_len) :
        SuperClass(file_path_name, window_size, sock, sockaddr * si_other, addr_len) {
    // open the file
    in_file.open(file_path_name);
    if (!in_file) {
        cout << "Unable to open file" << file_path_name << endl;
        exit(-1);   // call system to stop
    }

    // get length of file:
    in_file.seekg(0, in_file.end);
    file_length = is.tellg();
    in_file.seekg(0, in_file.beg);

    loadFileName();
}

void SenderWindow::sendPendingPackets() {

    for (int s_i = min_seq_idx, count = 0; count < window_size && (!is_complete); count++, s_i = (s_i + 1) % window_size) {
        {
            Slot cur_slot = slots[s_i];
            // load file
            if (cur_slot.slot_status == EMPTY) {
                in_file.read(cur_slot.slot_buf + HEADER_SIZE, PACKET_SIZE - HEADER_SIZE);
                streamsize size = in_file.gcount();
                setLoadedStatus(size, NORMAL, LOADED, file_pos);
                file_pos = in_file.tellg();

                // check if file reaches the end
                if (file.eof()) {
                    file_pos = file_length;
                    cur_slot.slot_type = LAST;
                    window_status = LAST;
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
                if (window_status == LAST) {
                    cur_slot.resent_time++;
                    if (cur_slot.resent_time > LAST_PACKET_RESENT_TIME) {
                        is_complete = true;
                        cur_slot.slot_status = SENT;
                    }
                }
            }

            // send out packets
            if (cur_slot.slot_status == LOADED) {
                int sent = sendto(sock, cur_slot.slot_buf, PACKET_SIZE, 0, (struct sockaddr *) si_other, addr_len);
                if (send == PACKET_SIZE) {
                    cur_slot.setSentStatus();
                } else {
                    cout << "Trying to send size of " << PACKET_SIZE << " but sent " << sent << " instead!" << endl;
                }
            }
        }
    }
}


void loadFileName() {
    // load it to the first slot
    Slot first_slot = slots[0];
    first_slot.file_position = -1;
    first_slot.slot_type = FIRST;

    // Save the file path in the packet data portion.
    first_slot.data_length = strlen(file_path_name) + 1;
    strcpy(first_slot.slot_buf + HEADER_SIZE, file_path_name);
    first_slot.slot_status = LOADED;
}

void SenderWindow::recievePacket() {
    //try to receive some data, this is a blocking call
    int recv_len = recvfrom(sock, recvBuf, PACKET_SIZE, 0, (struct sockaddr *) si_other, addr_len);
    if (recv_len < 0) {
        cout << "recvfrom failed" << endl;
        exit(-1);
    }
    if (recv_len == MIN_PACKET_SIZE){//ack packet size is set to the MIN_PACKET_SIZE
        if (checkPacket(recvBuf)){
           int ackNumber =  ntohl(((int*)&recvBuf) + 3);
            handleAck(ackNumber);
        } else {
            cout << "Corrupt file. Discard" << endl;
        }
    } else {
        cout << "Trying to receive size of " << MIN_PACKET_SIZE << " but got " << sent << " instead!" << endl;
    }
}

void SenderWindow::handleAck(int ackNumber) {
    for (int s_i = min_seq_idx, count = 0; count < window_size && slots[s_i].seq_number < ackNumber; count++, s_i = (s_i + 1) % window_size) {
        Slot cur_slot = slots[s_i];
        cur_slot.slot_status = EMPTY;
        cur_slot.updateSeqNumber();
    }
    min_seq_idx = s_i;
    cout << "packets less than " << min_seq_idx << " has all be acked" << endl;
}

bool SenderWindow::checkPacket(char* recvBuf){
    // get all info
    unsigned_short header_cksum_in = *((short*)((int *)slot_buf + 4));
//    unsigned_short data_cksum_in = *(((short *)slot_buf + 9));

    // check header
    unsigned_short header_cksum = cksum(((u_short* )recvBuf), (HEADER_SIZE - CKSUM_SIZE) / 2);

    // check data
//    unsigned_short data_cksum = cksum((u_short* )((int *)slot_buf + 5), (PACKET_SIZE - HEADER_SIZE) / 2);

    return (header_cksum_in == header_cksum);
}