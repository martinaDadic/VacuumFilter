//Martina Dadić
#include "original_vacuum.h"
#include <chrono>
#include <fstream>
#include <functional>
#include <cassert>

using namespace std;
using namespace std::chrono;

string vrijemeInserta(VacuumFilter<uint16_t, 16> &filter, string fileName);
string vrijemeLookupa(VacuumFilter<uint16_t, 16> &filter, string fileName, int negativeQueries);
string vrijemeRemovea(VacuumFilter<uint16_t, 16> &filter, string fileName);

int main(){
    ofstream results_file;
    results_file.open ("results_original.txt");
    ifstream data_summary_file("data/data_summary.csv");
    string line;
    size_t noOfBuckets;
    getline (data_summary_file, line); //preskacemo zaglavlja
    results_file << "data,sequence length,k,total k-mers,unique k-mers,positive queries,negative queries,number of buckets,insert time,number of failed inserts,lookup time,number of false negatives,false positive rate,remove time,number of failed removes,seed,memory consumption\n";
    while (getline (data_summary_file, line)) {
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
        noOfBuckets = (size_t)(stol(line.substr(0, line.find(';')))/3.8); //3.8 = 4*0.95
        line.erase(0, line.find(';') + 1);
        results_file << line.substr(0, line.find(';')) << ",";
        line.erase(0, line.find(';') + 1);
        int negativeQueries=stoi(line.substr(0, line.find(';')));
        results_file << to_string(negativeQueries) << ",";
        line.erase(0, line.find(';') + 1);
        results_file << noOfBuckets << ",";

        VacuumFilter<uint16_t, 16> filter;
        filter.init(uniqueKmers, 4, 500);

        string insertFile = line.substr(0, line.find(';'));
        line.erase(0, line.find(';') + 1);
        results_file << vrijemeInserta(filter,insertFile) << ",";
        results_file << vrijemeLookupa(filter,line.substr(0, line.find(';')), negativeQueries) << ",";
        line.erase(0, line.find(';') + 1);
        results_file << vrijemeRemovea(filter,insertFile) << ",";
        results_file << "seed: " << line.substr(0, line.find(';')) << ",";
        results_file << "memory consumption: " << filter.memory_consumption;
        results_file << "\n";
    }
    results_file.close();
    data_summary_file.close();
    return 0;
}

string vrijemeInserta(VacuumFilter<uint16_t, 16> &filter, string fileName){
    auto start = high_resolution_clock::now();
    int brojNeuspjelihInserta=0;
    ifstream data(fileName);
    string line;
    while (getline (data, line)) {
        uint64_t item = hash<string>{}(line);
        if(!filter.insert(item)){
            brojNeuspjelihInserta++;
        }
    }
    auto end = high_resolution_clock::now();
    duration<double> elapsed = end - start;
    double elapsed_time = elapsed.count();
    return to_string(elapsed_time) + "," + to_string(brojNeuspjelihInserta);
}

string vrijemeLookupa(VacuumFilter<uint16_t, 16> &filter, string fileName, int negativeQueries){
    auto start = high_resolution_clock::now();
    int brojFalseNegativa=0, brojFalsePozitiva=0;
    ifstream data(fileName);
    string line;
    getline (data, line);//preskacemo zaglavlja
    while (getline (data, line)) {
        string item=line.substr(0, line.find(';'));
        uint64_t item_hash = hash<string>{}(item);
        line.erase(0, line.find(';') + 1);
        bool didFind=filter.lookup(item_hash);
        bool shouldFind=stoi(line) == 1;
        if(!didFind && shouldFind){
            brojFalseNegativa++;
        }
        else if(didFind && !shouldFind){
            brojFalsePozitiva++;
        }
    }
    auto end = high_resolution_clock::now();
    duration<double> elapsed = end - start;
    double elapsed_time = elapsed.count();
    float falsePositiveRate = (float)brojFalsePozitiva / negativeQueries;
    return to_string(elapsed_time) + "," + to_string(brojFalseNegativa) + "," + to_string(falsePositiveRate);
}
string vrijemeRemovea(VacuumFilter<uint16_t, 16> &filter, string fileName){
    auto start = high_resolution_clock::now();
    int brojNeuspjelihRemovea=0;
    ifstream data(fileName);
    string line;
    while (getline (data, line)) {
        uint64_t item = hash<string>{}(line);
        if(!filter.del(item)){
            brojNeuspjelihRemovea++;
        }
    }
    auto end = high_resolution_clock::now();
    duration<double> elapsed = end - start;
    double elapsed_time = elapsed.count();
    return to_string(elapsed_time) + "," + to_string(brojNeuspjelihRemovea);
}