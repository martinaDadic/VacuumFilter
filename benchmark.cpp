//Martina Dadić
#include "vacuum_filter.h"
#include <chrono>

using namespace std;
using namespace std::chrono;

int main(){
    auto start = high_resolution_clock::now();
    auto end = high_resolution_clock::now();
    duration<double> elapsed = end - start;
    double elapsed_time = elapsed.count();
    //ovdje testiram radi li moj vacuum filter ispravno
    VacuumFilter filter(5000);
    cout << "Load Factor Test za n=1000, alpha=0.95, r=0.75 i L=20: " << filter.LoadFactorTest(1000, 0.95, 0.75, 20) <<"\n";
    cout << "Range selection za n=1000, alpha=0.95, r=0.75: " << filter.RangeSelection(1000, 0.95, 0.75) <<"\n";
    int brojNeuspjelihInserta=0;
    int brojFalseNegativa=0;
    int brojNeuspjelihRemovea=0;
    int brojFalsePositivea=0;
    for (int i=0;i<5000;i++){
        if(!filter.insert(to_string(i))){
            brojNeuspjelihInserta++;
        }
    }
    cout << "broj neuspjeha inserta: " << brojNeuspjelihInserta << "\n";
    for (int i=0;i<5000;i++){
        if(!filter.lookup(to_string(i))){
            brojFalseNegativa++;
        }
    }
    cout << "broj false negativa: " << brojFalseNegativa << "\n";
    for (int i=0;i<5000;i++){
        if(!filter.remove(to_string(i))){
            brojNeuspjelihRemovea++;
        }
    }
    cout << "broj neuspjeha removea: " << brojNeuspjelihRemovea << "\n";
    for (int i=5000;i<6000;i++){
        if(filter.lookup(to_string(i))){
            brojFalsePositivea++;
        }
    }
    cout << "broj false pozitiva: " << brojFalsePositivea << "\n";
    brojFalseNegativa=0;
    for (int i=0;i<5000;i++){
        if(!filter.lookup(to_string(i))){
            brojFalseNegativa++;
        }
    }
    cout << "broj false negativa nakon brisanja svih elemenata: " << brojFalseNegativa << "\n"; //ovdje ocekujemo broj 5000, jer su svi elementi izbrisani.
}