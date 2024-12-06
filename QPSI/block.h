#ifndef BLOCK__
#define BLOCK__

#include<cstring>
#include "utils.hpp"

namespace qmpc{

const long long MOD=(1LL<<31)-1;

class block{ public:
    unsigned int blk[NUM_INT];

    block(){
        memset(blk,0,sizeof(blk));
    }

    unsigned int& operator[](int x){
        return blk[x];
    }
    unsigned int operator[](int x)const{
        return blk[x];
    }


    block operator^(const block &rhs)const{
        block c;
        for(int i=0;i<NUM_INT;i++)
            c[i]=blk[i]^rhs[i];
        return c;
    }

    block operator&(const block &rhs)const{
        block c;
        for(int i=0;i<NUM_INT;i++)
            c[i]=blk[i]&rhs[i];
        return c;
    }
    bool operator==(const block &rhs)const{
        bool c=true;
        for(int i=0;i<NUM_INT;i++)
            c&=(blk[i]==rhs[i]);
        return c;
    }

    block operator+(const block &rhs)const{
        block c;
        for(int i=0;i<NUM_INT;i++)
            c[i]=((long long)blk[i]+rhs[i])%MOD;
        return c;
    }
    block operator-(const block &rhs)const{
        block c;
        for(int i=0;i<NUM_INT;i++)
            c[i]=((long long)blk[i]+(MOD-rhs[i]))%MOD;
        return c;
    }
    void mod(){
        for(int i=0;i<NUM_INT;i++)
            blk[i]%=MOD;
    }

    block inv(){
        block c;
        for(int i=0;i<NUM_INT;i++){
            c[i]=pw(blk[i],MOD-2,MOD);
        }
        return c;
    }

};

block operator*(const int &lhs,const block &rhs){
    block c;
    for(int i=0;i<NUM_INT;i++)
        c[i]=((long long)lhs*rhs[i]%MOD+MOD)%MOD;
    return c;
}

block makeBlock(unsigned long long v0,unsigned long long v1,unsigned long long v2,unsigned long long v3){
    unsigned long long v[4];
    v[0]=v0;
    v[1]=v1;
    v[2]=v2;
    v[3]=v3;
    block b;
    memcpy(&b,v,sizeof(block));
    return b;
}


inline bool getLSB(const block & x) {
	return (x[0] & 1) == 1;
}
  
const block zero_block = makeBlock(0, 0 ,0 ,0);
const block all_one_block = makeBlock(0xFFFFFFFFFFFFFFFF,0xFFFFFFFFFFFFFFFF,0xFFFFFFFFFFFFFFFF,0xFFFFFFFFFFFFFFFF);
const block select_mask[2] = {zero_block, all_one_block};


}

#endif