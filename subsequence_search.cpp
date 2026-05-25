// Frane Bazo
#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <random>
#include <set>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

const int RANDOM_SEED = 42;
const vector<int> DEFAULT_ARTIFICIAL_LENGTHS = {
    1000, 2000, 5000, 10000, 50000, 100000, 500000, 1000000, 5000000, 10000000};
const vector<int> DEFAULT_K_VALUES = {10, 20, 50, 100, 200};
const int DEFAULT_QUERY_COUNT = 10000;
const int MAX_NEGATIVE_ATTEMPTS_PER_QUERY = 1000;
const int FASTA_LINE_LENGTH = 60;
const string OUTPUT_DIRECTORY = "data";
const string SUMMARY_FILENAME = OUTPUT_DIRECTORY + "/data_summary.csv";

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

void save_fasta(const string& filename, const string& sequence_name,
                const string& sequence) {
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

vector<string> get_unique_kmers(const vector<string>& kmers) {
  set<string> seen_kmers;
  vector<string> unique_kmers;

  for (const string& kmer : kmers) {
    if (seen_kmers.find(kmer) == seen_kmers.end()) {
      seen_kmers.insert(kmer);
      unique_kmers.push_back(kmer);
    }
  }

  return unique_kmers;
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

bool create_output_directory() {
  if (filesystem::exists(OUTPUT_DIRECTORY)) {
    return true;
  }

  return filesystem::create_directory(OUTPUT_DIRECTORY);
}

string make_artificial_fasta_filename(int length) {
  return OUTPUT_DIRECTORY + "/artificial_len" + to_string(length) + ".fasta";
}

string make_insert_filename(const string& dataset_name, int k) {
  return OUTPUT_DIRECTORY + "/" + dataset_name + "_k" + to_string(k) +
         "_insert.txt";
}

string make_queries_filename(const string& dataset_name, int k) {
  return OUTPUT_DIRECTORY + "/" + dataset_name + "_k" + to_string(k) +
         "_queries.csv";
}

pair<vector<string>, vector<string>> generate_queries(
    const vector<string>& unique_kmers, int k,
    int query_count = DEFAULT_QUERY_COUNT) {
  set<string> kmer_set(unique_kmers.begin(), unique_kmers.end());

  vector<string> shuffled_kmers = unique_kmers;
  shuffle(shuffled_kmers.begin(), shuffled_kmers.end(), rng);

  vector<string> positive_queries(
      shuffled_kmers.begin(),
      shuffled_kmers.begin() + min(query_count, (int)shuffled_kmers.size()));

  vector<string> negative_queries;
  set<string> negative_query_set;

  int attempts = 0;
  int max_attempts = query_count * MAX_NEGATIVE_ATTEMPTS_PER_QUERY;

  while ((int)negative_queries.size() < query_count &&
         attempts < max_attempts) {
    attempts++;

    string candidate = generate_random_kmer(k);

    if (kmer_set.find(candidate) == kmer_set.end() &&
        negative_query_set.find(candidate) == negative_query_set.end()) {
      negative_query_set.insert(candidate);
      negative_queries.push_back(candidate);
    }
  }

  if ((int)negative_queries.size() < query_count) {
    cerr << "Warning: Generated only " << negative_queries.size()
         << " negative queries for k = " << k << "\n";
  }

  return {positive_queries, negative_queries};
}

void write_insert_file(const string& filename,
                       const vector<string>& unique_kmers) {
  ofstream file(filename);

  if (!file.is_open()) {
    cerr << "Error: Could not write insert file: " << filename << "\n";
    return;
  }

  for (const string& kmer : unique_kmers) {
    file << kmer << "\n";
  }
}

void write_queries_file(const string& filename,
                        const vector<string>& positive_queries,
                        const vector<string>& negative_queries) {
  ofstream file(filename);

  if (!file.is_open()) {
    cerr << "Error: Could not write queries file: " << filename << "\n";
    return;
  }

  file << "kmer;expected\n";

  for (const string& kmer : positive_queries) {
    file << kmer << ";" << 1 << "\n";
  }

  for (const string& kmer : negative_queries) {
    file << kmer << ";" << 0 << "\n";
  }
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

  if (!create_output_directory()) {
    cerr << "Error: Could not create output directory: " << OUTPUT_DIRECTORY
         << "\n";
    return 1;
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

    save_fasta(artificial_fasta_filename, "Artificially generated genome",
               artificial_sequence);

    datasets.push_back(
        {"artificial_len" + to_string(length), artificial_sequence});
  }

  ofstream summary_file(SUMMARY_FILENAME);

  if (!summary_file.is_open()) {
    cerr << "Error: Could not write summary file: " << SUMMARY_FILENAME << "\n";
    return 1;
  }

  summary_file
      << "dataset;sequence_length;k;total_kmers;unique_kmers;"
      << "positive_queries;negative_queries;insert_file;queries_file;seed\n";

  for (const auto& dataset : datasets) {
    string dataset_name = dataset.first;
    string sequence = dataset.second;

    for (int k : k_values) {
      vector<string> kmers = extract_kmers(sequence, k);
      vector<string> unique_kmers = get_unique_kmers(kmers);

      pair<vector<string>, vector<string>> queries =
          generate_queries(unique_kmers, k, query_count);
      vector<string> positive_queries = queries.first;
      vector<string> negative_queries = queries.second;

      string insert_filename = make_insert_filename(dataset_name, k);
      string queries_filename = make_queries_filename(dataset_name, k);

      write_insert_file(insert_filename, unique_kmers);
      write_queries_file(queries_filename, positive_queries, negative_queries);

      summary_file << dataset_name << ";" << sequence.size() << ";" << k << ";"
                   << kmers.size() << ";" << unique_kmers.size() << ";"
                   << positive_queries.size() << ";" << negative_queries.size()
                   << ";" << insert_filename << ";" << queries_filename << ";"
                   << RANDOM_SEED << "\n";
    }
  }

  return 0;
}