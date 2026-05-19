#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <set>
#include <algorithm>
#include <random>
#include <cctype>
#include <sstream>

using namespace std;

const int RANDOM_SEED = 42;
const vector<int> DEFAULT_ARTIFICIAL_LENGTHS = {1000, 10000, 100000, 1000000};
const vector<int> DEFAULT_K_VALUES = {10, 20, 50, 100, 200};
const int DEFAULT_QUERY_COUNT = 10000;
const int FASTA_LINE_LENGTH = 60;

mt19937 rng(RANDOM_SEED);

bool is_valid_dna_base(char base) {
    return base == 'A' || base == 'C' || base == 'G' || base == 'T';
}

bool is_valid_kmer(const string& kmer) {
    for (char base : kmer) {
        if (!is_valid_dna_base(base)) {
            return false;
        }
    }

    return true;
}

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

    if (!file.is_open()) {
        cerr << "Error: Could not open FASTA file: " << filename << "\n";
        return sequence;
    }

    string line;

    while (getline(file, line)) {
        if (line.empty() || line[0] == '>') {
            continue;
        }

        for (char character : line) {
            if (!isspace(character)) {
                sequence += toupper(character);
            }
        }
    }

    return sequence;
}

vector<string> extract_kmers(const string& sequence, int k) {
    vector<string> kmers;

    if ((int)sequence.size() < k) {
        return kmers;
    }

    kmers.reserve(sequence.size() - k + 1);

    for (int i = 0; i <= (int)sequence.size() - k; i++) {
        string kmer = sequence.substr(i, k);

        if (is_valid_kmer(kmer)) {
            kmers.push_back(kmer);
        }
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

vector<int> parse_int_list(const string& text) {
    vector<int> values;
    stringstream ss(text);
    string item;

    while (getline(ss, item, ',')) {
        if (!item.empty()) {
            values.push_back(stoi(item));
        }
    }

    return values;
}

string make_artificial_fasta_filename(int length) {
    return "artificial_len" + to_string(length) + ".fasta";
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

int main(int argc, char* argv[]) {
    string ecoli_fasta_filename = "sequence.fasta";
    int query_count = DEFAULT_QUERY_COUNT;
    vector<int> artificial_lengths = DEFAULT_ARTIFICIAL_LENGTHS;
    vector<int> k_values = DEFAULT_K_VALUES;

    if (argc >= 2) {
        ecoli_fasta_filename = argv[1];
    }

    if (argc >= 3) {
        query_count = stoi(argv[2]);
    }

    if (argc >= 4) {
        artificial_lengths = parse_int_list(argv[3]);
    }

    if (argc >= 5) {
        k_values = parse_int_list(argv[4]);
    }

    string ecoli_sequence = read_fasta(ecoli_fasta_filename);

    if (ecoli_sequence.empty()) {
        return 1;
    }
    
    vector<pair<string, string>> datasets;
    datasets.push_back({"ecoli", ecoli_sequence});

    for (int length : artificial_lengths) {
        string artificial_sequence = generate_sequence(length);
        string artificial_fasta_filename = make_artificial_fasta_filename(length);

        save_fasta(artificial_fasta_filename, "Artificially generated genome", artificial_sequence);

        datasets.push_back({"artificial_len" + to_string(length), artificial_sequence});
    }

    for (const auto& dataset : datasets) {
        string dataset_name = dataset.first;
        string sequence = dataset.second;

        for (int k : k_values) {
            vector<string> kmers = extract_kmers(sequence, k);

            pair<vector<string>, vector<string>> queries = generate_queries(kmers, k, query_count);
            vector<string> positive_queries = queries.first;
            vector<string> negative_queries = queries.second;
        }
    }

    return 0;
}