#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <set>
#include <algorithm>
#include <random>

using namespace std;

const int RANDOM_SEED = 42;
const int ARTIFICIAL_SEQUENCE_LENGTH = 1000000;
const int DEFAULT_QUERY_COUNT = 10000;
const int FASTA_LINE_LENGTH = 60;

mt19937 rng(RANDOM_SEED);

string generate_sequence(int length) {
    string dna_bases = "AGCT";
    string sequence;
    sequence.reserve(length);

    uniform_int_distribution<int> dist(0, 3);

    for (int i = 0; i < length; i++) {
        sequence += dna_bases[dist(rng)];
    }

    return sequence;
}

void save_fasta(const string& filename, const string& sequence_name, const string& sequence) {
    ofstream file(filename);

    file << ">" << sequence_name << "\n";

    for (int i = 0; i < (int)sequence.size(); i += FASTA_LINE_LENGTH) {
        file << sequence.substr(i, FASTA_LINE_LENGTH) << "\n";
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
    string dna_bases = "AGCT";
    string kmer;
    kmer.reserve(k);

    uniform_int_distribution<int> dist(0, 3);

    for (int i = 0; i < k; i++) {
        kmer += dna_bases[dist(rng)];
    }

    return kmer;
}

pair<vector<string>, vector<string>> generate_queries(
    const vector<string>& kmers,
    int k,
    int query_count = DEFAULT_QUERY_COUNT
) {
    set<string> kmer_set(kmers.begin(), kmers.end());

    vector<string> unique_kmers(kmer_set.begin(), kmer_set.end());
    shuffle(unique_kmers.begin(), unique_kmers.end(), rng);

    vector<string> positive_queries(
        unique_kmers.begin(),
        unique_kmers.begin() + min(query_count, (int)unique_kmers.size())
    );

    vector<string> negative_queries;

    while ((int)negative_queries.size() < query_count) {
        string candidate = generate_random_kmer(k);

        if (kmer_set.find(candidate) == kmer_set.end()) {
            negative_queries.push_back(candidate);
        }
    }

    return {positive_queries, negative_queries};
}

int main() {
    string artificial_fasta_filename = "artificial_genome.fasta";

    string artificial_sequence = generate_sequence(ARTIFICIAL_SEQUENCE_LENGTH);
    save_fasta(artificial_fasta_filename, "Artificially generated genom", artificial_sequence);

    string ecoli_fasta_filename = "sequence.fasta";
    string ecoli_sequence = read_fasta(ecoli_fasta_filename);

    vector<int> k_values = {10, 20, 50, 100, 200};
    int query_count = DEFAULT_QUERY_COUNT;
    
    vector<pair<string, string>> datasets = {
        {"ecoli",      ecoli_sequence},
        {"artificial", artificial_sequence},
    };

    for (auto& [dataset_name, sequence] : datasets) {
        for (int k : k_values) {
            vector<string> kmers = extract_kmers(sequence, k);
            auto [positive_queries, negative_queries] = generate_queries(kmers, k, query_count);
        }
    }

    return 0;
}