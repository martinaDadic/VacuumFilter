//Martina Dadić
#include "a5hash.h"
#include <cstdint>
#include <iostream>
#include <cmath>
using namespace std;

class VacuumFilter {
    public:
        int noOfBuckets; //m
        uint16_t buckets[100][4]; //tablica koja sadrži m buckets od kojih svaka ima 4 polja u kojima se spremaju fingerprints
        int L[4]; //polje koje sadrzi duljine AR-ova
        int n; //ukupan broj itema

        uint16_t b1(uint64_t x){ //1. kandidat
            return a5hash( &x, sizeof( x ), 0 ) % noOfBuckets;
        }
        uint16_t b2(uint64_t x){
            return Alt(b1(x), x%16);
        }
        uint16_t Alt(uint16_t b, uint16_t f){
            if (n < pow(2, 18)){
                return AltManji(b, f);
            }
            return AltVeci(b, f);
        }
        uint16_t AltVeci(uint16_t b, uint16_t f){
            if (L==NULL){
                for (int i=0;i<4;i++){
                    L[i]=RangeSelection(n, 0.95, (1 - i/4));
                }
                L[3]*=2; //povecamo zadnji da izbjegnemo fail
            }
            int l = L[f % 4]; //trenutni AR
            uint16_t delta = a5hash(&f, sizeof(f), 0) % l;
            return b ^ delta;
        }
        uint16_t AltManji(uint16_t b, uint16_t f){
            uint16_t delta = a5hash(&f, sizeof(f), 0) % noOfBuckets;
            uint16_t b_2=(b-delta) % noOfBuckets;
            b_2=(noOfBuckets - 1 - b_2 + delta) % noOfBuckets;
            return b_2;
        }
        int RangeSelection(int n, float alpha, int r){
            return 0;
        }
        bool lookup(){
            return false;
        }
        bool insert(){
            return false;
        }
        bool remove(){
            return false;
        }
        
};

int main(){
    VacuumFilter filter;
    filter.noOfBuckets = 5;
    cout << filter.noOfBuckets;
}