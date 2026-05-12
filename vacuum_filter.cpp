//Martina Dadić
#include "a5hash.h"
#include <cstdint>
#include <iostream>
#include <cmath>
using namespace std;

class VacuumFilter {
    public:
        int noOfBuckets; //m
        uint16_t buckets[1000][4] = {}; //tablica koja sadrži m buckets od kojih svaka ima 4 polja u kojima se spremaju fingerprints
        int L[4] = {}; //polje koje sadrzi duljine AR-ova
        int n; //ukupan broj itema

        VacuumFilter(int m){ //konstruktor
            noOfBuckets = m;
            n=0;
        }
        uint16_t Alt(uint16_t b, uint16_t f){
            if (n < 262144){ //2^18
                return AltManji(b, f);
            }
            return AltVeci(b, f);
        }
        uint16_t AltVeci(uint16_t b, uint16_t f){
            if (L[0]==0){
                for (int i=1;i<5;i++){
                    L[i-1]=RangeSelection(n, 0.95, (1.0 - i / 4.0));
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
            int L=1;
            while (LoadFactorTest(n, alpha, r, L) != true)
                L*=2;
            return L;
        }
        bool LoadFactorTest(int n, float alpha, int r, int L){
            int m = ceil((double)n / (4.0 * alpha * L) * L); //broj bucket-a
            int N = 4*r*m*alpha; //broj unesenih elemenata
            int c=m/L; //broj chunk-ova
            float P = 0.97 * 4 * L;
            float D = EstimatedMaxLoad(N, c);
            if (D<P)
                return true;
            return false;
        }
        float EstimatedMaxLoad(int N, int c){
            return N/c + 1.5*sqrt((2*N/c)*log(c));
        }
        bool insert(int x){
            uint16_t hashX = a5hash(&x, sizeof(x), 0); //hash itema
            uint16_t f=hashX & 0xFFFF; //fingerprint item-a
            uint16_t b1 = hashX % noOfBuckets; //1. kandidat
            uint16_t b2 = Alt(b1, f); //2. kandidat
            for(int i=0;i<4;i++){
                if (buckets[b1][i]==0){
                    buckets[b1][i]=f;
                    n++;
                    return true;
                }
                if (buckets[b2][i]==0){
                    buckets[b2][i]=f;
                    n++;
                    return true;
                }
            }
            int randomNumber = rand() % 8; //random broj od 0 do 7
            return false;
        }
        bool lookup(int x){
            uint16_t hashX = a5hash(&x, sizeof(x), 0); //hash itema
            uint16_t f=hashX & 0xFFFF; //fingerprint item-a
            uint16_t b1 = hashX % noOfBuckets; //1. kandidat
            uint16_t b2 = Alt(b1, f); //2. kandidat
            for(int i=0;i<4;i++){
                if (buckets[b1][i]==f){
                    return true;
                }
                if (buckets[b2][i]==f){
                    return true;
                }
            }
            return false;
        }
        bool remove(int x){
            uint16_t hashX = a5hash(&x, sizeof(x), 0); //hash itema
            uint16_t f=hashX & 0xFFFF; //fingerprint item-a
            uint16_t b1 = hashX % noOfBuckets; //1. kandidat
            uint16_t b2 = Alt(b1, f); //2. kandidat
            for(int i=0;i<4;i++){
                if (buckets[b1][i]==f){
                    buckets[b1][i]=0;
                    return true;
                }
                if (buckets[b2][i]==f){
                    buckets[b2][i]=0;
                    return true;
                }
            }
            return false;
        }
        
};

int main(){
    VacuumFilter filter(5000);
    cout << filter.noOfBuckets;
    cout << "Load Factor Test za n=1000, alpha=0.95, r=10 i L=20: " << filter.LoadFactorTest(1000, 0.95, 10, 20) <<"\n";
    cout << "Range selection za n=1000, alpha=0.95, r=10: " << filter.RangeSelection(1000, 0.95, 10) <<"\n";
}