/* Student ID:	2019026599
 * Name:		Yang JongMun
 */
#include "cachelab.h"
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>

// UTILITY

/* Cache struct for cache read.
 * 
 * s = set index
 * b = block offset
 * 
 * S = 2^s sets -> number of rows
 * E = 2^e lines per set -> number of columns
 * B = 2^b bytes per cache block(the data)
 */
typedef struct init_t {
	size_t s;
	size_t E;
	size_t b;

	size_t S;
	size_t B;
} init_t;

typedef struct line_t {
	char valid;
	size_t tag;
	size_t lru;
} line_t;

typedef struct set_t {
	line_t* lines;
} set_t;

typedef struct cache_t {
	set_t* sets;
} cache_t;

// GLOBAL
init_t init;
cache_t cache;
size_t lruCnt = 1;
// Simulation results
size_t hits = 0;
size_t misses = 0;
size_t evictions = 0;


// FUNCTION
// Print usage
void usage(char** argv) {
    printf("Usage: %s [-hv] -s <num> -E <num> -b <num> -t <file>\n", argv[0]);
    printf("Options:\n");
    printf("  -h         Print this help message.\n");
    printf("  -s <num>   Number of set index bits.\n");
    printf("  -E <num>   Number of lines per set.\n");
    printf("  -b <num>   Number of block offset bits.\n");
    printf("  -t <file>  Trace file.\n\n");
    printf("Examples:\n");
    printf("  linux>  %s -s 4 -E 1 -b 4 -t traces/yi.trace\n", argv[0]);
    printf("  linux>  %s -v -s 8 -E 2 -b 4 -t traces/yi.trace\n", argv[0]);
    exit(0);
}

// Access cache
void find(size_t address) {
	// Initialize variable for find cache data.
	size_t lru = 0xffffffff;
	int lruLine = 0;

	size_t setIndex = (0x7fffffff >> (31 - init.s)) & (address >> init.b);
	size_t tag = 0xffffffff & (address >> (init.s + init.b));

	set_t* set = &cache.sets[setIndex];
	line_t* line = 0;

	/* Check the cache.
	 * If find data is existed in cache, +1 hits(global)
	 * if there is no data, +1 misses.
	 */
	for (int i = 0; i < init.E; i++) {
		line = &set->lines[i];
        if (line->valid) {
            if (line->tag == tag) {
                line->lru = lruCnt++;
                hits++;
	            return;
            }
        }
    }
	misses++;
	
	/* If LRU is larger than line's LRU
	 * change LRU line value to index
	 * and LRU value to line's LRU
	 */
    for (int i = 0; i < init.E; ++i) {
		line = &set->lines[i];
        if (lru > line->lru) {
            lruLine = i;
            lru = line->lru;
        }
    }

	/* If the line is empty, +1 eviction
	 */
	line = &set->lines[lruLine];
    if (line->valid) {
        evictions++;
	}

	/* Change the line's value to eviction value
	 */
    line->valid = 1;
    line->tag = tag;
    line->lru = lruCnt++;
}

int main(int argc, char** argv)
{
	char c;
	FILE *file = 0;
	while ((c = getopt(argc, argv, "s:E:b:t:h")) != -1) {
		switch(c) {
		case 's':
			init.s = atoi(optarg);
			init.S = 2 << init.s;
			break;
		case 'E':
			init.E = atoi(optarg);
			break;
		case 'b':
			init.b = atoi(optarg);
			init.B = 2 << init.b;
			break;
		case 't': // Input filename
        	if (!(file = fopen(optarg, "r"))) {
				return 1;
			}
        	break;
		case 'h':
			usage(argv);
			exit(0);
		default:
			usage(argv);
			exit(1);
		}
	}

	// Check parameters are null
	if (init.s == 0 || init.b == 0 || init.E == 0 || file == 0) {
		printf("%s: Missing required command line argument\n", argv[0]);
		usage(argv);
		exit(1);
	}

	// Initialize cache
	cache.sets = (set_t*)malloc(sizeof(set_t) * init.S);
	for (int i = 0; i < init.S; i++) {
		cache.sets[i].lines = calloc(sizeof(line_t), init.E);
	}
	
	// variable for reading file
	char command;
    size_t address;

	// Read file and check a instructor to access cache struct
    while (fscanf(file, " %c %lx", &command, &address) != EOF) {
		switch(command) {
		case 'L':
				find(address);
				break;
            case 'S':
				find(address);
				break;
			case 'M':
				// Two access(Read and Write)
				find(address);
				find(address);
				break;
            default:
				break;
        }
    }

	// Print result
    printSummary(hits, misses, evictions);
    
	// Close file
	fclose(file);
	// Free cache
	for (int i = 0; i < init.E; i++) {
		free(cache.sets[i].lines);
	}
	free(cache.sets);

	return 0;
}