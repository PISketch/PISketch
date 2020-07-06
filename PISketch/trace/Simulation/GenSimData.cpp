#include <iostream>
#include <random>
#include <set>
#include <stdlib.h>
#include <time.h>
#include <typeinfo>
#include "Parse.h"
#include "./genzipf.h"


#define NUM_OF_WIN 1000
#define PKTS_PER_WINDOW 10000

#define NUM_OF_MAL_FLOWS 0
#define NUM_OF_FLOWS 120000

#define NUM_OF_WIN_PER_MAL 600
#define MAX_MAL_PKTS_PER_WIN 10

#define MAX_PKTS_PER_FLOW 1000
using namespace std;

char sim_data_dir = "./sim/0.dat";


int main(int argc, char *argv[]){
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


    // Generate the number of packets for each of the flows
	int frequency[NUM_OF_FLOWS];
	bool exist[NUM_OF_MAL_FLOWS];

	memset(frequency, 0, sizeof(frequency));

  	double alpha = 0.5;                 // Alpha parameter
  	int n = MAX_PKTS_PER_FLOW;

	for (int i = 0; i < NUM_OF_FLOWS; ++i)
		frequency[i] = zipf(alpha, n);

	// Output to files
	int file_cnt = 0;

	FILE *fsrc = fopen(sim_data_dir, "wb");
	while(file_cnt < NUM_OF_WIN)
	{
		int pkt_cnt = 0;

		//初始化攻击流这个窗口内的数据包数量
		for (int j = 0; j < NUM_OF_MAL_FLOWS; ++j)
			frequency[j] = (rand() % MAX_MAL_PKTS_PER_WIN) + 1;

		// 初始化攻击流是否在这个窗口内出现
		for (int i = 0; i < NUM_OF_MAL_FLOWS; ++i)
		{
			double occurrence = (double)rand() / (double)RAND_MAX;
			if (occurrence <= (double)NUM_OF_WIN_PER_MAL / (double)NUM_OF_WIN)
				exist[i] = true;
		}

		// 攻击流按照概率在该窗口内出现 2/2
		for (int i = 0; i < NUM_OF_MAL_FLOWS; ++i)
		{
			if (exist[i] == true && frequency[i] > 0)
			{
				*tmp = flowIDs[i];
				while(frequency[i] > 0)
				{
					fwrite(tmp, 1, 13, fsrc);
					--frequency[i];
					++pkt_cnt;
				}
				exist[i] = false;
			}		
		}

		for (int h = pkt_cnt; h < PKTS_PER_WINDOW; ++h)
		{
			int flow = rand() % NUM_OF_FLOWS;
			if (frequency[flow] <= 0)
			{
				--h;
				continue;
			}
			else
			{
		 		*tmp =flowIDs[flow];
				fwrite(tmp, 1, 13, fsrc);
		 		frequency[flow]--;
		 	}
		}		
		
		file_cnt++;
		printf("Generating... WINDOW: %d.dat\n",file_cnt);
		printf("\033[1A");
       	printf("\033[K");
	}
	fclose(fsrc);
	return 0;


}
