#ifndef FAKEOT__HPP
#define FAKEOT__HPP

#include "hash.h"
#include "NetIO.hpp"
#include <fstream>
#include <iostream>
#include <vector>
#include <algorithm>
#include <iomanip>
namespace qmpc{

class OT{ public:
 

    int party;
    NetIO *io;
    
    OT(NetIO *io,int party){

        this->io=io;
        this->party=party;
    }

    
	void send(unsigned char* data0,unsigned char* data1,int length) {  
        io->send_data(data0,length);
        io->send_data(data1,length);
    }
	void recv(unsigned char* data, const bool b, int length){ 
        unsigned char *tmp[2];
        tmp[0]=new unsigned char[length];
        tmp[1]=new unsigned char[length];

        io->recv_data(tmp[0],length);
        io->recv_data(tmp[1],length);
        memcpy(data,tmp[b],length);
    }
};
}
#endif