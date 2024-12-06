#ifndef UTILS__
#define UTILS__

#include<cstdlib>
#include "constants.h"

namespace qmpc{

inline void parse_party_and_port(char ** arg, int * party, int * port) {
	*party = atoi (arg[1]);
	*port = atoi (arg[2]);
}

template <typename T>
inline T bool_to_int(const bool *data) {
    T ret {};
    for (size_t i = 0; i < sizeof(T)*8; ++i) {
        T s {data[i]};
        s <<= i;
        ret |= s;
    }
    return ret;
}

template<typename T>
inline void int_to_bool(bool * data, T input, int len) {
	for (int i = 0; i < len; ++i) {
		data[i] = (input & 1)==1;
		input >>= 1;
	}
}


template<typename T>
inline void block_to_bool(bool * data, T input) {
	for (int i = 0; i < NUM_INT; ++i) {
        int_to_bool(data+i*32,(unsigned int)input[i],32);
	}
}


template <typename T>
inline T bool_to_block(const bool *data) {
    T ret {};
    for (int i = 0; i < NUM_INT; ++i) {
        ret[i]=bool_to_int<unsigned int>(data+i*32);
	}
    return ret;
}

long long pw(long long x,long long k,long long p){
    long long ans=1;
    for(;k;k>>=1){
        if(k&1)ans=ans*x%p;
        x=x*x%p;
    }
    return ans;
}

}
#endif