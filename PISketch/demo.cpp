#include <stdio.h>
#include <algorithm>
#include "Detector.h"
#include "trace/blender.h"

using namespace std;

//#define PRINT_RES
#define DATAFILE_NUM 1

typedef vector<FIVE_TUPLE> TRACE;
TRACE traces[1000];
char res_dir[100] = "./result/res_chwinSize_real_AE-wwf.txt";
char test_traffic_dir[100] = "../APT1.0/data/mix/PlugX.dat";
char att_list[100] = "./data/real/list.dat";
char src_att_list[100] = "../APT1.0/data/mix/attackList_backup.dat";

void ReadInTraces(const char *test_traffic_dir, uint32_t win_size)
{
	printf("%s\n", test_traffic_dir);
	char datafileName[100];
	uint32_t datafileNo = 0;
	sprintf(datafileName, "%s", test_traffic_dir, datafileNo);

	FIVE_TUPLE tmp_five_tuple;
	uint32_t sec;
	uint32_t usec;

	FILE *fin = fopen(datafileName, "rb");
	for(int datafileCnt = 0; datafileCnt < NUM_OF_WIN; ++datafileCnt)
	{
		traces[datafileCnt].clear();
		for (int tmp_pkt = 0; tmp_pkt < win_size; ++tmp_pkt)
		{
			if(!(fread(&sec, 1, 4, fin) == 4 && fread(&usec, 1, 4, fin) == 4 && fread(tmp_five_tuple.key, 1, 8, fin) == 8))
			{
				fclose(fin);
				if (++datafileNo >= DATAFILE_NUM)
				{
					printf("Packets Not Enough From Window %d\n", datafileCnt);
					exit(0);
				}
				sprintf(datafileName, "%s%d.dat", test_traffic_dir, datafileNo);
				fin = fopen(datafileName, "rb");
				--tmp_pkt;
				continue;			
			}
			traces[datafileCnt].push_back(tmp_five_tuple);
		}
		
		printf("Reading in %s, %ld packets\n", datafileName, traces[datafileCnt].size());
		printf("\033[1A");
        printf("\033[K");
	}
	fclose(fin);
	printf("Reading In Traces (%d files): OK\n", NUM_OF_WIN);
	
}

void TransformAttackList(const char *srcAttackList, const char *dstAttackList)
{
	FIVE_TUPLE tmp_five_tuple;
	uint32_t sec;
	uint32_t usec;

	FILE *fin = fopen(srcAttackList, "rb");
	FILE *flist = fopen(dstAttackList, "wb");
	
	while (fread(&sec, 1, 4, fin) == 4 && fread(&usec, 1, 4, fin) == 4 && fread(tmp_five_tuple.key, 1, 8, fin) == 8)
		fwrite(tmp_five_tuple.key, 1, 13, flist);
		

	fclose(fin);
	fclose(flist);
}

int main(int argc, char const *argv[])
{
	//./a.out bucket_num counter_per_bucket window_threshold frequency_threshold att_traffic
	if (argc != 7)
	{
		printf("./a.out bucket_num counter_per_bucket win_thre sp_thre win_size\n");
		return 0;
	}

	uint32_t win_size = (uint64_t)atoi(argv[6]);
	SP_THRE = (double)atoi(argv[4]) / (double)atoi(argv[5]);
	WIN_THRE = atoi(argv[3]);
	int counter_per_bucket = atoi(argv[2]);
	int bucket_num = atoi(argv[1]);
	SP_THRE_DETECT = SP_THRE;
	WIN_THRE_DETECT = WIN_THRE;
	NUM_OF_WIN = TOT_PKTS / win_size;

	// Read traces
	//ReadInTraces(test_traffic_dir);
	//sprintf(test_traffic_dir,"./trace/Simulation/sim");
	printf("%s\n",test_traffic_dir);
	ReadInTraces(test_traffic_dir, win_size);

	// Insert traces into Detector
	double random_num;
	int packet_cnt = 0;
	int cur_win = 1;
	srand( (unsigned)time( NULL ) );
	Detector *detector = NULL;
	double insert_time_us = 0.0;
	timeval tmp_start, tmp_end;
	
	//------------
	detector = new Detector(bucket_num, counter_per_bucket);
	//detector = new Detector( BUCKET_NUM, COUNTER_PER_BUCKET, window_threshold, frequency_threshold);
	
	map<FIVE_TUPLE, Flow> flows;
	map<FIVE_TUPLE, Flow>::iterator iter;

	for(int datafileCnt = 0; datafileCnt < NUM_OF_WIN; ++datafileCnt)
	{
		packet_cnt = (int)traces[datafileCnt].size();
		cur_win = datafileCnt+1;
		
		for(int i = 0; i < packet_cnt; ++i)
		{
			gettimeofday(&tmp_start, NULL);
			detector->insert(traces[datafileCnt][i], cur_win);
			gettimeofday(&tmp_end, NULL);
			insert_time_us += ((double)tmp_end.tv_sec * 1000000 + (double)tmp_end.tv_usec) - ((double)tmp_start.tv_sec * 1000000 + (double)tmp_start.tv_usec);

			 // Sort flows
			 iter = flows.find(traces[datafileCnt][i]);
			 if (iter != flows.end())
			 {
			 	++iter->second.frequency;
			 	if(cur_win != iter->second.last_win)
			 	{
			 		++iter->second.window;
			 		iter->second.last_win = cur_win;
			 	}

			 }
			 else
			 	flows.insert(pair<FIVE_TUPLE, Flow>(traces[datafileCnt][i], Flow(traces[datafileCnt][i], 1, 1, cur_win)));


		}
		
		printf("Inserting: %d in %d\n", cur_win, NUM_OF_WIN);
		printf("\033[1A");
        printf("\033[K");
	}
	printf("Insert completed.\n");

	// Print flow rank
	
	set<Flow, CMP_TOT> flow_set;
	for (iter = flows.begin(); iter != flows.end(); ++iter)
		flow_set.insert(iter->second);
	//Total_rank(flow_set, att_list);
	

	TransformAttackList(src_att_list, att_list);

	// Detect APT
	int hit = 0;
	vector<FIVE_TUPLE> attackers;
	vector<FIVE_TUPLE> real_attackers;
	double AAE, ARE;


    printf("\nNp: %d\n", win_size);

	detector->identify(attackers, flows, AAE, ARE);
	printf("Total: %d\n", flow_set.size());
	printf("Detected: %d\n", (int)attackers.size());
	
	// Count actual hit

	//------------
	//sprintf(att_list,"./trace/Simulation/sim/list.dat", test_traffic_dir);
	FILE *ra = fopen(att_list, "rb");

	char tmp_five_tuple[13];
	
	while (fread(&tmp_five_tuple, 1, 13, ra) == 13)
	{
		real_attackers.push_back(FIVE_TUPLE(tmp_five_tuple));
	}
	
	printf("Actual: %d\n", (int) real_attackers.size());
	fclose(ra);


	
	for (int j = 0; j < (int)attackers.size(); j++)
	{
		std::vector<FIVE_TUPLE>::iterator v;
		v = find(real_attackers.begin(), real_attackers.end(), attackers[j]);
		if (v != real_attackers.end())
		{
			hit++;
		}
	}
	
	
	printf("HIT: %d\n", hit);
	float precision, recall;
	int att_num_detect = (int) attackers.size();
	int att_num_real = (int) real_attackers.size();
	precision = (float) hit / att_num_detect;
	recall = (float) hit / att_num_real;
	printf("Precision: %f\n", precision);
	printf("Recall: %f\n", recall);
	printf("F1-score: %f\n", 2*precision*recall/(precision+recall));
	printf("AAE: %lf\n", AAE);
	printf("ARE: %lf\n", ARE);


	double Mips = (double)(NUM_OF_WIN * win_size) / insert_time_us;
	printf("Mips: %lf\n\n", Mips);

#ifdef PRINT_RES
	FILE *fout = fopen(res_dir,"a+");
    char buf[1000] = { 0 };
    sprintf(buf, "Data: %s\n", test_traffic_dir); fwrite(buf, 1, strlen(buf), fout);
    sprintf(buf, "Np: %d\n", win_size); fwrite(buf, 1, strlen(buf), fout);
    sprintf(buf, "M: %d (d) * %d (m)\n", bucket_num, counter_per_bucket); fwrite(buf, 1, strlen(buf), fout);
    sprintf(buf, "Tw&Ts: %d & %lf\n", WIN_THRE, SP_THRE); fwrite(buf, 1, strlen(buf), fout);
    sprintf(buf, "Total: %d\n", flow_set.size()); fwrite(buf, 1, strlen(buf), fout);
    sprintf(buf, "Actual: %d\n", real_attackers.size()); fwrite(buf, 1, strlen(buf), fout);
	sprintf(buf, "Detected: %d\n", attackers.size()); fwrite(buf, 1, strlen(buf), fout);
    sprintf(buf, "HIT: %d\n", hit); fwrite(buf, 1, strlen(buf), fout);
    sprintf(buf, "Precision: %f\n", precision); fwrite(buf, 1, strlen(buf), fout);
	sprintf(buf, "Recall: %f\n", recall); fwrite(buf, 1, strlen(buf), fout);
	sprintf(buf, "F1-score: %f\n", 2*precision*recall/(precision+recall)); fwrite(buf, 1, strlen(buf), fout);
	sprintf(buf, "AAE: %lf\n", AAE); fwrite(buf, 1, strlen(buf), fout);
	sprintf(buf, "ARE: %lf\n", ARE); fwrite(buf, 1, strlen(buf), fout);
	sprintf(buf, "Mips: %f\n\n", Mips); fwrite(buf, 1, strlen(buf), fout);

  
	fclose(fout);
#endif

	delete detector;
	
	return 0;
}
