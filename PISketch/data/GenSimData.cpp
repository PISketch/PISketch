#include <iostream>
#include <random>
#include <set>
#include <stdlib.h>
#include <time.h>
#include <typeinfo>
#include <memory.h>
#include "./genzipf.h"

#define NUM_OF_WIN 1000
#define NUM_OF_WIN_PER_MAL 600
#define MAX_MAL_PKTS_PER_WIN 2
using namespace std;

int PKTS_PER_WINDOW = 10000;

int NUM_OF_MAL_FLOWS = 200;
int NUM_OF_FLOWS = 400000;
double ALPHA = 1.5;

char sim_data_dir[100];


int main(int argc, char *argv[]){

	if (argc != 6)
	{
		printf("Wrong Input!\n");
		return 0;
	}


	PKTS_PER_WINDOW = atoi(argv[3]);

	NUM_OF_MAL_FLOWS = atoi(argv[1]);
	NUM_OF_FLOWS = atoi(argv[2]);
	ALPHA = ((double)atoi(argv[4])) / ((double)atoi(argv[5]));

	// Check parameters 
	if(NUM_OF_WIN_PER_MAL > NUM_OF_WIN)
		return 0;
	// Store flows
	std::set<uint64_t> flowIDs_set;
	std::vector<uint64_t> flowIDs;
	
	uint8_t five_tuple[13];
	srand((unsigned)time(NULL));  
	
	// Generate random flow IDs and store them with vector
	while(flowIDs_set.size() < NUM_OF_FLOWS) {

		for (int i = 0; i < 13; ++i)
			five_tuple[i] = rand() % 256;
		flowIDs_set.insert(*((uint64_t *)five_tuple));
	}

	for(set<uint64_t>::iterator it=flowIDs_set.begin() ;it!= flowIDs_set.end(); it++)
        flowIDs.push_back(*it);

  	rand_val(530);

	// Output to files
	int file_cnt = 0;
	sprintf(sim_data_dir, "./sim/%d_%d_%d_%lf.dat", NUM_OF_MAL_FLOWS, NUM_OF_FLOWS, PKTS_PER_WINDOW, ALPHA);
	FILE *fsrc = fopen(sim_data_dir, "wb");
	printf("Generating... FILE: %s\n", sim_data_dir);

	while(file_cnt < NUM_OF_WIN)
	{
		printf("Generating... WINDOW: %d\n",file_cnt);

		int pkt_cnt = 0;

		// 攻击流按照概率在该窗口内出现 2/2
		for (int i = 0; i < NUM_OF_MAL_FLOWS; ++i)
		{
			double occurrence = (double)rand() / (double)RAND_MAX;
			if (occurrence <= (double)NUM_OF_WIN_PER_MAL / (double)NUM_OF_WIN)
			{
				uint64_t tmp = flowIDs[i];
				int frequency = (rand() % MAX_MAL_PKTS_PER_WIN) + 1;

				while(frequency--)
				{
					fwrite(&tmp, 1, 8, fsrc);
					fwrite(&tmp, 1, 8, fsrc);
					++pkt_cnt;

				}
			}		
		}

		for (int h = pkt_cnt; h < PKTS_PER_WINDOW; ++h)
		{
			int flow = NUM_OF_MAL_FLOWS + zipf(ALPHA, NUM_OF_FLOWS - NUM_OF_MAL_FLOWS);

			uint64_t tmp = flowIDs[flow];
			fwrite(&tmp, 1, 8, fsrc);
			fwrite(&tmp, 1, 8, fsrc);
		}		
		
		file_cnt++;
		printf("\033[1A");
       	printf("\033[K");
	}
	fclose(fsrc);
	printf("\033[1A");
    printf("\033[K");
	printf("FILE: %s Complete.\n", sim_data_dir);

	return 0;


}
