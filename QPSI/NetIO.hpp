#ifndef RECORD_IO_CHANNEL
#define RECORD_IO_CHANNEL

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <vector> 
using std::string;
using std::vector; 

#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/tcp.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include "block.h"
namespace qmpc{

class NetIO { public:
	bool is_server;
	int mysocket = -1;
	int consocket = -1;
	FILE * stream = nullptr;
	char * buffer = nullptr;
	bool has_sent = false;
	string addr;
	int port;
	uint64_t counter = 0;
 

	NetIO(const char * address, int port,bool quiet = false) {
		 

		this->port = port;
		is_server = (address == nullptr);
		if (address == nullptr) {
			struct sockaddr_in dest;
			struct sockaddr_in serv;
			socklen_t socksize = sizeof(struct sockaddr_in);
			memset(&serv, 0, sizeof(serv));
			serv.sin_family = AF_INET;
			serv.sin_addr.s_addr = htonl(INADDR_ANY); /* set our address to any interface */
			serv.sin_port = htons(port);           /* set the server port number */    
			mysocket = socket(AF_INET, SOCK_STREAM, 0);
			int reuse = 1;
			setsockopt(mysocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse, sizeof(reuse));
			if(bind(mysocket, (struct sockaddr *)&serv, sizeof(struct sockaddr)) < 0) {
				perror("error: bind");
				exit(1);
			}
			if(listen(mysocket, 1) < 0) {
				perror("error: listen");
				exit(1);
			}
			consocket = accept(mysocket, (struct sockaddr *)&dest, &socksize);
			close(mysocket);
		}
		else {
			addr = string(address);
			
			struct sockaddr_in dest;
			memset(&dest, 0, sizeof(dest));
			dest.sin_family = AF_INET;
			dest.sin_addr.s_addr = inet_addr(address);
			dest.sin_port = htons(port);

			while(1) {
				consocket = socket(AF_INET, SOCK_STREAM, 0);

				if (connect(consocket, (struct sockaddr *)&dest, sizeof(struct sockaddr)) == 0) {
					break;
				}
				
				close(consocket);
				usleep(1000);
			}
		}
		set_nodelay();
		stream = fdopen(consocket, "wb+");
		const int NETWORK_BUFFER_SIZE=65536;
		buffer = new char[NETWORK_BUFFER_SIZE];
		memset(buffer, 0, NETWORK_BUFFER_SIZE);
		setvbuf(stream, buffer, _IOFBF, NETWORK_BUFFER_SIZE);
		if(!quiet)
			std::cout << "connected\n";
	}

	void sync() {
		int tmp = 0;
		if(is_server) {
			send_data(&tmp, 1);
			recv_data(&tmp, 1);
		} else {
			recv_data(&tmp, 1);
			send_data(&tmp, 1);
			flush();
		}
	}

	~NetIO(){
		fflush(stream);
		close(consocket);
		//delete[] buffer;
	}

	void set_nodelay() {
		const int one=1;
		setsockopt(consocket,IPPROTO_TCP,TCP_NODELAY,&one,sizeof(one));
	}

	void set_delay() {
		const int zero = 0;
		setsockopt(consocket,IPPROTO_TCP,TCP_NODELAY,&zero,sizeof(zero));
	}

	void flush() {
		fflush(stream);
	}
 
    
	void send_data(const void * data, int len) { 
		
        counter += len;
		int sent = 0;
		while(sent < len) {
			int res = fwrite(sent + (char*)data, 1, len - sent, stream);
			if (res >= 0)
				sent+=res;
			else
				fprintf(stderr,"error: net_send_data %d\n", res);
		}
		has_sent = true;
	}

	void recv_data(void  * data, int len) {

        if(has_sent)
			fflush(stream);
		has_sent = false;
		int sent = 0;
		while(sent < len) {
			int res = fread(sent + (char*)data, 1, len - sent, stream);
			if (res >= 0)
				sent += res;
			else 
				fprintf(stderr,"error: net_send_data %d\n", res);
		} 
	}


	void send_bool(const bool *data,int len){
		//assert(len%8==0);
		unsigned char *d=new unsigned char[len/8];
		for(int i=0;i<len/8;i++)
			d[i]=0;
		for(int i=0;i<len;i++){
			int id=i/8;
			d[id]|=(data[i]?1:0)<<(i%8);
		}
		send_data(d,len/8);
		delete[] d;
	}
	void recv_bool(bool *data,int len){
		unsigned char *d=new unsigned char[len/8];
		recv_data(d,len/8);
		for(int i=0;i<len;i++){
			int id=i/8;
			data[i]=d[id]>>(i%8)&1;
		}
		delete[] d;
	}


	template<class T>
	void send_vec(std::vector<T> &x){
		int sz=x.size();
		send_data(&sz,sizeof(sz));
		send_data(x.data(),sizeof(T)*sz);
	}
	template<class T>
	void recv_vec(std::vector<T> &x){
		int sz;
		recv_data(&sz,sizeof(sz));
		x.resize(sz);
		recv_data(x.data(),sizeof(T)*sz); 	
	}


	void send_block(block *data,int len){
		send_data(data,sizeof(block)*len);
	}
	void recv_block(block *data,int len){
		recv_data(data,sizeof(block)*len);
	}

	void send_string(string s){
		int size=s.length();
		send_data(&size,4);
		send_data(s.data(),size);
	}
	void recv_string(string &s){
		int size;
		recv_data(&size,4);
		char *tmp=new char[size+1];
		recv_data(tmp,size);
		s=string(tmp);
		delete[] tmp;
	}
}; 

}
#endif  //NETWORK_IO_CHANNEL
