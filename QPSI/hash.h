#ifndef _HASH_H__
#define _HASH_H__

#include "block.h"
#include "constants.h"
#include <openssl/sha.h>
#include <stdio.h>
namespace qmpc{

class Hash { public:
	SHA256_CTX hash;
	char buffer[HASH_BUFFER_SIZE];
	int size = 0;
	static const int DIGEST_SIZE = 32;
	Hash() {
		SHA256_Init(&hash);
	}
	~Hash() {
	}
	void put(const void * data, int nbyte) {
		if (nbyte > HASH_BUFFER_SIZE)
			SHA256_Update(&hash, data, nbyte);
		else if(size + nbyte < HASH_BUFFER_SIZE) {
			memcpy(buffer+size, data, nbyte);
			size+=nbyte;
		} else {
			SHA256_Update(&hash, (char*)buffer, size);
			memcpy(buffer, data, nbyte);
			size = nbyte;
		}
	}
	void put_block(const block* block, int nblock=1){
		put(block, sizeof(block)*nblock);
	}
	void digest(char * a) {
		if(size > 0) {
			SHA256_Update(&hash, (char*)buffer, size);
			size=0;
		}
		SHA256_Final((unsigned char *)a, &hash);
	}
	void reset() {
		SHA256_Init(&hash);
		size=0;
	}
	static void hash_once(void * digest, const void * data, int nbyte) {
		(void )SHA256((const unsigned char *)data, nbyte, (unsigned char *)digest);
	}

	static block hash_for_block(const void * data, int nbyte) {
		block digest;
		hash_once(&digest, data, nbyte);
		return digest; 
	}
	static block hash_for_block(const block &data) {
		block digest;
		hash_once(&digest, &data, sizeof(data));
		return digest; 
	}


};

}


#endif// HASH_H__
