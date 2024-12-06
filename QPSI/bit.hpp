#ifndef BITS_HPP
#define BITS_HPP

#include "block.h"
#include "gc.hpp"

namespace qmpc{

class Bit{ public:
	block bit;

	//Bit(bool _b = false, int party = PUBLIC);
	Bit() {}
	Bit(const block& a) {
		memcpy(&bit, &a, sizeof(block));
	}

	template<typename O = bool>
	O reveal(int party = PUBLIC) const;

	Bit operator!=(const Bit& rhs) const;
	Bit operator==(const Bit& rhs) const;
	Bit operator &(const Bit& rhs) const;
	Bit operator |(const Bit& rhs) const;
	Bit operator !() const;

	//swappable
	Bit select(const Bit & select, const Bit & new_v)const ;
	Bit operator ^(const Bit& rhs) const;
	Bit operator ^=(const Bit& rhs);

	//batcher
	template<typename... Args>
	static size_t bool_size(Args&&... args) {
		return 1;
	}

	static void bool_data(bool *b, bool data) {
		b[0] = data;
	}
};

/*inline Bit::Bit(bool b, int party) {
	if (party == PUBLIC)
		bit = CircuitExecution::circ_exec->public_label(b);
	else {
        ProtocolExecution::prot_exec->feed(&bit, party, &b, 1); 
    }
}*/

inline Bit Bit::select(const Bit & select, const Bit & new_v) const{
	Bit tmp = *this;
	tmp = tmp ^ new_v;
	tmp = tmp & select;
	return *this ^ tmp;
}

/*template<typename O>
inline O Bit::reveal(int party) const {
	O res;
	ProtocolExecution::prot_exec->reveal(&res, party, &bit, 1);
	return res;
}*/ 

inline Bit Bit::operator==(const Bit& rhs) const {
	return !(*this ^ rhs);
}

inline Bit Bit::operator!=(const Bit& rhs) const {
	return (*this) ^ rhs;
}

inline Bit Bit::operator &(const Bit& rhs) const{
	Bit res;
	res.bit = CircuitExecution::circ_exec->and_gate(bit, rhs.bit);
	return res;
}
inline Bit Bit::operator ^(const Bit& rhs) const{
	Bit res;
	res.bit = CircuitExecution::circ_exec->xor_gate(bit, rhs.bit);
	return res;
}
inline Bit Bit::operator ^=(const Bit& rhs) {
	this->bit = CircuitExecution::circ_exec->xor_gate(bit, rhs.bit);
	return (*this);
}

inline Bit Bit::operator |(const Bit& rhs) const{
	return (*this ^ rhs) ^ (*this & rhs);
}

inline Bit Bit::operator!() const {
	return CircuitExecution::circ_exec->not_gate(bit);
}


}
#endif
