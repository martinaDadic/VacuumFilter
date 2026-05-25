//Martina Dadić
#include "vacuum_filter.h"
#include <chrono>
#include <fstream>

using namespace std;
using namespace std::chrono;

string vrijemeInserta(VacuumFilter &filter, string fileName);
string vrijemeLookupa(VacuumFilter &filter, string fileName, int negativeQueries);
string vrijemeRemovea(VacuumFilter &filter, string fileName);

int main(){
    ofstream results_file;
    results_file.open ("results.txt");
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

        VacuumFilter filter(noOfBuckets, uniqueKmers);
        string insertFile = line.substr(0, line.find(';'));
        line.erase(0, line.find(';') + 1);
        results_file << vrijemeInserta(filter,insertFile) << "\n";
        results_file << vrijemeLookupa(filter,line.substr(0, line.find(';')), negativeQueries) << "\n";
        line.erase(0, line.find(';') + 1);
        results_file << vrijemeRemovea(filter,insertFile) << "\n";
        results_file << "seed: " << line.substr(0, line.find(';')) << "\n";
        results_file << "memory consumption: " << filter.memory_consumption() << "\n";
    }
    results_file.close();
    data_summary_file.close();
    return 0;
}

string vrijemeInserta(VacuumFilter &filter, string fileName){
    auto start = high_resolution_clock::now();
    int brojNeuspjelihInserta=0;
    ifstream data(fileName);
    string line;
    while (getline (data, line)) {
        if(!filter.insert(line)){
            brojNeuspjelihInserta++;
        }
    }
    auto end = high_resolution_clock::now();
    duration<double> elapsed = end - start;
    double elapsed_time = elapsed.count();
    return "insert time: " + to_string(elapsed_time) + "\n" + "number of failed inserts: " + to_string(brojNeuspjelihInserta) + "\n";
}

string vrijemeLookupa(VacuumFilter &filter, string fileName, int negativeQueries){
    auto start = high_resolution_clock::now();
    int brojFalseNegativa=0, brojFalsePozitiva=0;
    ifstream data(fileName);
    string line;
    getline (data, line);//preskacemo zaglavlja
    while (getline (data, line)) {
        string item=line.substr(0, line.find(';'));
        line.erase(0, line.find(';') + 1);
        bool didFind=filter.lookup(item);
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
string vrijemeRemovea(VacuumFilter &filter, string fileName){
    auto start = high_resolution_clock::now();
    int brojNeuspjelihRemovea=0;
    ifstream data(fileName);
    string line;
    while (getline (data, line)) {
        if(!filter.remove(line)){
            brojNeuspjelihRemovea++;
        }
    }
    auto end = high_resolution_clock::now();
    duration<double> elapsed = end - start;
    double elapsed_time = elapsed.count();
    return "remove time: " + to_string(elapsed_time) + "\n" + "number of failed removes: " + to_string(brojNeuspjelihRemovea) + "\n";
}

void testiranjeVacuuma(){
    //ovdje testiram radi li moj vacuum filter ispravno
    VacuumFilter filter(10000000, 10000000);
    cout << "Load Factor Test za n=10000000, alpha=0.95, r=0.75 i L=20: " << filter.LoadFactorTest(1000, 0.95, 0.75, 20) <<"\n";
    cout << "Range selection za n=10000000, alpha=0.95, r=0.75: " << filter.RangeSelection(1000, 0.95, 0.75) <<"\n";
    int brojNeuspjelihInserta=0;
    int brojFalseNegativa=0;
    int brojNeuspjelihRemovea=0;
    int brojFalsePositivea=0;
    auto start = high_resolution_clock::now();
    for (int i=0;i<10000000;i++){
        if(!filter.insert(to_string(i))){
            brojNeuspjelihInserta++;
        }
    }
    auto end = high_resolution_clock::now();
    duration<double> elapsed = end - start;
    double elapsed_time = elapsed.count();
    cout << "broj neuspjeha inserta: " << brojNeuspjelihInserta << "\n";
    cout << "vrijeme izvrsavanja inserta: " << elapsed_time << "\n";
    for (int i=0;i<10000000;i++){
        if(!filter.lookup(to_string(i))){
            brojFalseNegativa++;
        }
    }
    cout << "broj false negativa: " << brojFalseNegativa << "\n";
    for (int i=0;i<10000000;i++){
        if(!filter.remove(to_string(i))){
            brojNeuspjelihRemovea++;
        }
    }
    cout << "broj neuspjeha removea: " << brojNeuspjelihRemovea << "\n";
    for (int i=10000000;i<11000000;i++){
        if(filter.lookup(to_string(i))){
            brojFalsePositivea++;
        }
    }
    cout << "broj false pozitiva: " << brojFalsePositivea << "\n";
    brojFalseNegativa=0;
    for (int i=0;i<10000000;i++){
        if(!filter.lookup(to_string(i))){
            brojFalseNegativa++;
        }
    }
    cout << "broj false negativa nakon brisanja svih elemenata: " << brojFalseNegativa << "\n"; //ovdje ocekujemo broj 5000, jer su svi elementi izbrisani.
}