#include "NetIO.hpp"
//#include "qot.hpp"
#include "fakeot.hpp"
#include<vector>
#include "prg.h"
#include<iostream>
#include "gc.hpp"
#include "bit.hpp"

using namespace std;
using namespace qmpc;

block delta;
CircuitExecution* CircuitExecution::circ_exec;
int party,port;
NetIO *io;
OT *ot;


Bit gen(int value,int owner){
	Bit res;

	if(owner==PUBLIC)
		return CircuitExecution::circ_exec->public_label(value);

	if(owner==ALICE){
		if(party==ALICE){
			block A;
			PRG().random_block(&A, 1);
			res.bit=A;
			if(value)
				A=A^delta;
			io->send_block(&A, 1);
			return res;
		}else{
			io->recv_block(&res.bit, 1);
			return res;
		}
	
	}else{
		if(party==ALICE){
			block A[2];
			PRG().random_block(A, 1);
			A[1]=A[0]^delta;
			ot->send((unsigned char*)&A[0],(unsigned char*)&A[1],sizeof(block));
			res.bit=A[0];
			return res;
		}else{
			ot->recv((unsigned char*)&res.bit, value, sizeof(block));
			return res;
		}
	}
	return res;
}
int reveal(Bit b){ //To Bob

	if(party==ALICE){
		io->send_block(&b.bit, 1);
		return -1;
	}else{
		block label;
		io->recv_block(&label, 1);
		if(label==b.bit)
			return 0;
		else
			return 1;
	}
}



void And_test(){
	//AND Test

	srand(123);
	int T=100;
	while(T--){
		int a=rand()%2,b=rand()%2,c=-1;
		Bit A,B;

		A=gen(a,ALICE);
		B=gen(b,BOB);
	
		Bit C = A & B;

		c=reveal(C);

		if(party==BOB){
			if(c==(a&b))
				puts("AND Test Success!");
			else
				puts("AND Test Failed!");
		}

	}
}

int main(int argc,char **argv){    
    if(argc<3){
        puts("./main <party> <port>");
        return 0;
    }
    sscanf(argv[1],"%d",&party);
    sscanf(argv[2],"%d",&port);
 
 	io=new NetIO(party==ALICE?NULL:"127.0.0.1",port);
	ot=new OT(io,party);

	if(party==ALICE){
        HalfGateGen<NetIO> * t = new HalfGateGen<NetIO>(io);
		delta = t->delta;
        CircuitExecution::circ_exec = t;
	}else{
        HalfGateEva<NetIO> * t = new HalfGateEva<NetIO>(io);
        CircuitExecution::circ_exec = t;
	}

	//And_test();

	int T=10;
	while(T--){
		unsigned int _a=rand(),_b=rand();
		Bit a[32],b[32];
		for(int i=0;i<32;i++){
			a[i]=gen(_a>>i&1,ALICE);
			b[i]=gen(_b>>i&1,BOB);
		}

		Bit res=gen(0,PUBLIC);
		Bit found=gen(0,PUBLIC);

		for(int i=31;i>=0;i--){
			Bit ineq = (a[i]!=b[i]);

			res=res.select((!found) & ineq,!a[i]);
			
			found = found | ineq;

			
		}

		int res1=_a<_b;
		int res2=reveal(res);

		if(party==BOB){
			if(res1==res2)
				puts("yes");
			else{
				puts("no");
				break;
			}

		}
	}

    return 0;
}