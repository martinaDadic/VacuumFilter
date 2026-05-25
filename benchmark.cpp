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

        VacuumFilter filter(noOfBuckets, uniqueKmers);
        string insertFile = line.substr(0, line.find(';'));
        line.erase(0, line.find(';') + 1);
        results_file << vrijemeInserta(filter,insertFile) << ",";
        results_file << vrijemeLookupa(filter,line.substr(0, line.find(';')), negativeQueries) << ",";
        line.erase(0, line.find(';') + 1);
        results_file << vrijemeRemovea(filter,insertFile) << ",";
        results_file << line.substr(0, line.find(';')) << ",";
        results_file << filter.memory_consumption();
        results_file << "\n";
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
    return to_string(elapsed_time) + "," + to_string(brojNeuspjelihInserta);
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
    return to_string(elapsed_time) + "," + to_string(brojFalseNegativa) + "," + to_string(falsePositiveRate);
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
    return to_string(elapsed_time) + "," + to_string(brojNeuspjelihRemovea);
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