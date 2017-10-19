#include<iostream>
#include<fstream>

using namespace std;

int main()
{
    ifstream is;
    if(is)
    {
        is.seekg(0, is.end);
        is.open("/Users/xiajunru/Code/COMP556/P1/project1_submitted/root/John1.txt");
        
        if (is.is_open()) {
            cout << "file opened " << endl;
            char * buffer = new char [1000];
        
            is.read(buffer, 0);
        
            cout << "buffer: " << buffer<<endl;
        
            is.close();
        
        cout << buffer << endl;
        
        delete [] buffer;
        }
    }
    return 0;
}
