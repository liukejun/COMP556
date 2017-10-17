#include <memory>
#include <time.h>
using namespace std;
//class MyPacket;
//typedef shared_ptr <MyPacket> MyPacketPtr;

class MyPacket {
private:
    int type;
    int seq_num;
    int window_size;
    int data_length;
    unsigned long checksum;
    string data;
    
public:
    // 0: directory filename, 1: content, 2: ACK, 3: END
    
    char *buffer;
    
    bool acked = false;
    
    struct timeval latestSendTime;
    
    MyPacket(int type, int seq_num, int window_size, int data_length, unsigned long checksum, string data);
    
    void clear();
    
    int getType();
    
    int getSeqNum();
    
    int getWinSize();
    
    int getDataLength();
    
    unsigned long getCheckSum();
    
    string getData();
    
    char* getBuf();
    
    unsigned long computeChecksum();
    
    void displayContent();
    
//    void deserialize(char * buf);
    
//    ~MyPacket();
    
};
