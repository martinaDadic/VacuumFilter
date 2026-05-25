// Martina Dadić
#include "vacuum_filter.h"

#include "a5hash.h"

using namespace std;

VacuumFilter::VacuumFilter(size_t m, size_t n) {  // constructor
  noOfBuckets = 2;
  while (noOfBuckets < m) {
    noOfBuckets <<= 1;
  }
  buckets.resize(noOfBuckets);
  noOfItems = n;
  for (int i = 0; i < 4; i++) {
    L[i] = 0;
  }
}
uint32_t VacuumFilter::Alt(
    uint32_t b,
    uint16_t f) {  // function for deciding which alternative bucket to use
  if (noOfItems <
      262144) {  // 2^18, bad performance when there is a small number of keys
    return AltLessThan(b, f);
  }
  return AltMoreThan(b, f);
}
uint32_t VacuumFilter::AltMoreThan(
    uint32_t b, uint16_t f) {  // finding the alternative bucket
  if (L[0] == 0) {             // if we haven't yet calculated the ARs
    for (int i = 0; i < 4; i++) {
      L[i] = RangeSelection(
          noOfItems, 0.95,
          (1.0 - i / 4.0));  // different ranges depending on the number of
                             // items that should be in each AR
    }
    L[3] *= 2;  // increasing the last AR to avoid fingerprint gathering which
                // can cause failures
  }
  uint32_t l = L[f % 4];  // current AR
  uint16_t delta = a5hash(&f, sizeof(f), 0) %
                   l;  // helper function for calculating the alternative bucket
  return (b ^ delta) & (noOfBuckets - 1);  // protecting from overflow
}
uint32_t VacuumFilter::AltLessThan(  // finding the alternative bucket, for
                                     // number of items < 2^18
    uint32_t b, uint16_t f) {        // finding the alternative bucket
  uint32_t delta = a5hash(&f, sizeof(f), 0) %
                   noOfBuckets;  // helper function, part of the formula
  uint32_t b_2 = (b - delta + noOfBuckets) % noOfBuckets;
  b_2 = (noOfBuckets - 1 - b_2 + delta + noOfBuckets) % noOfBuckets;
  return b_2;
}
int VacuumFilter::RangeSelection(
    int n, float alpha,
    float r) {  // finding the optimal AR length for given parameters
  int L = 1;
  while (LoadFactorTest(n, alpha, r, L) != true && L < noOfBuckets)
    L *= 2;  // increasing until load factor test comes out as true
  return L;
}
bool VacuumFilter::LoadFactorTest(
    int n, float alpha, float r,
    int L) {  // testing the load factor for given parameters
  int m = ceil((double)n / (4.0 * alpha * L) * L);  // number of buckets
  int N = 4.0 * r * m * alpha;                      // number of inserted items
  int c = m / L;                                    // number of chunks
  float P = 0.97 * 4.0 * L;  // the lowest capacity value for each chunk
  float D = EstimatedMaxLoad(N, c);
  if (D < P)
    return true;  // if the estimated maximum load is smaller than the lowest
                  // capacity value, we can say that the load factor is good
                  // enough
  return false;
}
float VacuumFilter::EstimatedMaxLoad(
    double N,
    int c) {  // formula which approximates the maximum load of a chunk
  return (N / c) + 1.5 * sqrt((2.0 * N / c) * log((double)c));
}
bool VacuumFilter::insert(
    string x) {  // function for inserting an item into the filter
  uint32_t hashX = a5hash(x.data(), x.size(), 0);       // hash of the item
  uint32_t hashBucket = a5hash(x.data(), x.size(), 1);  // hash of the bucket
  uint16_t f = (hashX & 0xFFFF);  // fingerprint of the item
  if (f == 0) f = 1;              // 0 is reserved for empty slots
  uint32_t b1 =
      hashBucket & (noOfBuckets - 1);  // the first candidate for the bucket
  uint32_t b2 = Alt(b1, f);            // the alternate bucket candidate
  for (int i = 0; i < 4; i++) {  // if any of the slots are available, we insert
                                 // the fingerprint there
    if (buckets[b1][i] == 0) {
      buckets[b1][i] = f;
      return true;
    } else if (buckets[b2][i] == 0) {
      buckets[b2][i] = f;
      return true;
    }
  }
  uint32_t bNext;
  int randomNumber =
      rand() % 2;  // choosing a random bucket of the two candidates
  if (randomNumber == 0)
    bNext = b1;
  else
    bNext = b2;
  for (int i = 0; i < MAXEVICTS; i++) {
    for (int j = 0; j < 4; j++) {
      uint16_t f1 =
          buckets[bNext]
                 [j];  // fingerprint of the item that is currently in the slot
      uint32_t altBucket = Alt(bNext, f1);  // alternative bucket for the f1
      int empty =
          emptySlot(altBucket);  // returns the index of an empty slot in the
                                 // bucket, if none are empty, returns -1
      if (empty != -1) {  // if there is an empty slot, put the fingerprint
                          // there
        buckets[bNext][j] = f;
        buckets[altBucket][empty] = f1;
        return true;
      }
    }
    int randomNumber =
        rand() %
        4;  // choose a random slot in the bucket, switch the fingerprints
    uint16_t temp = buckets[bNext][randomNumber];
    buckets[bNext][randomNumber] = f;
    f = temp;
    bNext = Alt(bNext, f);  // switch to the next bucket
  }
  return false;  // if we have evicted too many times, we end the search
}
int VacuumFilter::emptySlot(
    uint32_t b) {  // function for finding an empty slot in a bucket
  for (int i = 0; i < 4; i++) {
    if (buckets[b][i] == 0) return i;
  }
  return -1;
}
bool VacuumFilter::lookup(
    string x) {  // function for finding an item in the filter
  uint32_t hashX = a5hash(x.data(), x.size(), 0);  // hash of the item
  uint32_t hashBucket =
      a5hash(x.data(), x.size(), 1);  // hash for selecting the bucket
  uint16_t f = (hashX & 0xFFFF);      // fingerprint of the item
  if (f == 0) f = 1;                  // 0 indicates an empty slot
  uint32_t b1 = hashBucket & (noOfBuckets - 1);  // first candidate bucket
  uint32_t b2 = Alt(b1, f);                      // second candidate bucket
  for (int i = 0; i < 4; i++) {  // if there is the fingerprint in any of the
                                 // two candidates, return true
    if (buckets[b1][i] == f) {
      return true;
    }
    if (buckets[b2][i] == f) {
      return true;
    }
  }
  return false;
}
bool VacuumFilter::remove(string x) {
  uint32_t hashX = a5hash(x.data(), x.size(), 0);  // hash of the item
  uint32_t hashBucket =
      a5hash(x.data(), x.size(), 1);  // hash for selecting the bucket
  uint16_t f = (hashX & 0xFFFF);      // fingerprint of the item
  if (f == 0) f = 1;                  // 0 indicates an empty slot
  uint32_t b1 = hashBucket & (noOfBuckets - 1);  // first candidate bucket
  uint32_t b2 = Alt(b1, f);                      // second candidate bucket
  for (int i = 0; i < 4; i++) {  // first we look up the item and if we find it,
                                 // we delete the value in the slot
    if (buckets[b1][i] == f) {
      buckets[b1][i] = 0;
      return true;
    }
    if (buckets[b2][i] == f) {
      buckets[b2][i] = 0;
      return true;
    }
  }
  return false;
}
size_t VacuumFilter::memory_consumption() {  // returns the memory consumption
                                             // of the filter in bytes
  return noOfBuckets * sizeof(array<uint16_t, 4>);
}
