

using namespace std;
class MyPacket;
typedef shared_ptr <MyPacket> MyPacketPtr;

class MyPacket {
private:
    int type;
    int seq_num;
    int window_size;
    int data_length;
    short checksum;
    char* data;
    
public:
    // 0: directory, 1: fileName, 2: content, 3: ACK
    
    char *buffer;
    
    MyPacket(int type, int seq_num, int window_size, int data_length, short checksum, char* data) {
        this->type = type;
        this->seq_num = seq_num;
        this->window_size = window_size;
        this->data_length = data_length;
        this->checksum = checksum;
        this->data = data;
        this->buffer = (char *) malloc(1018);
        *(int*)buffer = (int)htonl(type);
        *(int*)(buffer + 4) = (int)htonl(seq_num);
        *(int*)(buffer + 8) = (int)htonl(window_size);
        *(int*)(buffer + 12) = (int)htonl(data_length);
        *(short*)(buffer + 16) = (short)htons(checksum);
        *(char*)(buffer + 18) = (char) *data;
    }
    
    void clear() {
        memset(buffer, 0, data_length + 18);
        memset(data, 0, data_length);
        free(buffer);
        free(data);
    }
    
    int getType() {
        return this->type;
    }
    
    int getSeqNum() {
        return this->seq_num;
    }
    
    int getWinSize() {
        return this->window_size;
    }
    
    int getDataLength() {
        return this->data_length;
    }
    
    short getCheckSum() {
        return this->checksum;
    }
    
    char* getData() {
        return this->data;
    }
    
    char* getBuf() {
        return this->buffer;
    }
    
    void deserialize(char * buf) {
        this->type = (int) ntohl(*(int*)(buf));
        this->seq_num = (int) ntohl(*(int*)(buf + 4));
        this->window_size = (int) ntohl(*(int*)(buf + 8));
        this->data_length = (int) ntohl(*(int*)(buf + 12));
        this->checksum = (short) ntohs(*(short*)(buf + 16));
        this->data = (char*)(buf + 18);
    }
    
    
};
