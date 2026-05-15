#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <set>
#include <algorithm>
#include <random>
#include <filesystem>

using namespace std;

mt19937 rng(42);

string generate_sequence(int size) {
    string base = "AGCT";
    string sequence;
    sequence.reserve(size);

    uniform_int_distribution<int> dist(0, 3);

    for (int i = 0; i < size; i++) {
        sequence += base[dist(rng)];
    }

    return sequence;
}

void save_fasta(const string& filename, const string& name, const string& sequence) {
    ofstream file(filename);

    file << ">" << name << "\n";

    for (int i = 0; i < (int)sequence.size(); i += 60) {
        file << sequence.substr(i, 60) << "\n";
    }
}

int main() {
    string artificial_filename = "sekvenca.fasta";

    string artificial_sequence = generate_sequence(1000000);
    save_fasta(artificial_filename, "Umjetno generiran genom", artificial_sequence);

    return 0;
}