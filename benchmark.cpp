//Martina Dadić
#include "vacuum_filter.h"
#include <chrono>
#include <fstream>

using namespace std;
using namespace std::chrono;

int main(){
    ofstream results_file;
    results_file.open ("results.txt");
    ifstream data_summary_file("data/data_summary.csv");
    string line;
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
        results_file << "unique k-mers: " << line.substr(0, line.find(';')) << "\n";
        line.erase(0, line.find(';') + 1);
        results_file << "positive queries: " << line.substr(0, line.find(';')) << "\n";
        line.erase(0, line.find(';') + 1);
        results_file << "negative queries: " << line.substr(0, line.find(';')) << "\n";
        line.erase(0, line.find(';') + 1);
    }
    results_file.close();
    data_summary_file.close();
    return 0;
}

void statistika(string izvorPodataka, size_t k,vector<string> podaci){
    //podaci, k, brojBucketa, n=broj insertanih, vrijemeinserte, vrijeme lookupa, vrijeme removea, fals epositive rate, broj neuspjelih inserata, removea i lookupa
    //n = podaci.length()/4 * alpha (load factor, 0.95)
    //ispis u datoteku
}

void testiranjeVacuuma(){
    //ovdje testiram radi li moj vacuum filter ispravno
    VacuumFilter filter(10000000);
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