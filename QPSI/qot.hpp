#ifndef QOT__HPP
#define QOT__HPP

#include "hash.h"
#include "NetIO.hpp"
#include <fstream>
#include <iostream>
#include <vector>
#include "bch_codec.h"
#include <algorithm>
#include <iomanip>
namespace qmpc{
struct Qubit{
    bool basis;
    bool value;
};


class Reader{
public:
    std::ifstream fin;


    Reader(std::string filename){
        fin.open(filename,std::ios::in|std::ios::binary);
    }

    void read_frame(std::vector<Qubit>&qubits){
        int cnt=0;
        unsigned char tmp[4096];
        fin.read((char*)tmp,8);
    
        if(tmp[0]==0xA0 && tmp[1]==0xA1 && tmp[2]==0xA2 && tmp[3]==0xA3){
            
        }else{
            
        }
    
        int length=(1*tmp[4]<<8) | tmp[5];
        length=(length-3)/2;
        fin.read((char*)tmp,4); 
        if(length>100){
            return ;
        }

        for(int i=0;i<length;i++){
            fin.read((char*)tmp,4);
            Qubit qb;
            qb.basis=tmp[3]/2%2;
            qb.value=tmp[3]%2;
            qubits.push_back(qb);
        }
        fin.read((char*)tmp,6);
        return ;
    }
    void add_qubits(vector<Qubit>&res,int num){
        vector<Qubit> tmp;
        int cnt=0;
        while(res.size()<num){
            read_frame(tmp);
            for(auto &x:tmp){
                res.push_back(x); 
            }
        }
    }


};



class QuantumOT{ public:
 

    int party;
    NetIO *io;
    Reader *reader;

    std::vector<unsigned char>basis;
    std::vector<unsigned char>value;

    int cur=0;

    std::vector<Qubit>qubits;
    std::vector<unsigned char>I;
    std::vector<unsigned char>bits[2];


    bch_control * bch;
    static const int m=9,t=30;
    static const int N=(1<<m)-1;
    int msgBits;

    QuantumOT(NetIO *io,int party){

        this->io=io;
        this->party=party;
        reader=new Reader(party==1?"./rawkey/alice/key.bin":"./rawkey/bob/key.bin");

        bch = init_bch(m,t,0);
        msgBits = N - bch->ecc_bits;
    }

    /*
        
            send bits[i]+Encode(ri),H(ri)+mi

            recv a,b
                r=Decode(a-bits[i])
                m=b-H(r) 

    */

    std::vector<Qubit>pool;

    static const int k=8*2048,l=N,nl=4*N;
    Qubit qubit[nl][k];
    unsigned char H[nl][Hash::DIGEST_SIZE];
    bool check[nl];

    bool tbasis[nl][k],tvalue[nl][k];
    void add(){
        
        std::vector<Qubit>tmp;
        
        reader->add_qubits(tmp,nl*k);

        if(party==2){
            for(int i=0;i<nl;i++){
                for(int j=0;j<k;j++){
                    qubit[i][j]=tmp[i+j*nl];
                    tbasis[i][j]=qubit[i][j].basis;
                    tvalue[i][j]=qubit[i][j].value;
                }
            }
            for(int i=0;i<nl;i++){
                Hash::hash_once(H[i],qubit[i],sizeof(qubit[i]));
            }
            for(int i=0;i<nl;i++){
                io->send_data(H[i],Hash::DIGEST_SIZE);
            }
            io->recv_data(check,nl);
            for(int i=0;i<nl;i++)if(check[i]){
                //io->send_data(qubit[i],sizeof(qubit[i]));
                io->send_bool(tbasis[i],k);
                io->send_bool(tvalue[i],k);
            }
            
        }else{
            for(int i=0;i<nl;i++){
                io->recv_data(H[i],Hash::DIGEST_SIZE);
            }
            for(int i=0;i<nl;i++)
                check[i]=i<l?1:0;
            std::random_shuffle(check,check+nl);//TODO
            io->send_data(check,nl);
            
            int yes=0,all=0;

            for(int i=0;i<nl;i++)if(check[i]){
                //io->recv_data(qubit[i],sizeof(qubit[i]));
                io->recv_bool(tbasis[i],k);
                io->recv_bool(tvalue[i],k);
                for(int j=0;j<k;j++){
                    qubit[i][j].basis=tbasis[i][j];
                    qubit[i][j].value=tvalue[i][j];
                }
                unsigned char H2[Hash::DIGEST_SIZE];
                Hash::hash_once(H2,qubit[i],sizeof(qubit[i]));
                if(memcmp(H[i],H2,Hash::DIGEST_SIZE)!=0){
                    puts("cmp no");
                    exit(-1);
                }
                for(int j=0;j<k;j++){
                    Qubit qb1=tmp[i+j*nl];
                    Qubit qb2=qubit[i][j];
                    if(qb1.basis==qb2.basis){
                        all++;
                        if(qb1.value==qb2.value)
                            yes++;
                    }
                }
            }
            if(yes>=all*0.9){
                //yes
                std::cerr<<"rate "<<1.0*yes/all<<std::endl;
            }else{
                puts("no");
                exit(-1);
            }
        }

        for(int j=0;j<k;j++){
            for(int i=0;i<nl;i++)if(!check[i]){    
                pool.push_back(tmp[i+j*nl]);
            }                
        }


        static int CNT=0;
        std::cerr<<"counter "<<++CNT<<std::endl;
    }

    void get_qubits(vector<Qubit>&res,int num){
        while(pool.size()<num)
            add();
        
        res.clear();
        while(num--){
            res.push_back(pool.back());
            pool.pop_back();
        }
    }


	void send_base(unsigned char* data0,unsigned char* data1,int length) {  
        int n=3*N;
        get_qubits(qubits,n);
        basis.resize(n);
        value.resize(n);
        I.resize(n);
        bits[0].clear();
        bits[1].clear();

        for(int i=0;i<n;i++){
            basis[i]=qubits[i].basis;
            value[i]=qubits[i].value;
        }
        io->send_bool((bool*)basis.data(),basis.size());
        io->recv_bool((bool*)I.data(),I.size());
        for(int i=0;i<n;i++)
            bits[I[i]].push_back(qubits[i].value);
    
        for(int j=0;j<2;j++){
            unsigned char r[512];
            unsigned char H[Hash::DIGEST_SIZE];
            unsigned char data[Hash::DIGEST_SIZE];
            
            for(int i=0;i<msgBits;i++)
                r[i]= rand()%2;//TODO

            Hash::hash_once(H,r,msgBits);
            
            for(int i=0;i<length;i++)
                data[i]=H[i]^(j==0?data0[i]:data1[i]);

            encodebits_bch(bch,&r[0],&r[msgBits]);
            for(int i=0;i<N;i++)
                r[i]^=bits[j][i];

            io->send_bool((bool*)r,N+1);
            io->send_data(data,length);
            
            io->flush();
        }
	}

	void send(unsigned char* data0,unsigned char* data1,int length) {  
        bool flag=false;
        int cnt=0;
        do{
            send_base(data0,data1,length);
            io->recv_data(&flag,1);
            if(++cnt>10){
                puts("FAIL TOO MANY TIMES");
                exit(-1);
            }
        }while(!flag);
    }
	bool recv_base(unsigned char* data, const bool b, int length){ 
        bool flag=true;
        int n=3*N;
        get_qubits(qubits,n);
        basis.resize(n); 
        value.resize(n);
        I.resize(n);
        bits[0].clear();
        bits[1].clear();
        
        io->recv_bool((bool*)basis.data(),basis.size());
    
        for(int i=0;i<n;i++){
            int d= basis[i]==qubits[i].basis?1:0;
            if(b==1)
                I[i]=d;
            else
                I[i]=d^1; 
        }

        for(int i=0;i<n;i++)
            bits[I[i]].push_back(qubits[i].value);

        if(bits[0].size()<N || bits[1].size()<N){
            puts("qubits not enough");
            flag=false;
            bits[0].resize(N);
            bits[1].resize(N);
            for(int i=0;i<n;i++)
                I[i]=i%2;
        }

        io->send_bool((bool*)I.data(),I.size());


        for(int j=0;j<2;j++){
            unsigned char r[512];
            unsigned char H[Hash::DIGEST_SIZE]; 
            unsigned char tmp[Hash::DIGEST_SIZE]; 
            
            io->recv_bool((bool*)r,N+1);
            io->recv_data(tmp,length);

            if(j==b){
                for(int i=0;i<N;i++)
                    r[i]^=bits[j][i];
                unsigned int errLocOut[bch->t];
                int nerrFound = decodebits_bch(bch, &r[0], &r[msgBits], &errLocOut[0]);                
                if(nerrFound<0)flag=false;
                correctbits_bch(bch,&r[0],&errLocOut[0],nerrFound);
                Hash::hash_once(H,r,msgBits);
                for(int i=0;i<length;i++)
                    data[i]=H[i]^tmp[i];
            }
        }
        return flag;
	}
	void recv(unsigned char* data, const bool b, int length){ 
        bool flag=false;
        do{
            flag=recv_base(data,b,length);
            io->send_data(&flag,1);
            io->flush();
        }while(!flag);
    }
};
}
#endif