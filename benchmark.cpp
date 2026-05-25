// Martina Dadić
#include <chrono>
#include <fstream>

#include "vacuum_filter.h"

using namespace std;
using namespace std::chrono;

string insert_time(VacuumFilter& filter, string fileName);
string lookup_time(VacuumFilter& filter, string fileName, int negativeQueries);
string remove_time(VacuumFilter& filter, string fileName);

int main() {
  ofstream results_file;
  results_file.open("results.txt");  // file in which we output the results
  ifstream data_summary_file(
      "data/data_summary.csv");  // file which contains the summary of the data
                                 // we are testing on
  string line;
  size_t noOfBuckets;
  getline(data_summary_file, line);  // skipping the header
  results_file << "data,sequence length,k,total k-mers,unique k-mers,positive "
                  "queries,negative queries,number of buckets,insert "
                  "time,number of "
                  "failed inserts,lookup time,number of false negatives,false "
                  "positive "
                  "rate,remove time,number of failed removes,seed,memory "
                  "consumption\n";  // we output the data in csv format
  while (getline(data_summary_file,
                 line)) {  // for each line we output the information contained
    results_file << line.substr(0, line.find(';')) << ",";
    line.erase(0, line.find(';') + 1);
    results_file << line.substr(0, line.find(';')) << ",";
    line.erase(0, line.find(';') + 1);
    results_file << line.substr(0, line.find(';')) << ",";
    line.erase(0, line.find(';') + 1);
    results_file << line.substr(0, line.find(';')) << ",";
    line.erase(0, line.find(';') + 1);
    size_t uniqueKmers = stol(line.substr(0, line.find(';')));
    results_file << uniqueKmers << ",";
    noOfBuckets =
        (size_t)(stol(line.substr(0, line.find(';'))) /
                 3.8);  // 3.8 = 4*0.95, 0.95 is the target load factor, 4 for
                        // the number of slots in a bucket
    line.erase(0, line.find(';') + 1);
    results_file << line.substr(0, line.find(';')) << ",";
    line.erase(0, line.find(';') + 1);
    int negativeQueries = stoi(line.substr(0, line.find(';')));
    results_file << to_string(negativeQueries) << ",";
    line.erase(0, line.find(';') + 1);
    results_file << noOfBuckets << ",";

    VacuumFilter filter(noOfBuckets, uniqueKmers);
    string insertFile = line.substr(0, line.find(';'));
    line.erase(0, line.find(';') + 1);
    results_file << insert_time(filter, insertFile) << ",";
    results_file << lookup_time(filter, line.substr(0, line.find(';')),
                                negativeQueries)
                 << ",";
    line.erase(0, line.find(';') + 1);
    results_file << remove_time(filter, insertFile) << ",";
    results_file << line.substr(0, line.find(';')) << ",";
    results_file << filter.memory_consumption();
    results_file << "\n";
  }
  results_file.close();
  data_summary_file.close();
  return 0;
}

string insert_time(
    VacuumFilter& filter,
    string fileName) {  // function which measures and outputs the time to input
                        // all the items in the filter
  auto start = high_resolution_clock::now();
  int numberOfFailedInserts = 0;
  ifstream data(fileName);
  string line;
  while (getline(data, line)) {
    if (!filter.insert(line)) {
      numberOfFailedInserts++;
    }
  }
  auto end = high_resolution_clock::now();
  duration<double> elapsed = end - start;
  double elapsed_time = elapsed.count();
  return to_string(elapsed_time) + "," + to_string(numberOfFailedInserts);
}

string lookup_time(VacuumFilter& filter, string fileName,
                   int negativeQueries) {  // function which measures and
                                           // outputs the time to lookup
                                           // all the query items
  auto start = high_resolution_clock::now();
  int falseNegatives = 0, falsePositives = 0;
  ifstream data(fileName);
  string line;
  getline(data, line);  // preskacemo zaglavlja
  while (getline(data, line)) {
    string item = line.substr(0, line.find(';'));
    line.erase(0, line.find(';') + 1);
    bool didFind = filter.lookup(item);
    bool shouldFind = stoi(line) == 1;
    if (!didFind && shouldFind) {
      falseNegatives++;
    } else if (didFind && !shouldFind) {
      falsePositives++;
    }
  }
  auto end = high_resolution_clock::now();
  duration<double> elapsed = end - start;
  double elapsed_time = elapsed.count();
  float falsePositiveRate = (float)falsePositives / negativeQueries;
  return to_string(elapsed_time) + "," + to_string(falseNegatives) + "," +
         to_string(falsePositiveRate);
}
string remove_time(
    VacuumFilter& filter,
    string fileName) {  // function which measures and outputs the time to
                        // remove all the items in the filter
  auto start = high_resolution_clock::now();
  int numberOfFailedRemoves = 0;
  ifstream data(fileName);
  string line;
  while (getline(data, line)) {
    if (!filter.remove(line)) {
      numberOfFailedRemoves++;
    }
  }
  auto end = high_resolution_clock::now();
  duration<double> elapsed = end - start;
  double elapsed_time = elapsed.count();
  return to_string(elapsed_time) + "," + to_string(numberOfFailedRemoves);
}

void testVacuum() {  // testing if all my functionalities work correctly,
                     // I used this function for testing during
                     // development
  VacuumFilter filter(10000000, 10000000);
  cout << "Load Factor Test for n=10000000, alpha=0.95, r=0.75 and L=20: "
       << filter.LoadFactorTest(1000, 0.95, 0.75, 20) << "\n";
  cout << "Range selection for n=10000000, alpha=0.95, r=0.75: "
       << filter.RangeSelection(1000, 0.95, 0.75) << "\n";
  int numberOfFailedInserts = 0;
  int FalseNegatives = 0;
  int numberOfFailedRemoves = 0;
  int FalsePositives = 0;
  auto start = high_resolution_clock::now();
  for (int i = 0; i < 10000000; i++) {
    if (!filter.insert(to_string(i))) {
      numberOfFailedInserts++;
    }
  }
  auto end = high_resolution_clock::now();
  duration<double> elapsed = end - start;
  double elapsed_time = elapsed.count();
  cout << "number of failed inserts: " << numberOfFailedInserts << "\n";
  cout << "elapsed time: " << elapsed_time << "\n";
  for (int i = 0; i < 10000000; i++) {
    if (!filter.lookup(to_string(i))) {
      FalseNegatives++;
    }
  }
  cout << "number of false negatives: " << FalseNegatives << "\n";
  for (int i = 0; i < 10000000; i++) {
    if (!filter.remove(to_string(i))) {
      numberOfFailedRemoves++;
    }
  }
  cout << "number of failed removes: " << numberOfFailedRemoves << "\n";
  for (int i = 10000000; i < 11000000; i++) {
    if (filter.lookup(to_string(i))) {
      FalsePositives++;
    }
  }
  cout << "number of false positives: " << FalsePositives << "\n";
  FalseNegatives = 0;
  for (int i = 0; i < 10000000; i++) {
    if (!filter.lookup(to_string(i))) {
      FalseNegatives++;
    }
  }
  cout << "false negatives after deleting all the items: " << FalseNegatives
       << "\n";
}