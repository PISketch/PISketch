#ifndef _DETECTOR_H_
#define _DETECTOR_H_

#include "./src/param.h"
#include "./src/bucket.h"
#include "./src/flow.h"
#include "./Rank.h"
#include "./BOBHash32.h"

#define KEEP_WIN 0

using namespace std;


class Detector
{
public:
	Bucket buckets[BUCKET_NUM];
	uint32_t expected_flow_num = EXPECTED_FLOW_NUM;
	uint32_t bucket_num = BUCKET_NUM, counter_per_bucket = COUNTER_PER_BUCKET;


	Detector(){ clear(); }
	~Detector(){ clear(); }
	void clear(){ memset(buckets, 0, sizeof(buckets));}

  	Detector(int bucket_num, int counter_per_bucket, int exp_flow_num)
  	{
		/*
		Bucket bk;
		bk.initBucket(counter_per_bucket);
		buckets = new Bucket[bucket_num];
		for (int i = 0; i < bucket_num; ++i)
		{
			buckets[i] = bk;
		}
		*/
		memset(buckets, 0, sizeof(buckets));
		this->bucket_num = bucket_num;
  		this->counter_per_bucket = counter_per_bucket;
		expected_flow_num = exp_flow_num;
  	}

  	void insert(uint32_t *key, uint32_t cur_win, uint32_t rest_win, double random_num, double pkts_per_win)
  	{
  		uint32_t fp = 0;
		uint32_t f = (uint8_t)1;
		// Map a key to a fingerprint

  		int pos = CalculateFP(key, fp);
  		int matched_index = -1;

  		// Search the key in the bucket
		for (int i = 0; i < counter_per_bucket; i++)
  		{
  			if(fp == buckets[pos].key[i] && fp != 0)
  			{
  				matched_index = i;
  			}
  			if (0 == buckets[pos].key[i] && buckets[pos].frequency[i] != 0 )
  			{
  				matched_index = i;
  			}	
 	 	}

		if (matched_index != -1)
  		{
			buckets[pos].frequency[matched_index] += f;
        	int last_win = buckets[pos].last_win[matched_index];

       		if(last_win != cur_win)
			{
				int wins = buckets[pos].window[matched_index];
				buckets[pos].last_win[matched_index] = cur_win;
				buckets[pos].window[matched_index] += f;		
			}
			buckets[pos].kp[matched_index] = KickProbability(buckets[pos].frequency[matched_index], buckets[pos].window[matched_index], pkts_per_win);
			return;
		}

		int max_counter  = -1;
		double max_value = 0.0;
		double rp = 0.0;

		for (int j = 0; j < counter_per_bucket; j++)
		{
			if (buckets[pos].kp[j] != 0.0)
			{
				rp = (double)1/(buckets[pos].kp[j]);
				//rp = (double)exp(0-buckets[pos].kp[j]);
			}else{
				rp = 0.0;
			}

			if (rp == 0.0)
			{
				max_value = rp;
				max_counter = j;
				break;
			}else if ( max_value < rp ) 
			{
				max_counter  = j;
				max_value = rp;
			}

		}

		// Substitute
		//
		if (max_value == 0.0 || random_num <= max_value )
		{
			buckets[pos].key[max_counter] = fp;
			if (max_value == 0.0)
			{
				buckets[pos].window[max_counter] = f;
				buckets[pos].frequency[max_counter] = f;
			}
			else if (KEEP_WIN == 1)
			{
				buckets[pos].frequency[max_counter] -= f;
				if (buckets[pos].frequency[max_counter] < 0)
				{
					buckets[pos].frequency[max_counter] = 0;
				}
			}
			else
			{
				buckets[pos].window[max_counter] += f;
			}

			buckets[pos].last_win[max_counter] = cur_win;
			buckets[pos].kp[max_counter] = KickProbability(buckets[pos].frequency[max_counter], buckets[pos].window[max_counter], pkts_per_win);
		}
		return;
  	}
  	

  	void query(uint8_t *key, uint32_t & wins, uint32_t & frequency)
  	{

  	}

  	void identify(vector<uint32_t> &attackers)
  	{
  		/*
  		printf("%d:%d:%d:%d\n", bucket_num, counter_per_bucket, window_threshold, frequency_threshold);
  		for (int i = 0; i < bucket_num; ++i)
		{
			for (int j = 0; j < counter_per_bucket; ++j)
  			{  				

  				if(buckets[i].frequency[j] < frequency_threshold && buckets[i].window[j] >= window_threshold)
  				{
					attackers.push_back(buckets[i].key[j]);
				}

						
			}  			
  		}
  		*/
  		printf("%d:%d:%d:%d\n", bucket_num, counter_per_bucket, expected_flow_num);

  		set<Flow, CMP_FILTER> flow_set;
  		for (int i = 0; i < bucket_num; ++i)
			for (int j = 0; j < counter_per_bucket; ++j)
  				flow_set.insert(Flow(buckets[i].key[j], buckets[i].window[j], buckets[i].frequency[j]));

  		Detect_rank(expected_flow_num, flow_set, attackers);
  		/*
  		// Skyline
  		if (!FREQ_PRIOR) // Window first
  		{
  			set<Flow, WIN_FIRST> skyline;
			set<Flow, WIN_FIRST>::iterator iter;
 			
  		
  			for (int i = 0; i < bucket_num; ++i)
				for (int j = 0; j < counter_per_bucket; ++j)
  					skyline.insert(Flow(buckets[i].key[j], buckets[i].window[j], buckets[i].frequency[j]));

	  		int expected_left = expected_flow_num;
	  		int line_freq = -1;

  			FILE *fout = fopen("./skyline.txt","wr");

  			while (expected_left)
  			{
  				line_freq = -1;
  				for (iter = skyline.begin(); iter != skyline.end();)
  				{
  					if (line_freq == -1 || iter->frequency < line_freq)
  					{
  						char buf[100];
  						sprintf(buf, "%d %d %lf\n", iter->window, iter->frequency, (double)(iter->frequency) / (double)(iter->window));
  						fwrite(buf, 1, strlen(buf), fout);

						attackers.push_back(iter->key);
						skyline.erase(iter++);
						line_freq = iter->frequency;
  						


						if(!(--expected_left))
							break;
  					}
  					else
  						++iter;
  				}
  			}
  			fclose(fout);
  		}
  		else // Frequency first
  		{
 			set<Flow, FREQ_FIRST> skyline;
			set<Flow, FREQ_FIRST>::iterator iter;
 			
  		
  			for (int i = 0; i < bucket_num; ++i)
				for (int j = 0; j < counter_per_bucket; ++j)
  					skyline.insert(Flow(buckets[i].key[j], buckets[i].window[j], buckets[i].frequency[j]));

	  		int expected_left = expected_flow_num;
	  		int line_win = -1;

  			FILE *fout = fopen("./skyline.txt","wr");

  			while (expected_left)
  			{
  				line_win = -1;
  				for (iter = skyline.begin(); iter != skyline.end();)
  				{
  					if (line_win == -1 || iter->window > line_win)
  					{
  						char buf[100];
  						sprintf(buf, "%d %d %lf\n", iter->window, iter->frequency, (double)(iter->frequency) / (double)(iter->window));
  						fwrite(buf, 1, strlen(buf), fout);

						attackers.push_back(iter->key);
						skyline.erase(iter++);
						line_win = iter->window;
  						
  						

						if(!(--expected_left))
							break;
  					}
  					else
  						++iter;
  				}
  			}
  			fclose(fout);
  		}
  		*/


  		
  	}

  	
  	double KickProbability(uint32_t frequency, uint32_t window, double pkts_per_win)
  	{
  		//return pkts_per_win * (double)window / (double)frequency;
  		return (double)window * (double)window / (double)frequency;
  		//return 2 * (double)window * log((double)window) - (double)window *log((double)frequency);
  	}


	int CalculateFP(uint32_t *key, uint32_t &fp)
	{
		fp = *((uint32_t*)key);
		return CalculateBucketPos(fp) % bucket_num;
	}

};

/*
int APTDetect::insert(uint8_t *key, uint32_t cur_win, uint32_t rest_wins, double random_num)
{	
	uint32_t fp = 0;
	uint32_t f = 1; 
  	int pos = CalculateFP(key, fp);
  	int matched_index = -1;
  	float replace_probability[COUNTER_PER_BUCKET];


	for (int i = 0; i < COUNTER_PER_BUCKET; i++)
  	{
  		if (fp == buckets[pos].keys[i])
  		{
  			matched_index = i;
  		}	
  	}

	if (matched_index != -1)
  	{
  		int wins = buckets[pos].val_w[matched_index] + 1;

  		if((rest_wins + wins) < window_threshold) 
		{
			return 1;
		}
		buckets[pos].val_f[matched_index] += f;
        
        int last_win = buckets[pos].last_win[matched_index];

        if(last_win != cur_win)
		{
			buckets[pos].val_w[matched_index] += f;
			buckets[pos].last_win[matched_index] = cur_win;		
		}

		buckets[pos].kp[matched_index] = KickProbability(buckets[pos].val_f[matched_index], buckets[pos].val_w[matched_index]);
		
		return 0;
		
	}
	if ((1 + rest_wins) < window_threshold)
	{
		return 1; //
	}

	int max_counter  = 0;
	double max_value = buckets[pos].kp[0];
	double rp = 0;

	for (int j = 0; j < COUNTER_PER_BUCKET; j++)
	{
		rp = buckets[pos].kp[j];

		if (rp == 0)
		{
			max_value = rp;
			max_counter = j;
			break;
		} 
		if ( max_value < rp ) 
		{
			max_counter  = j;
			max_value = rp;
		}
	}


	if (max_value == 0 || random_num <= max_value )
	{
		buckets[pos].keys[max_counter] = fp;
		buckets[pos].val_w[max_counter] = f;
		buckets[pos].val_f[max_counter] = f;
		buckets[pos].last_win[max_counter] = cur_win;
		buckets[pos].kp[max_counter] = KickProbability(f, f);
	}


	return 0;
}

int APTDetect::query(uint8_t *key, uint32_t & wins, uint32_t & frequency)
{
	uint32_t fp;
	int pos = CalculateFP(key, fp);

	for(int i = 0; i < COUNTER_PER_BUCKET; ++i)
		if(buckets[pos].keys[i] == fp)
		{
			wins = buckets[pos].val_w[i];
			frequency = buckets[pos].val_f[i];
		}

	return 0;
}
	
  	
int APTDetect::identify(vector<uint32_t> &attackers)
{
	for (int i = 0; i < BUCKET_NUM; ++i)
	{
		for (int j = 0; j < COUNTER_PER_BUCKET; ++j)
  		{	
  			if(buckets[i].val_f[j] < frequency_threshold && buckets[i].val_w[j] > window_threshold)
  			{
				attackers.push_back(buckets[i].keys[j]);
			}
		}  			
  	}

  	return 0;
}

	



double APTDetect::KickProbability(uint32_t frequency, uint32_t windows) 
{

	
	int digital = 0;
	int fre = frequency;
	while(fre != 0)
	{
		fre = fre/10;
		digital++;
	}
	int flength = digital;
	digital = 0;

	int win = windows;
	while(win != 0)
	{
		win = win/10;
		digital++;
	}
	int wlength = digital;

	int length = flength - wlength + 1;
	double kp = (double)frequency / ((10 ^ length) * windows);
	return kp;
	
	
	//return (double)windows/(frequency*10);
	//return (frequency - windows) * 0.0001;
}
*/
#endif