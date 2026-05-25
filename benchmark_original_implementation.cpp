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
    while (getline (data_summary_file, line)) {
        results_file << line.substr(0, line.find(';')) << "\n";
        line.erase(0, line.find(';') + 1);
        results_file << "sequence length: " << line.substr(0, line.find(';')) << "\n";
        line.erase(0, line.find(';') + 1);
        results_file << "k: " << line.substr(0, line.find(';')) << "\n";
        line.erase(0, line.find(';') + 1);
        results_file << "total k-mers: " << line.substr(0, line.find(';')) << "\n";
        line.erase(0, line.find(';') + 1);
        size_t uniqueKmers = stol(line.substr(0, line.find(';')));
        results_file << "unique k-mers: " << uniqueKmers << "\n";
        noOfBuckets =  (size_t)(stol(line.substr(0, line.find(';')))/3.8); //3.8 = 4*0.95
        line.erase(0, line.find(';') + 1);
        results_file << "positive queries: " << line.substr(0, line.find(';')) << "\n";
        line.erase(0, line.find(';') + 1);
        int negativeQueries=stoi(line.substr(0, line.find(';')));
        results_file << "negative queries: " << to_string(negativeQueries) << "\n";
        line.erase(0, line.find(';') + 1);
        results_file << "number of buckets: " << noOfBuckets << "\n";

        VacuumFilter<uint16_t, 16> filter;
        filter.init(uniqueKmers, 4, 500);

        string insertFile = line.substr(0, line.find(';'));
        line.erase(0, line.find(';') + 1);
        results_file << vrijemeInserta(filter,insertFile) << "\n";
        results_file << vrijemeLookupa(filter,line.substr(0, line.find(';')), negativeQueries) << "\n";
        line.erase(0, line.find(';') + 1);
        results_file << vrijemeRemovea(filter,insertFile) << "\n";
        results_file << "seed: " << line.substr(0, line.find(';')) << "\n";
        results_file << "memory consumption: " << filter.memory_consumption << "\n";
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
    return "insert time: " + to_string(elapsed_time) + "\n" + "number of failed inserts: " + to_string(brojNeuspjelihInserta) + "\n";
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
    return "lookup time: " + to_string(elapsed_time) 
        + "\n" + "number of false negatives: " + to_string(brojFalseNegativa) + "\n" 
        + "false positive rate: " + to_string(falsePositiveRate) + "\n";
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
    return "remove time: " + to_string(elapsed_time) + "\n" + "number of failed removes: " + to_string(brojNeuspjelihRemovea) + "\n";
}