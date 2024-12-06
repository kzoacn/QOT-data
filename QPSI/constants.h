#ifndef _CONFIG_H__
#define _CONFIG_H__ 
namespace qmpc{

const static int AES_BATCH_SIZE = 8;
const static int HASH_BUFFER_SIZE = 1024*8;
const static int NETWORK_BUFFER_SIZE2 = 1024*32;
const static int NETWORK_BUFFER_SIZE = 1024*1024;
const static int FILE_BUFFER_SIZE = 1024*16;
const static int CHECK_BUFFER_SIZE = 1024*8;
const static int LENGTH_LIMIT=1000;

const static int XOR = -1;
const static int PUBLIC = 0;
const static int ALICE = 1;
const static int BOB = 2;

const char fix_key[] = "\x61\x7e\x8d\xa2\xa0\x51\x1e\x96\x5e\x41\xc2\x9b\x15\x3f\xc7\x7a";

const int BIT_WIDTH=4;


const int NUM_BITS=256;
const int NUM_BYTES=NUM_BITS/8;
const int NUM_INT=NUM_BYTES/4;

const int REP=10;
const int OPEN=5;



    const int num_bf=2000;
    const int num_ot=num_bf*3;
    const int num_open=num_bf;
    const int num_k=num_bf;
    double alpha=0.8;
}
#endif// CONFIG_H__
