// Martina Dadić
#include <array>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

#include "a5hash.h"
using namespace std;

class VacuumFilter {  // class for the vacuum filter, contains the 3 functions
                      // for inserting, looking up and removing items
 public:
  size_t noOfBuckets;                  // m
  vector<array<uint32_t, 4>> buckets;  // a vector which contains buckets, each
                                       // bucket can contain 4 fingerprints
  size_t L[4] = {};  // array which contains the lenghts of the ARs
  size_t noOfItems;  // total number of items which we intend to insert into the
                     // filter
  const int MAXEVICTS =
      500;  // maximum number of evictions we will do when inserting an item

  VacuumFilter(size_t m, size_t n);
  bool insert(string x);
  bool lookup(string x);
  bool remove(string x);
  uint32_t Alt(uint32_t b, uint16_t f);
  uint32_t AltMoreThan(uint32_t b, uint16_t f);
  uint32_t AltLessThan(uint32_t b, uint16_t f);
  int RangeSelection(int n, float alpha, float r);
  bool LoadFactorTest(int n, float alpha, float r, int L);
  float EstimatedMaxLoad(double N, int c);
  int emptySlot(uint32_t b);
  size_t memory_consumption();
};
