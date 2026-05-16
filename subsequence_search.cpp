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

string read_fasta(const string& filename) {
    string sequence;

    ifstream file(filename);
    string line;

    while (getline(file, line)) {
        if (!line.empty() && line[0] != '>') {
            sequence += line;
        }
    }

    return sequence;
}

vector<string> extract_kmers(const string& sequence, int k) {
    vector<string> kmers;

    for (int i = 0; i <= (int)sequence.size() - k; i++) {
        kmers.push_back(sequence.substr(i, k));
    }

    return kmers;
}

string generate_random_kmer(int k) {
    string base = "AGCT";
    string kmer;

    uniform_int_distribution<int> dist(0, 3);

    for (int i = 0; i < k; i++) {
        kmer += base[dist(rng)];
    }

    return kmer;
}

pair<vector<string>, vector<string>> generate_queries(
    const vector<string>& kmers,
    int k,
    int n_queries = 10000
) {
    set<string> kmer_set(kmers.begin(), kmers.end());

    vector<string> shuffled = kmers;
    shuffle(shuffled.begin(), shuffled.end(), rng);

    vector<string> pozitivni(
        shuffled.begin(),
        shuffled.begin() + min(n_queries, (int)shuffled.size())
    );

    vector<string> negativni;

    while ((int)negativni.size() < n_queries) {
        string candidate = generate_random_kmer(k);

        if (kmer_set.find(candidate) == kmer_set.end()) {
            negativni.push_back(candidate);
        }
    }

    return {pozitivni, negativni};
}

int main() {
    string artificial_filename = "sekvenca.fasta";

    string artificial_sequence = generate_sequence(1000000);
    save_fasta(artificial_filename, "Umjetno generiran genom", artificial_sequence);

    string ecoli_filename = "sequence.fasta";
    string ecoli_sequence = read_fasta(ecoli_filename);

    vector<int> k_vrijednosti = {10, 20, 50, 100, 200};
    int N_QUERIES = 10000;

    int k = k_vrijednosti[0];
    
    vector<pair<string, string>> datasets = {
        {"ecoli",      ecoli_sequence},
        {"artificial", artificial_sequence},
    };

    for (auto& [dataset_name, sekvenca] : datasets) {
        for (int k : k_vrijednosti) {
            vector<string> kmers = extract_kmers(sekvenca, k);
            auto [pozitivni, negativni] = generate_queries(kmers, k, N_QUERIES);
        }
    }

    return 0;
}