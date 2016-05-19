#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include "c-sim.h"

struct Block{
	int valid, index, timestamp;
	uint32_t tag, address;
};

int reads, writes, hits, misses = 0;

struct Block* init_block(){
	struct Block* b = (struct Block*)malloc(sizeof(struct Block));
	b->valid = 0;

	return b;
}

struct Block* create_block(uint32_t tag, int index, int timestamp, uint32_t address, uint32_t ip){
	struct Block* b = (struct Block*)malloc(sizeof(struct Block));
	b->valid = 1;
	b->tag = tag;
	b->index = index;
	b->timestamp = timestamp;
	b->address = address;

	return b;
}

void direct(int set_n, FILE *file, int block_size){
	struct Block** cache = (struct Block**)malloc(sizeof(struct Block*)*set_n);
	int i;
	for(i = 0; i < set_n; i++){
		cache[i] = init_block();
	} 

	char inst;
	char str[1000], str2[1000];
	uint32_t ip, address;
	int set;
	uint32_t tag;

	while(fscanf(file, "%s %c %s", str, &inst, str2) == 3){
		if(strcmp(str, "#eof") != 0){
			ip = strtoul(str, NULL, 16);
			address = strtoul(str2, NULL, 16);
			int cal1 = ceil(log(block_size)/log(2));	
			uint32_t tmp = address >> cal1;
			int cal2 = ceil(log(set_n)/log(2));
			unsigned mask = ((1 << cal2) - 1); 
			set = tmp & mask;
			tag = tmp >> cal2;
	
			if(cache[set]->valid == 0){
				cache[set] = create_block(tag, set, 0, address, ip);
				misses++;
				reads++;
				if(inst == 'W'){
					writes++;
				}
			}else if(cache[set]->tag == tag){
				hits++;
				if(inst == 'W'){
					writes++;
				}
			}else{
				cache[set] = create_block(tag, set, 0, address, ip);
				misses++;
				reads++;
				if(inst == 'W'){
					writes++; 
				}
			}			
		}			
	}

	return;
}

void full_assoc(int block_n, FILE *file, int block_size){
	struct Block** cache = (struct Block**)malloc(sizeof(struct Block*)*block_n);
	int i;
	for(i = 0; i < block_n; i++){
		cache[i] = init_block();
	}
	
	char inst;
	char str[1000], str2[1000];
	uint32_t ip, address, tag;
	int timestamp = -1;

	while(fscanf(file, "%s %c %s", str, &inst, str2) == 3){
		if(strcmp(str, "#eof") != 0){
			ip  = strtoul(str, NULL, 16);
			address = strtoul(str2, NULL, 16);
			int cal = ceil(log(block_size)/log(2));
			tag = address >> cal;
			
			int found = 0;
			for(i = 0; i < block_n; i++){
				if(cache[i]->valid == 1){
					if(cache[i]->tag == tag){
						hits++;
						if(inst == 'W'){
							writes++;
						}
						found = 1;						
						break;
					}
				}
			}
			if(found == 0){
				misses++;
				reads++;
				if(inst == 'W'){
					writes++;
				}
				for(i = 0; i < block_n; i++){
					if(cache[i]->valid == 0){
						cache[i] = create_block(tag, 0, timestamp++, address, ip);
						found = 1;
						break;
					}
				}
			}
			/* cache is full */
			if(found == 0){
				int min_stamp = cache[0]->timestamp;
				int min_index = 0;
				for(i = 0; i < block_n; i++){
					if(cache[i]->timestamp < min_stamp){
						min_stamp = cache[i]->timestamp;
						min_index = i;
					}
				}

				cache[min_index] = create_block(tag, 0, timestamp++, address, ip);
			}
		}
	}

	return;
	
}

void n_assoc(int set_n, FILE *file, int block_size, int block_n){
	struct Block*** cache = (struct Block***)malloc(sizeof(struct Block**)*set_n);
	int i, j;
	for(i = 0; i < set_n; i++){
		cache[i] = (struct Block**)malloc(sizeof(struct Block*)*block_n);
		for(j = 0; j < block_n; j++){
			cache[i][j] = init_block();
		}
	}
	
	char inst;
	char str[1000], str2[1000];
	uint32_t ip, address, tag;
	int timestamp = -1, set;

	while(fscanf(file, "%s %c %s", str, &inst, str2) == 3){
		if(strcmp(str, "#eof") != 0){
			ip = strtoul(str, NULL, 16);
			address = strtoul(str2, NULL, 16);
			int cal1 = ceil(log(block_size)/log(2));	
			uint32_t tmp = address >> cal1;
			int cal2 = ceil(log(set_n)/log(2));
			unsigned mask = ((1 << cal2) - 1); 
			set = tmp & mask;
			tag = tmp >> cal2;

			
			/* empty set */
			if(cache[set][0]->valid == 0){
				cache[set][0] = create_block(tag, set, timestamp++, address, ip);
				misses++;
				reads++;
				if(inst == 'W'){
					writes++;
				}
			}else{
				/* partially filled set */
				int found = 0;
				for(j = 0; j < block_n; j++){
					if(cache[set][j]->tag == tag){
						hits++;
						if(inst == 'W'){
							writes++;
						}
						found = 1;
						break;
					}
				}

				if(found == 0){
					misses++;
					reads++;
					if(inst == 'W'){
						writes++;
					}
					for(j = 0; j < block_n; j++){
						if(cache[set][j]->valid == 0){
							cache[set][j] = create_block(tag, set, timestamp++, address, ip);
							found = 1;
							break;
						}
					}
					/* full set */
					if(found == 0){
						int min_stamp = cache[set][0]->timestamp;
						int min_index = 0;
						for(j = 0; j < block_n; j++){
							if(cache[set][j]->timestamp < min_stamp){
								min_stamp = cache[set][j]->timestamp;
								min_index = j;
							}
						}
						cache[set][min_index] = create_block(tag, set, timestamp++, address, ip);
					}
				}
			}
		}
	}
	return;	
}


int main(int argc, char *argv[]){
	
	int cache_size;
	int block_size;
	char *assoc;
	int set_n;

	if(argc != 5){
		return 0;
	}
	
	cache_size = atoi(argv[1]);
	assoc = argv[2];
	block_size = atoi(argv[3]);
	FILE *file;
	file = fopen(argv[4], "r");
	if(file == NULL){
		printf("error\n");
		return 0;
	}
	
	if(strcmp(assoc, "direct") == 0){
		set_n = cache_size/(block_size);
		direct(set_n, file, block_size);
	}else if(strcmp(assoc, "assoc") == 0){
		set_n = cache_size/(block_size);
		full_assoc(set_n, file, block_size);
	}else{
		assoc = strtok(assoc, "assoc:");
		int n = atoi(assoc);
		set_n = cache_size/(n * block_size);
		n_assoc(set_n, file, block_size, n);
	}

	printf("Memory reads: %d\n", reads);
	printf("Memory writes: %d\n", writes);
	printf("Cache hits: %d\n", hits);
	printf("Cache misses: %d\n", misses);

	return 0;
}	
