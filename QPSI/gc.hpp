#ifndef QMPC_CIRCUIT_EXECUTION_H
#define QMPC_CIRCUIT_EXECUTION_H

#include "prg.h"
#include "block.h"
#include "NetIO.hpp"
#include <algorithm>

namespace qmpc{

using std::swap;

class CircuitExecution { public:
	static CircuitExecution * circ_exec;
	virtual block and_gate(const block& in1, const block& in2) = 0;
	virtual block xor_gate(const block&in1, const block&in2) = 0;
	virtual block not_gate(const block& in1) = 0;
	virtual block public_label(bool b) = 0;
	virtual ~CircuitExecution (){ }
};


inline block halfgates_garble(block LA0, block A1, block LB0, block B1, block delta, block *table) {
	bool pa = getLSB(LA0);
	bool pb = getLSB(LB0);

	block C0;
	PRG().random_block(&C0, 1);
	block C1=C0^delta;

	block HA0=Hash::hash_for_block(LA0);
	block HA1=Hash::hash_for_block(A1);

	table[0] = Hash::hash_for_block(HA0^LB0)^C0;
	table[1] = Hash::hash_for_block(HA1^LB0)^C0;
	table[2] = Hash::hash_for_block(HA0^B1)^C0;
	table[3] = Hash::hash_for_block(HA1^B1)^C1;

	if(pa){
		swap(table[0],table[1]);
		swap(table[2],table[3]);
	}
	if(pb){
		swap(table[0],table[2]);
		swap(table[1],table[3]);
	}

	return C0;
}

inline block halfgates_eval(block A, block B, const block *table) {
	int sa, sb;

	sa = getLSB(A);
	sb = getLSB(B);

	int idx = sa+2*sb;
	block C;
	C = table[idx]^Hash::hash_for_block(Hash::hash_for_block(A)^B);

	return C;
}


template<typename T>
class HalfGateGen:public CircuitExecution {
public:
	block delta;
	T * io;
	block constant[2]; 
	HalfGateGen(T * io) :io(io) {
		block tmp[2];
		PRG().random_block(tmp, 2);
		set_delta(tmp[0]);
		io->send_block(tmp+1, 1); 
	}
	void set_delta(const block & _delta) {
		//delta = set_bit(_delta, 0);
		delta=_delta;
		if(!getLSB(delta))
			delta[0]^=1;
		PRG().random_block(constant, 2);
		io->send_block(constant, 2);
		constant[1] = constant[1] ^ delta;
	}
	block public_label(bool b) override {
		return constant[b];
	}
	block and_gate(const block& a, const block& b) override {
		block table[4];
		block res = halfgates_garble(a, a^delta, b, b^delta, delta, table);
		io->send_block(table, 4);
		return res;
	}
	block xor_gate(const block&a, const block& b) override {
		return a ^ b;
	}
	block not_gate(const block&a) override {
		return xor_gate(a, public_label(true));
	} 
};


template<typename T>
class HalfGateEva:public CircuitExecution {
public:
	T * io;
	block constant[2]; 
	HalfGateEva(T * io) :io(io) {
		set_delta();
		block tmp;
		io->recv_block(&tmp, 1); 
	}
	void set_delta() {
		io->recv_block(constant, 2);
	}
	block public_label(bool b) override {
		return constant[b];
	}
	block and_gate(const block& a, const block& b) override {
		block table[4];
		io->recv_block(table, 4);
		return halfgates_eval(a, b, table);
	}
	block xor_gate(const block& a, const block& b) override {
		return a ^ b;
	}
	block not_gate(const block&a) override {
		return xor_gate(a, public_label(true));
	} 
};


}

#endif// QMPC_CIRCUIT_EXECUTION_H