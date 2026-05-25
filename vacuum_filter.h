//Martina Dadić
#include "a5hash.h"
#include <cstdint>
#include <iostream>
#include <cmath>
#include <vector>
#include <array>
#include <string>
using namespace std;

class VacuumFilter {
    public:
        size_t noOfBuckets; //m
        vector<array<uint32_t,4>> buckets; //tablica koja sadrži m buckets od kojih svaka ima 4 polja u kojima se spremaju fingerprints
        size_t L[4] = {}; //polje koje sadrzi duljine AR-ova
        size_t noOfItems; //ukupan broj itema
        const int MAXEVICTS = 500;

        VacuumFilter(size_t m, size_t n);
        bool insert(string x);
        bool lookup(string x);
        bool remove(string x);
        uint32_t Alt(uint32_t b, uint16_t f);
        uint32_t AltVeci(uint32_t b, uint16_t f);
        uint32_t AltManji(uint32_t b, uint16_t f);
        int RangeSelection(int n, float alpha, float r);
        bool LoadFactorTest(int n, float alpha, float r, int L);
        float EstimatedMaxLoad(double N, int c);
        int emptySlot(uint32_t b);
        size_t memory_consumption();

};
