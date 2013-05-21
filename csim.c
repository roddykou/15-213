/* 
 * Name: Zheng Kou
 * Andrew ID: zhengk
 */

#include "cachelab.h"
#include <getopt.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

//variables to store input arguments
unsigned int s = 0;                 //number of set index bits
unsigned int E = 0;                 //number of lines per set
unsigned int b = 0;                 //number of block bits
FILE * file_pointer = NULL;         //file pointer
char * file_name;                   //file name
    
int hit_count = 0;
int miss_count = 0;
int eviction_count = 0;

unsigned int ** valid, ** tag;      //virtual cache memory

static const char * optString = "s:E:b:t:";      //valid options

void arguments_setting(int , char * []);
void inti_cache();

int main(int argc, char * argv[]) {
    arguments_setting(argc, argv);
    inti_cache();

    while(!feof(file_pointer)) {
        //read the arguments from trace file
        long long address;
  	    unsigned int size;
        char type;    
        fscanf(file_pointer, " %c %llx,%d\n", &type, &address, &size);
        
        //ignore instruction load
        if(type == 'I')
            continue;
        
        //printf("%c %x, %d\n", type, address, size);
        //extract tag bits and set bits from the address
        unsigned int tag_bits, set_bits;
        tag_bits = address >> (b + s);
        set_bits = (address >> b) % (1 << s);
        
        //printf("tag_bits = %x, set_bits = %x\n", tag_bits, set_bits);
        
        unsigned int line_runner;           //get ready to loop for every line
        int miss_or_hit = 0;
        
        /*
         * search in every line in the specific set
         * in order to check if there is a line that 
         * is both valid and matches the tag bits of the "address"
         * then HITS
         */
        for(line_runner = 0; line_runner < E; line_runner++) {
            
            //if the line is valid and the tag matches, then hit
            if(valid[set_bits][line_runner] != 0) {
                if(tag_bits == tag[set_bits][line_runner]) {
                    //set valid to be one, increment after the for loop
                    valid[set_bits][line_runner] = 0;
                    hit_count++;
                    miss_or_hit = 1;
                    printf("%c %llx, hit\n", type, address);
               }
                valid[set_bits][line_runner]++;
            }
            
        }
        
        
        /*
         * if it doesn't hit, then it is a miss.
         * And check whether or not it occur an eviction
         */
        
        if(!miss_or_hit) {
            
            miss_count++;
            printf("%c %llx, miss\n", type, address);
            
            int evict_or_not = 1;
            
            for (line_runner = 0; line_runner < E; line_runner++) {
                //if an empty line exist, copy to this empty line
                //this doesn't occur an eviction
                if(valid[set_bits][line_runner] == 0) {
                    tag[set_bits][line_runner] = tag_bits;
                    valid[set_bits][line_runner] = 1;
                    evict_or_not = 0;
                    break;
                }
            }
            
            //else copy to the line that has the biggest valid value
            //and this is an eviction
            int max_valid_line = 0;
            if(evict_or_not == 1) {
                for(line_runner = 0; line_runner < E; line_runner++) {
                    if(valid[set_bits][line_runner] > valid[set_bits][max_valid_line])
                        max_valid_line = line_runner;
                }
                //set tag and valid to the new line
                tag[set_bits][max_valid_line] = tag_bits;
                valid[set_bits][max_valid_line] = 1;
                eviction_count++;
                printf("evict\n");
            }
        }
        
        //if type is M, store must be a hit
        if(type == 'M') {
            hit_count++;
            printf("hit\n");
        }
    }
    
	printSummary(hit_count, miss_count, eviction_count);
	return 0;
}


/*
 * take command line input arguments
 * set number of set index bits,
 * number of lines per set,
 * number of block bits and 
 * file path.
 */
void arguments_setting(int argc, char * argv[]) {
    int opt = 0;
    opt = getopt( argc, argv, optString );
	while( opt != -1 ) {
		switch( opt )
        {
			case 's':
				s = atoi(optarg);
				break;
				
			case 'E':
				E = atoi(optarg);
				break;
				
			case 'b':
				b = atoi(optarg);
				break;
				
			case 't':
				file_name = optarg;
				break;
				
			default:
                printf("%c is not a valid option\n",opt);
				break;
		}
        opt = getopt( argc, argv, optString );
	}
}

void inti_cache() {
    file_pointer = fopen(file_name, "r");
    
    //set the number of sets
    unsigned int S = 1 << s;
    
    //dynamically allocates two 2D arrays, tag and valid
    tag = (unsigned int **)malloc(sizeof(unsigned int *) * S);
    valid = (unsigned int **)malloc(sizeof(unsigned int *) * S);
    for (unsigned int i = 0; i < S; i++) {
        tag[i] = (unsigned int *)malloc(sizeof(unsigned int) * E);
        valid[i] = (unsigned int *)malloc(sizeof(unsigned int) * E);
        for (unsigned int j = 0; j < E; j++) {
            tag[i][j] = 0;
            valid[i][j] = 0;
        }
    }
}









