#include <iostream>
#include <cstdint>
#include <vector>

#include "SuperKmerLight.hpp"
#include "Kmers.hpp"



#ifndef BUCKETS_H
#define BUCKETS_H


//TODO CHANGE VECTOR of STruct TO Struct of vector
template <class DATA>
class Bucket{
public:
	static void set_parameters(uint8_t k, uint8_t m) {
		Bucket::k = k;
		Bucket::minimizer_size = m;
		SKCL<DATA>::set_parameters(k, m);
	}

	vector<SKCL<DATA> > skml;
	uint32_t sorted_size;


	Bucket();
	~Bucket();
	// // void add_kmer(kmer_full & kmer);
	DATA * insert_kmer(kmer_full & kmer);
	// uint64_t print_kmers(string& result,const  string& mini, robin_hood::unordered_flat_map<string, uint8_t> & real_count, robin_hood::unordered_map<kint, uint8_t> * cursed_kmers, bool check)const;
	// uint64_t size()const;
	// uint64_t number_kmer()const;
	DATA * find_kmer(kmer_full& kmer);
	// uint64_t number_kmer_counted()const;


	void next_kmer(kmer_full & kmer, kint minimizer);
	bool has_next_kmer();

private:
	static uint8_t k;
	static uint8_t minimizer_size;

	SKCL<DATA> * buffered_skmer;
	uint8_t * nucleotides_reserved_memory;
	uint32_t skmer_reserved;
	uint64_t next_data;
	uint64_t data_reserved_number;
	DATA * data_reserved_memory;

	DATA * find_kmer_unsorted(kmer_full& kmer);
	DATA * find_kmer_from_interleave(kmer_full& kmer, SKCL<DATA> & mockskm, uint8_t * mock_nucleotides);
	DATA * insert_kmer_buffer(kmer_full & kmer);
	void insert_buffer();
	void data_space_update();

	uint32_t enumeration_skmer_idx;
	uint8_t enumeration_kmer_idx;
};

// Static variable definition
template <class DATA>
uint8_t Bucket<DATA>::k = 0;
template <class DATA>
uint8_t Bucket<DATA>::minimizer_size = 0;


template <class DATA>
Bucket<DATA>::Bucket() {
	this->sorted_size = 0;
	this->buffered_skmer = (SKCL<DATA> *)NULL;

	this->skml.reserve(10);
	this->nucleotides_reserved_memory = (uint8_t *)malloc(SKCL<DATA>::allocated_bytes * skml.capacity());
	this->next_data = 0;
	this->data_reserved_number = 10;
	this->data_reserved_memory = (DATA *)malloc(sizeof(DATA) * this->data_reserved_number);

	enumeration_skmer_idx = 0;
	enumeration_kmer_idx = 0;
}


template <class DATA>
Bucket<DATA>::~Bucket() {
	free(this->nucleotides_reserved_memory);
	free(this->data_reserved_memory);
}


template <class DATA>
void Bucket<DATA>::data_space_update() {
	if (this->next_data == this->data_reserved_number) {
		size_t to_reserve = (int)min(1000., 0.5 * this->data_reserved_number);
		this->data_reserved_memory = (DATA *)realloc(this->data_reserved_memory, sizeof(DATA) * (this->data_reserved_number + to_reserve));
		this->data_reserved_number += to_reserve;
	}
}


template <class DATA>
DATA * Bucket<DATA>::insert_kmer(kmer_full & kmer) {
	this->data_space_update();
	DATA * value = this->insert_kmer_buffer(kmer);
	this->next_data += 1;

	if(skml.size()-sorted_size>10){
		this->insert_buffer();
	}

	return value;
}


template <class DATA>
void Bucket<DATA>::insert_buffer(){
	for(auto it(skml.begin()+sorted_size);it<skml.end();++it){
		it->interleaved=it->interleaved_value(
				this->nucleotides_reserved_memory + it->idx * SKCL<DATA>::allocated_bytes);
	}
	sort(skml.begin()+sorted_size , skml.end(),[ ]( const SKCL<DATA>& lhs, const SKCL<DATA>& rhs ){return lhs.interleaved < rhs.interleaved;});
	inplace_merge(skml.begin(), skml.begin()+sorted_size ,skml.end() ,[ ]( const SKCL<DATA>& lhs, const SKCL<DATA>& rhs ){return lhs.interleaved < rhs.interleaved;});
	sorted_size=skml.size();
}


template <class DATA>
DATA * Bucket<DATA>::insert_kmer_buffer(kmer_full & kmer){
	cout << "INSERT" << endl;
	DATA * value_pointer = NULL;
	this->data_space_update();

	//WE TRY TO COMPACT IT TO THE LAST SUPERKMER
	if (buffered_skmer != NULL) {
		value_pointer = buffered_skmer->compact_right(
				kmer,
				this->nucleotides_reserved_memory +
						SKCL<DATA>::allocated_bytes * buffered_skmer->idx,
				data_reserved_memory + buffered_skmer->data_idx
		);
	}

	// Create a new superkmer
	if(value_pointer == NULL){
		if(skml.size()==skml.capacity()){
			skml.reserve(skml.capacity()*1.5);
			this->nucleotides_reserved_memory = (uint8_t *)realloc(
											this->nucleotides_reserved_memory,
											skml.capacity() * SKCL<DATA>::allocated_bytes
			);
			// print_kmer(skml[0].get_ith_kmer(0, nucleotides_reserved_memory), 31); cout << endl;
		}

		skml.emplace_back(
			kmer.get_compacted(SKCL<DATA>::minimizer_size),
			(int)kmer.minimizer_idx,
			skml.size(),
			this->nucleotides_reserved_memory + (this->skml.size() * SKCL<DATA>::allocated_bytes),
			this->next_data
		);
		print_kmer(skml[skml.size()-1].get_skmer(
				this->nucleotides_reserved_memory + (skml[skml.size()-1].idx * SKCL<DATA>::allocated_bytes),
				0
		), k); cout << " skmer" << endl;
		buffered_skmer = &(skml[skml.size()-1]);
		value_pointer = data_reserved_memory + this->next_data;
	}

	cout << "/INSERT" << endl;
	return value_pointer;
}


template <class DATA>
DATA * Bucket<DATA>::find_kmer_from_interleave(kmer_full& kmer, SKCL<DATA> & mockskm, uint8_t * mock_nucleotides){
	DATA * data_pointer = NULL;

	uint64_t low = lower_bound(
		skml.begin(),
		skml.begin() + sorted_size,
		mockskm,
		[ ]( const SKCL<DATA>& lhs, const SKCL<DATA>& rhs ){
			return lhs.interleaved < rhs.interleaved;
		}
	) - skml.begin();
	uint64_t value_max = mockskm.interleaved_value_max(mock_nucleotides, 5);
	
	while (data_pointer == NULL and low < (uint64_t)sorted_size and skml[low].interleaved <= value_max) {
		cout << "*** QUERY INTERLEAVED ***" << endl;
		data_pointer = skml[low].query_kmer(kmer,
					this->nucleotides_reserved_memory + SKCL<DATA>::allocated_bytes * skml[low].idx,
					this->data_reserved_memory + skml[low].data_idx
		);
		low++;
	}

	return data_pointer;
}


template <class DATA>
DATA * Bucket<DATA>::find_kmer_unsorted(kmer_full& kmer) {
	DATA * value_pointer = NULL;
	for (uint i=sorted_size ; value_pointer == NULL and i<skml.size() ; i++) {
		value_pointer = skml[i].query_kmer(
				kmer,
				this->nucleotides_reserved_memory + SKCL<DATA>::allocated_bytes * skml[i].idx,
				this->data_reserved_memory + skml[i].data_idx
		);
	}

	return value_pointer;
}


template <class DATA>
DATA * Bucket<DATA>::find_kmer(kmer_full& kmer) {
	static uint8_t * nucleotides_area = new uint8_t[SKCL<DATA>::allocated_bytes];
	// cout << "Bucket - find_kmer" << endl;
	SKCL<DATA> mockskm = SKCL<DATA>(kmer.get_compacted(SKCL<DATA>::minimizer_size), kmer.minimizer_idx, 0, nucleotides_area, 0);
	mockskm.interleaved = mockskm.interleaved_value(nucleotides_area);
	
	uint prefix_size = mockskm.prefix_size();
	uint suffix_size = mockskm.suffix_size();

	uint size_interleave = min(prefix_size,suffix_size) * 2;
	// cout << "Size " << size_interleave << endl;
	cout << "skmer number: " << skml.size() << endl;
	for (auto & skmer : skml) {
		kint skmer_val = skmer.get_skmer(nucleotides_reserved_memory + (skmer.idx * SKCL<DATA>::allocated_bytes),0);
		print_kmer(
				skmer_val,
				skmer.size + k - 1
		); cout << endl;
	}

	if(size_interleave>=6){
		cout << "NORMAL INTERLEAVED" << endl;
		DATA * val_pointer = find_kmer_from_interleave(kmer, mockskm, nucleotides_area);
		// cout << (uint *)val_pointer << endl;
		// cout << "/Bucket - find_kmer" << endl;
		return val_pointer;
	}else{
		cout << "TRUNCKATED INTERLEAVED " << size_interleave << endl;
		if(suffix_size>prefix_size){
			//SUFFIX IS LARGER PREFIX IS MISSING
			if(size_interleave==4){
				for(uint64_t i(0);i<4;++i){
					mockskm.interleaved+=i<<52;
					DATA * value_pointer = find_kmer_from_interleave(kmer, mockskm, nucleotides_area);
					if(value_pointer != NULL){return value_pointer;}
					mockskm.interleaved-=i<<52;
				}
			}
			if(size_interleave==2){
				cout << "2 missing nucl from prefix" << endl;
				for(uint64_t ii(0);ii<4;++ii){
					mockskm.interleaved+=ii<<56;
					for(uint64_t i(0);i<4;++i){
						mockskm.interleaved+=i<<52;
						cout << "interleaved "; print_kmer(mockskm.interleaved >> 48, 8); cout << endl;
						DATA * value_pointer = find_kmer_from_interleave(kmer, mockskm, nucleotides_area);
						if(value_pointer != NULL){return value_pointer;}
						mockskm.interleaved-=i<<52;
					}
					mockskm.interleaved-=ii<<56;
				}
			}
			if(size_interleave==0){
				//~ cout<<"0 prefix"<<endl;
				for(uint64_t iii(0);iii<4;++iii){
					mockskm.interleaved+=iii<<60;
					for(uint64_t ii(0);ii<4;++ii){
						mockskm.interleaved+=ii<<56;
						for(uint64_t i(0);i<4;++i){
							mockskm.interleaved+=i<<52;
							DATA * value_pointer = find_kmer_from_interleave(kmer, mockskm, nucleotides_area);
							if(value_pointer != NULL){return value_pointer;}
							mockskm.interleaved-=i<<52;
						}
						mockskm.interleaved-=ii<<56;
					}
					mockskm.interleaved-=iii<<60;
				}
			}
		}else{
			//PREFIX IS LARGER, SUFFIX IS MISSING
			if(size_interleave==4){
				for(uint64_t i(0);i<4;++i){
					mockskm.interleaved+=i<<54;
					DATA * value_pointer = find_kmer_from_interleave(kmer, mockskm, nucleotides_area);
					if(value_pointer != NULL){return value_pointer;}
					mockskm.interleaved-=i<<54;
				}
			}
			if(size_interleave==2){
				for(uint64_t ii(0);ii<4;++ii){
					mockskm.interleaved+=ii<<58;
					for(uint64_t i(0);i<4;++i){
						mockskm.interleaved+=i<<54;
						DATA * value_pointer = find_kmer_from_interleave(kmer, mockskm, nucleotides_area);
						if(value_pointer != NULL){return value_pointer;}
						mockskm.interleaved-=i<<54;
					}
					mockskm.interleaved-=ii<<58;
				}
			}
			if(size_interleave==0){
				for(uint64_t iii(0);iii<4;++iii){
					mockskm.interleaved+=iii<<62;
					for(uint64_t ii(0);ii<4;++ii){
						mockskm.interleaved+=ii<<58;
						for(uint64_t i(0);i<4;++i){
							mockskm.interleaved+=i<<54;
							DATA * value_pointer = find_kmer_from_interleave(kmer, mockskm, nucleotides_area);
							if(value_pointer != NULL){return value_pointer;}
							mockskm.interleaved-=i<<54;
						}
						mockskm.interleaved-=ii<<58;
					}
					mockskm.interleaved-=iii<<62;
				}
			}
		}
	}

	return find_kmer_unsorted(kmer);
}


template <class DATA>
bool Bucket<DATA>::has_next_kmer() {
	if (enumeration_skmer_idx >= skml.size())
		return false;

	SKCL<DATA> & skmer = skml[enumeration_skmer_idx];
	if (enumeration_kmer_idx >= skmer.size) {	
		enumeration_skmer_idx += 1;
		enumeration_kmer_idx = 0;
		return has_next_kmer();
	}	

	return true;
}


template <class DATA>
void Bucket<DATA>::next_kmer(kmer_full & kmer, kint minimizer) {
	// Nothing to do here
	if (not has_next_kmer())
		return;

	SKCL<DATA> & skmer = skml[enumeration_skmer_idx];
	skmer.get_kmer(
			enumeration_kmer_idx,
			this->nucleotides_reserved_memory + (skmer.idx * SKCL<DATA>::allocated_bytes),
			minimizer,
			kmer
	);
	
	enumeration_kmer_idx += 1;
}


#endif
