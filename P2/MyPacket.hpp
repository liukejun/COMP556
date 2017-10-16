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
    char * data;
    
public:
    // 0: directory, 1: fileName, 2: content, 3: ACK
    
    char *buffer;
    
    MyPacket(int type, int seq_num, int window_size, int data_length, short checksum, char* data);
    
    void clear();
    
    int getType();
    
    int getSeqNum();
    
    int getWinSize();
    
    int getDataLength();
    
    short getCheckSum();
    
    char* getData();
    
    char* getBuf();
    
    void deserialize(char * buf);
    
    ~MyPacket();
    
};