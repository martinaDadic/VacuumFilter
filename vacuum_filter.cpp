//Martina Dadić
#include "a5hash.h"
#include <cstdint>
#include <iostream>
using namespace std;

class VacuumFilter {
    public:
        int noOfBuckets; //m
        uint16_t buckets[100][4]; //tablica koja sadrži m buckets od kojih svaka ima 4 polja u kojima se spremaju fingerprints

        uint16_t b1(uint64_t x){ //1. kandidat
            return a5hash( &x, sizeof( x ), 0 ) % noOfBuckets;
        }
        uint16_t b2(uint64_t x){
            return Alt(b1(x), x%16);
        }
        uint16_t Alt(uint16_t b, uint16_t f){
            return b ^ a5hash(&f, sizeof(f), 0);
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