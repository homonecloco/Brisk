#include <iostream>
#include <stdint.h>
#include <chrono>

#include "CLI11.hpp"
#include "zstr.hpp"
#include "Brisk2.hpp"

using namespace std;

// --- Useful functions to count kmers ---
void count_fasta(Brisk<uint8_t> counter, string filename);
void count_sequence(Brisk<uint8_t> counter, string sequence);


int parse_args(int argc, char** argv, string & fasta, uint8_t & k, uint8_t & m,
								uint & mode) {
	CLI::App app{"Brisk library demonstrator - kmer counter"};

  auto file_opt = app.add_option("-f,--file", fasta, "Fasta file to count");
  file_opt->required();
  app.add_option("-k", k, "Kmer size");
  app.add_option("-m", m, "Minimizer size");
  app.add_option("--mode", mode, "Execution mode (0: output count, no checking | 1: performance mode, no output | 2: debug mode");

  CLI11_PARSE(app, argc, argv);
  return 0;
}


int main(int argc, char** argv) {
	string fasta = "";
	uint8_t k=31, m=11;
	uint mode = 0;

	parse_args(argc, argv, fasta, k, m, mode);
  cout << fasta << " " << (uint)k << " " << (uint)m << endl;

	// if (mode > 1) {
	// 	check = true;
	// 	cout << "LETS CHECK THE RESULTS" << endl;
	// }


	cout << "\n\n\nI count " << fasta << endl;
	cout << "Kmer size:	" << (uint)k << endl;
	cout << "Minimizer size:	" << (uint)m << endl;
  
	auto start = std::chrono::system_clock::now();
	Brisk<uint8_t> counter(k, m);
	count_fasta(counter, fasta);
	
	auto end = std::chrono::system_clock::now();
	chrono::duration<double> elapsed_seconds = end - start;
	cout << "Kmer counted elapsed time: " << elapsed_seconds.count() << "s\n";
	cout << endl;

	// if (mode == 2) {
	// 	cout<<menu.dump_counting()<<" errors"<<endl;;
	// }
	// menu.dump_stats();

	return 0;
}


void clean_dna(string& str){
	for(uint i(0); i< str.size(); ++i){
		switch(str[i]){
			case 'a':break;
			case 'A':break;
			case 'c':break;
			case 'C':break;
			case 'g':break;
			case 'G':break;
			case 't':break;
			case 'T':break;
			// case 'N':break;
			// case 'n':break;
			default:  str[i]='A';break;;
		}
	}
	transform(str.begin(), str.end(), str.begin(), ::toupper);
}

string getLineFasta(zstr::ifstream* in) {
	string line, result;
	getline(*in, line);
	char c = static_cast<char>(in->peek());
	while (c != '>' and c != EOF) {
		getline(*in, line);
		result += line;
		c = static_cast<char>(in->peek());
	}
	clean_dna(result);
	return result;
}

/** Counter function.
  * Read a complete fasta file line by line and store the counts into the Brisk datastructure.
  */
void count_fasta(Brisk<uint8_t> counter, string filename) {
	// Test file existance
	struct stat exist_buffer;
  bool file_existance = (stat (filename.c_str(), &exist_buffer) == 0);
	if(not file_existance){
		cerr<<"Problem with file opening:	"<<filename<<endl;
		exit(1);
	}

	// Read file line by line
	uint nb_core = 1;
	zstr::ifstream in(filename);
	vector<string>  buffer;
	uint line_count = 0;

	#pragma omp parallel num_threads(nb_core)
	{
		string line;
		while (in.good() or not buffer.empty()) {
			#pragma omp critical(input)
			{
				if(not buffer.empty()){
					line=buffer[buffer.size()-1];
					buffer.pop_back();
				}else{
					line = getLineFasta(&in);
					if(line.size()>100000000000){
						buffer.push_back(line.substr(0,line.size()/4));
						buffer.push_back(line.substr(line.size()/4-counter.k+1,line.size()/4+counter.k-1));
						buffer.push_back(line.substr(line.size()/2-counter.k+1,line.size()/4+counter.k-1));
						line=line.substr(3*line.size()/4-counter.k+1);
					}
				}

			}
			count_sequence(counter, line);
			line_count++;
		}
	}
}


void count_sequence(Brisk<uint8_t> counter, string sequence) {
	// Line too short
	if (sequence.size() < counter.k)
		return;
}


// void count_line(string& line) {
// 	//~ cout<<"COUNT LINE"<<endl;
// 	if (line.size() < k) {
// 		return;
// 	}
// 	clean(line);
// 	vector<kmer_full> kmers;
// 	//~ cout<<"count line"<<endl;
// 	// Init Sequences
// 	kint kmer_seq = (str2num(line.substr(0, k))), kmer_rc_seq(rcb(kmer_seq));
// 	uint64_t min_seq  = (uint64_t)(str2num(line.substr(k - super_minimizer_size, super_minimizer_size))), min_rcseq(rcbc(min_seq, super_minimizer_size)), min_canon(min(min_seq, min_rcseq));
// 	// Init MINIMIZER
// 	int8_t relative_min_position;
// 	int64_t minimizer = get_minimizer(kmer_seq, relative_min_position);
// 	bool multiple_min  = relative_min_position < 0;

// 	// cout << "is multiple " << multiple_min << endl;
// 	// print_kmer(minimizer, super_minimizer_size);
// 	// cout << endl;

// 	// cout << "relative " << (int)relative_min_position << endl;
// 	uint8_t position_minimizer_in_kmer;
	
// 	if (multiple_min){
// 		position_minimizer_in_kmer = (uint8_t)(-relative_min_position - 1);
// 	}else{
// 		position_minimizer_in_kmer = relative_min_position;
// 	}

// 	// cout << "absolute " << (uint)position_minimizer_in_kmer << endl;

// 	uint64_t hash_mini = hash64shift(abs(minimizer));
// 	if (multiple_min) {
// 		kint canon(min(kmer_rc_seq,kmer_seq));
// 		mutex_cursed[canon%1024].lock();
// 		cursed_kmers[canon%1014][canon]++;
// 		mutex_cursed[canon%1024].unlock();
// 	} else {
// 		if(minimizer<0){
// 			kmers.push_back({k-relative_min_position-super_minimizer_size+2, kmer_rc_seq});
// 		}else{
// 			if(kmer_seq!=0){//PUT COMPLEXITY THESHOLD
// 				kmers.push_back({relative_min_position+2, kmer_seq});
// 			}
// 		}
// 	}

// 	if (check) {
// 		real_count[getCanonical(line.substr(0, k))]++;
// 	}

// 	uint64_t line_size = line.size();
// 	for (uint64_t i = 0; i + k < line_size; ++i) {

// 		// Update KMER and MINIMIZER candidate with the new letter
// 		updateK(kmer_seq, line[i + k]);
// 		updateRCK(kmer_rc_seq, line[i + k]);
// 		updateM(min_seq, line[i + k]);
// 		updateRCM(min_rcseq, line[i + k]);
// 		min_canon = (min(min_seq, min_rcseq));

// 		//THE NEW mmer is a MINIMIZER
// 		uint64_t new_hash = (hash64shift(min_canon));
// 		//the previous MINIMIZER is outdated
// 		// cout << (int)position_minimizer_in_kmer << " " << multiple_min << " " << k << " " << super_minimizer_size << endl;
// 		if (position_minimizer_in_kmer >= (k - super_minimizer_size)) {
// 			//~ cout << "Outdated mini" << endl;
// 			if(minimizer<0){
// 				reverse(kmers.begin(),kmers.end());
// 				minimizer*=-1;
// 			}
// 			// print_kmer(minimizer, super_minimizer_size);
// 			// cout << " outdated minimizer" << endl;
// 			menu.add_kmers(kmers,minimizer/16);
// 			// Search for the new MINIMIZER in the whole kmer
// 			minimizer    = get_minimizer(kmer_seq, relative_min_position);
// 			multiple_min = (relative_min_position < 0);
// 			if (multiple_min){
// 				position_minimizer_in_kmer = (uint8_t)(-relative_min_position-1);
// 			}else{
// 				position_minimizer_in_kmer = relative_min_position;
// 			}
// 			hash_mini = hash64shift(abs(minimizer));
// 		}
// 		else if (new_hash < hash_mini) {
// 			// cout << "New mini" << endl;
// 			// print_kmer(min_canon, super_minimizer_size);
// 			// cout << endl;
// 			// print_kmer(min_seq, super_minimizer_size);
// 			// cout << endl;

// 			// Clear the previous kmer list
// 			if(minimizer<0){
// 				reverse(kmers.begin(),kmers.end());
// 				minimizer*=-1;
// 			}
// 			// print_kmer(minimizer, super_minimizer_size);
// 			// cout << " new hash" << endl;
// 			menu.add_kmers(kmers,minimizer/16);

// 			// Create the new minimizer
// 			minimizer                  = (min_canon);
// 			hash_mini                  = new_hash;
// 			multiple_min                                       = false;
// 			position_minimizer_in_kmer = relative_min_position = 0;
// 			if(min_canon!=min_seq){
// 				minimizer*=-1;
// 			}
// 		}
// 		// duplicated MINIMIZER
// 		else if (new_hash == hash_mini) {
// 			//~ cout << "Duplicate mini" << endl;
// 			multiple_min = true;
// 			position_minimizer_in_kmer ++;
// 			relative_min_position = -((int8_t)position_minimizer_in_kmer) - 1;
// 			//~ cout<<relative_min_position<<endl;
// 		}
// 		else {
// 			//~ cout<<(int)position_minimizer_in_kmer<<" "<<(int)k - (int)super_minimizer_size-1<<endl;
// 			//~ cout << "Nothing special" << endl;
// 			position_minimizer_in_kmer++;
// 			if (multiple_min){
// 				relative_min_position--;
// 			}else{
// 				relative_min_position++;
// 			}
// 		}

// 		// TODO: Multi-minimizer process
// 		if (multiple_min) {
// 			kint canon(min(kmer_rc_seq,kmer_seq));
// 			mutex_cursed[canon%1024].lock();
// 			cursed_kmers[canon%1014][canon]++;
// 			mutex_cursed[canon%1024].unlock();
// 		} else {
// 			// Normal add of the kmer into kmer list
// 			if(minimizer<0){
// 				int8_t val = ((int8_t)k) - ((int8_t)relative_min_position) - ((int8_t)super_minimizer_size) + 2;
// 				// kmers.push_back({k-relative_min_position-super_minimizer_size+4, kmer_rc_seq});
// 				kmers.push_back({val, kmer_rc_seq});
// 			}else{
// 				if(kmer_seq!=0){//PUT COMPLEXITY THESHOLD
// 					kmers.push_back({relative_min_position+2, kmer_seq});
// 				}
// 			}
// 		}
// 		if (check) {
// 			real_count[getCanonical(line.substr(i + 1, k))]++;
// 		}
// 	}
// 	if(minimizer<0){
// 		reverse(kmers.begin(),kmers.end());
// 		minimizer*=-1;
// 	}
// 	// print_kmer(minimizer, super_minimizer_size);
// 	// cout << " end read" << endl;
// 	menu.add_kmers(kmers,minimizer/16);
// 	//~ menu.dump_counting();
// }

