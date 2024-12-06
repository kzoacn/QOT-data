#ifndef _PRG_H__
#define _PRG_H__
#include "block.h"
#include "constants.h"
#include <memory>
#include <random>
#include "hash.h" 
 
namespace qmpc{
class PRG { public:
	uint64_t counter = 0; 
	block key;
	PRG(const void * seed = nullptr, int id = 0) {	
		
		if (seed != nullptr) {
			reseed((const block *)seed, id);
		} else {
			block v; 
			uint32_t * data = (uint32_t *)(&v);
			std::random_device rand_div;
			for (size_t i = 0; i < sizeof(block) / sizeof(uint32_t); ++i)
				data[i] = rand_div(); 
			reseed(&v);
		}
	}
	void reseed(const block* seed, uint64_t id = 0) {
		key=*seed;
		counter = 0;
	}

	void random_data(void *data, int nbytes) {
		random_block((block *)data, nbytes/32);
		if (nbytes % 32 != 0) {
			block extra;
			random_block(&extra, 1);
			memcpy((nbytes/32*32)+(char *) data, &extra, nbytes%32);
		}
	}
	void random_bool(bool * data, int length) {
		uint8_t * uint_data = (uint8_t*)data;
		random_data(uint_data, length);
		for(int i = 0; i < length; ++i)
			data[i] = uint_data[i] & 1;
	}

	void random_block(block * data, int nblocks=1) {
		for (int i = 0; i < nblocks; ++i) {
			data[i] = key ^ makeBlock(counter++ ,0 ,0 ,0);
		}
		for (int i = 0; i < nblocks; ++i) 
			data[i]=Hash::hash_for_block(&data[i],sizeof(block));
	}
};

}

#endif// PRP_H__
