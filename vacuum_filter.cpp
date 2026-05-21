//Martina Dadić
#include "a5hash.h"
#include "vacuum_filter.h"

using namespace std;



VacuumFilter::VacuumFilter(size_t m){ //konstruktor
    noOfBuckets = m;
    buckets.resize(noOfBuckets);
    n=0;
}
uint32_t VacuumFilter::Alt(uint32_t b, uint16_t f){
    if (n < 262144){ //2^18
        return AltManji(b, f);
    }
    return AltVeci(b, f);
}
uint32_t VacuumFilter::AltVeci(uint32_t b, uint16_t f){
    if (L[0]==0){ //ako nismo vec izracunal AR-ove
        for (int i=1;i<5;i++){
            L[i-1]=RangeSelection(n, 0.95, (1.0 - i / 4.0));
        }
        L[3]*=2; //povecamo zadnji da izbjegnemo fail
    }
    int l = L[f % 4]; //trenutni AR
    uint16_t delta = a5hash(&f, sizeof(f), 0) % l;
    return b ^ delta;
}
uint32_t VacuumFilter::AltManji(uint32_t b, uint16_t f){
    uint16_t delta = a5hash(&f, sizeof(f), 0) % noOfBuckets;
    uint16_t b_2=(b-delta) % noOfBuckets;
    b_2=(noOfBuckets - 1 - b_2 + delta) % noOfBuckets;
    return b_2;
}
int VacuumFilter::RangeSelection(int n, float alpha, float r){
    int L=1;
    while (LoadFactorTest(n, alpha, r, L) != true)
        L*=2;
    return L;
}
bool VacuumFilter::LoadFactorTest(int n, float alpha, float r, int L){
    int m = ceil((double)n / (4.0 * alpha * L) * L); //broj bucket-a
    int N = 4.0*r*m*alpha; //broj unesenih elemenata
    int c=m/L; //broj chunk-ova
    float P = 0.97 * 4.0 * L; //najniza vrijednost kapaciteta svakog chunk-a
    float D = EstimatedMaxLoad(N, c);
    if (D<P)
        return true;
    return false;
}
float VacuumFilter::EstimatedMaxLoad(double N, int c){
    return (N / c) + 1.5 * sqrt((2.0 * N / c) * log((double)c));
}
bool VacuumFilter::insert(string x){
    uint32_t hashX = a5hash(x.data(), x.size(), 0); //hash itema
    uint16_t f=hashX & 0xFFFF; //fingerprint item-a
    uint32_t b1 = hashX % noOfBuckets; //1. kandidat
    uint32_t b2 = Alt(b1, f); //2. kandidat
    for(int i=0;i<4;i++){
        if (buckets[b1][i]==0){
            buckets[b1][i]=f;
            n++;
            return true;
        }
        else if (buckets[b2][i]==0){
            buckets[b2][i]=f;
            n++;
            return true;
        }
    }
    uint32_t bIduci;
    int randomNumber = rand() % 2; //random biramo izmedu b1 i b2
    if (randomNumber == 0)
        bIduci=b1;
    else
        bIduci=b2;
    for(int i=0;i<MAXEVICTS;i++){
        for(int j=0;j<4;j++){
            uint16_t f1=buckets[bIduci][j];
            int empty=emptySlot(Alt(bIduci, f1));//vraca mjesto slobodnog slota, ako ga ima, ako ne, vraca 0
            if(empty!=-1){
                buckets[bIduci][j]=f;
                buckets[Alt(bIduci, f1)][empty]=f1;
                n++;
                return true;
            }
        }
        int randomNumber = rand() % 4; //random biramo slot iz bIduci
        uint16_t temp = buckets[bIduci][randomNumber];
        buckets[bIduci][randomNumber]=f;
        f=temp;
        bIduci=Alt(bIduci, f);
    }
    return false;
}
int VacuumFilter::emptySlot(uint32_t b){
    for(int i=0;i<4;i++){
        if (buckets[b][i]==0)
            return i;
    }
    return -1;
}
bool VacuumFilter::lookup(string x){
    uint32_t hashX = a5hash(x.data(), x.size(), 0); //hash itema
    uint16_t f=hashX & 0xFFFF; //fingerprint item-a
    uint32_t b1 = hashX % noOfBuckets; //1. kandidat
    uint32_t b2 = Alt(b1, f); //2. kandidat
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
bool VacuumFilter::remove(string x){
    uint32_t hashX = a5hash(x.data(), x.size(), 0); //hash itema
    uint16_t f=hashX & 0xFFFF; //fingerprint item-a
    uint32_t b1 = hashX % noOfBuckets; //1. kandidat
    uint32_t b2 = Alt(b1, f); //2. kandidat
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
