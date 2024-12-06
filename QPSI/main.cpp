#include "NetIO.hpp"
//#include "qot.hpp"
#include "fakeot.hpp"
#include<vector>
#include "prg.h"
#include<iostream>

using namespace std;

namespace qmpc{


template<class T>
class BoomFilter{
public:

    int m,k;
    bool *data;

    BoomFilter(int m,int k){
        this->m=m;
        this->k=k;
        data=new bool[m];
        memset(data,0,m*sizeof(bool));
    } 

    vector<int> subset(T x){
        vector<int>res;
        block b=Hash::hash_for_block(&x,sizeof(T));
        PRG prg(&b);
        for(int i=0;i<k;i++){
            unsigned int y;
            prg.random_data(&y,sizeof(y));
            res.push_back(y%m);
        }
        return res;
    }

    void insert(T x){
        auto S=subset(x); 
        for(auto y:S)
            data[y]=1;
    }

    bool count(T x){
        bool ans=true;
        auto S=subset(x);
        for(auto y:S)
            ans&=data[y];
        return ans;
    }

};

class PSI{
public:
    

    block key[num_ot][2];
    bool b[num_ot];
    bool open[num_ot];
    int perm[num_bf];

    NetIO *io;
    OT *ot;
    int party;
    PRG prg;
    PSI(int party,NetIO *io){
        this->party=party;
        this->io=io;
        this->ot=new OT(io,party);
    }

    template<class T>
    void shuffle(T *a,int n){
        for(int i=1;i<n;i++){
            unsigned int x;
            prg.random_data(&x,1);
            x=x%(i+1);
            swap(a[i],a[x]);
        }
    }
    vector<int> plain_intersect(vector<int>vec_alice,vector<int>vec_bob){
        int n=vec_alice.size();
        int m=vec_bob.size();
        vector<int>result;
        BoomFilter<int> bf(num_bf,10);
        if(party==ALICE){
            for(int i=0;i<n;i++){
                int x=vec_alice[i];
                io->send_data(&x,sizeof(x));
            }
        }else{
            for(int i=0;i<n;i++){
                int x;
                io->recv_data(&x,sizeof(x));
                vec_alice[i]=x;
            }
        }
        for(int i=0;i<m;i++)
            bf.insert(vec_bob[i]);
        for(int i=0;i<n;i++)
            if(bf.count(vec_alice[i]))
                result.push_back(vec_alice[i]);
        //std::set_intersection(std::begin(vec_alice), std::end(vec_alice), std::begin(vec_bob), std::end(vec_bob),std::inserter(result, std::begin(result)));
        return result;
    }

    vector<int> intersect(vector<int>vec_alice,vector<int>vec_bob){ 
        vector<int>res;
        if(party==ALICE){
            for(int i=0;i<num_ot;i++){
                prg.random_block(&key[i][0],2);
                ot->send((unsigned char*)&key[i][0],(unsigned char*)&key[i][1],sizeof(block));
            }
        }else{ 
            for(int i=0;i<num_ot;i++)
                b[i]=i<(alpha*num_ot);
            shuffle(b,num_ot);
            for(int i=0;i<num_ot;i++)
                ot->recv((unsigned char*)&key[i][b[i]],b[i],sizeof(block));
        }

        if(party==ALICE){
            for(int i=0;i<num_ot;i++)
                open[i]=i<num_open;
            shuffle(open,num_ot);
            io->send_data(open,num_ot);
        }else{
            io->recv_data(open,num_ot);
        }

        if(party==ALICE){
            block tmp;
            int cnt=0,all=0;
            for(int i=0;i<num_ot;i++)if(open[i]){
                io->recv_block(&tmp,1);
                all++;
                if(tmp==key[i][1])
                    cnt++;
            } 
            //TODO
            //if(cnt>alpha*all){
            //    puts("too many one");
            //}
        }else{
            for(int i=0;i<num_ot;i++)if(open[i])
                io->send_block(&key[i][b[i]],1);
        }

        BoomFilter<int> bf(num_bf,10);
        if(party==ALICE){ 
            io->recv_data(perm,sizeof(perm));

        }else{
            vector<int>one,zero;
            for(int i=0;i<num_ot;i++)if(!open[i]){
                if(b[i]){
                    one.push_back(i);
                }else{
                    zero.push_back(i);
                }
            }
            random_shuffle(one.begin(),one.end());
            random_shuffle(zero.begin(),zero.end());
            int cc=0;
            for(auto x:vec_bob)
                bf.insert(x);
            for(int i=0;i<num_bf;i++){
                if(bf.data[i]){
                    if(one.empty()){puts("no enough one");}
                    perm[cc++]=one.back();
                    one.pop_back();
                }else{
                    if(zero.empty()){puts("no enough zero");}
                    perm[cc++]=zero.back();
                    zero.pop_back();
                }
            } 

            io->send_data(perm,sizeof(perm));
        }

        if(party==ALICE){ 
            for(auto x:vec_alice){
                auto S=bf.subset(x);
                block K;
                Hash h;
                h.put(&x,sizeof(x));
                for(auto y:S){
                    h.put(&key[perm[y]][1],sizeof(block));
                }
                h.digest((char*)&K); 
                io->send_block(&K,1);
            }
        }else{
            vector<block>KA;
            block K;
            for(int j=0;j<(int)vec_alice.size();j++){
                io->recv_block(&K,1);
                KA.push_back(K);
            }


            for(auto x:vec_bob){
                auto S=bf.subset(x);
                Hash h;
                h.put(&x,sizeof(x));
                for(auto y:S){
                    h.put(&key[perm[y]][1],sizeof(block));
                }
                h.digest((char*)&K);
                for(auto y:KA)if(y==K){
                    res.push_back(x);
                    break;
                }
            }

        }
        return res;
    }
};


}


using namespace qmpc;


int main(int argc,char **argv){    
    int party,port;
    if(argc<3){
        puts("./main <party> <port>");
        return 0;
    }
    sscanf(argv[1],"%d",&party);
    sscanf(argv[2],"%d",&port);
 
    NetIO *io=new NetIO(party==ALICE?NULL:"127.0.0.1",port);
 
    PSI *psi=new PSI(party,io); 
    int n=50,m=50;
    vector<int>vec_alice,vec_bob;
    vec_alice.resize(n);
    vec_bob.resize(m);

    alpha=1.0*n*10/num_bf*2;

 
int T=100;
while(T--){


    if(party==ALICE){
        vec_alice[0]=1;
        for(int i=1;i<n;i++){
            vec_alice[i]=vec_alice[i-1]+(rand()%3+1);
        }
    }else{

        vec_bob[0]=1;
        for(int i=1;i<m;i++){
            vec_bob[i]=vec_bob[i-1]+(rand()%3+1);
        }
    }

    /*if(party==ALICE){
            cout<<"Alice "<<party<<endl;
            for(auto x:vec_alice)cout<<x<<" ";cout<<endl;
    }else{

            cout<<"Bob "<<party<<endl;
            for(auto y:vec_bob)cout<<y<<" ";cout<<endl;
    }*/


    auto vec1=psi->plain_intersect(vec_alice,vec_bob);
    auto vec2=psi->intersect(vec_alice,vec_bob);

    sort(vec1.begin(),vec1.end());
    sort(vec2.begin(),vec2.end());
    if(party==BOB){

        if(vec1==vec2){ 
        }
        else{ 

            cout<<"vec1"<<endl;
            for(auto x:vec1)cout<<x<<" ";cout<<endl;
            cout<<"vec2"<<endl;
            for(auto y:vec2)cout<<y<<" ";cout<<endl;
            puts("NNNO");
            return 0;
        }
    }
}
puts("yes");
    return 0;
}