#ifndef _DETECTOR_H_
#define _DETECTOR_H_

#include "./src/param.h"
#include "./src/bucket.h"
#include "./src/flow.h"
#include "./Rank.h"
#include "./BOBHash32.h"

using namespace std;

#define KEEP_WIN 0
#define OVERFLOW_PROTECT

class Detector
{
public:
	Bucket buckets[BUCKET_NUM];
	uint32_t bucket_num = BUCKET_NUM;
	uint32_t counter_per_bucket = COUNTER_PER_BUCKET;
  uint32_t overflow_flag = 0;
	BOBHash32 bobhash;

	Detector(){ clear(); }
	~Detector(){ clear(); }
	void clear(){memset(buckets, 0, sizeof(buckets));}

  	Detector(int bucket_num, int counter_per_bucket)
  	{
		  memset(buckets, 0, sizeof(buckets));
		  this->bucket_num = bucket_num;
  		this->counter_per_bucket = counter_per_bucket;
      overflow_flag = 0;
		  bobhash.initialize(1001);
  	}

  	void insert(FIVE_TUPLE five_tuple, uint32_t cur_win)
  	{ 
		uint32_t f = (uint8_t)1;
		// Map a key to a fingerprint

  		int pos = bobhash.run(five_tuple.key, 8) % bucket_num;
  		int min_index = -1;
  		double max_kp = -1;

  		// Search the key in the bucket
		for (int i = 0; i < counter_per_bucket; i++)
  		{
  			if (IsFiveTuple0(buckets[pos].five_tuple[i]))
  			{
  				buckets[pos].five_tuple[i] = five_tuple;
  				buckets[pos].window[i] = f;
  				buckets[pos].frequency[i] = f;
  				buckets[pos].last_win[i] = cur_win;
  				return;
  			}
  			if (five_tuple == buckets[pos].five_tuple[i])
  			{
  				if(buckets[pos].last_win[i] != cur_win)
				{
					buckets[pos].last_win[i] = cur_win;
					buckets[pos].window[i] += f;		
				}
  				buckets[pos].frequency[i] += f;
  				return;
  			}
  			double tmp_kp = 1 / PI_Probability((double)buckets[pos].frequency[i], (double)buckets[pos].window[i]);
  			if (tmp_kp > max_kp)
  			{
  				min_index = i;
  				max_kp = tmp_kp;
  			}
 	 	}

		// Substitute
		//
		if ( (double)(rand() % 100 + 1) / 100.0 <= max_kp)
		{
			buckets[pos].five_tuple[min_index] = five_tuple;
			
			if (KEEP_WIN == 1)
			{
				buckets[pos].frequency[min_index] -= f;
				if (buckets[pos].frequency[min_index] == 0)
					buckets[pos].frequency[min_index] = f;
			}
			else
      {
				//buckets[pos].window[min_index] -= f;
        //if (buckets[pos].window[min_index] == 0)
        buckets[pos].window[min_index] += f;
#ifdef OVERFLOW_PROTECT
        if (PI_Probability(buckets[pos].frequency[min_index], buckets[pos].window[min_index] - 1) < 100.0 &&
          PI_Probability(buckets[pos].frequency[min_index], buckets[pos].window[min_index]) > 100.0)
        {
            ++overflow_flag;
            if (overflow_flag > bucket_num * counter_per_bucket * 9/10)
            {
              overflow_flag = 0;
              WIN_THRE_DETECT = (WIN_THRE_DETECT + NUM_OF_WIN) / 2;
              SP_THRE_DETECT = (double)NUM_OF_WIN / (double)WIN_THRE_DETECT;
            }
        }
#endif
      }

			buckets[pos].last_win[min_index] = cur_win;
		}
		return;
  	}
 
  	void identify(vector<FIVE_TUPLE> &attackers, map<FIVE_TUPLE, Flow> flow_map, double &AAE, double &ARE)
  	{
  		printf("M:%d*%d\nTw/Ts:%d/%lf\n", bucket_num, counter_per_bucket, WIN_THRE, SP_THRE);
  		set<Flow, CMP_FILTER> flow_set;
  		for (int i = 0; i < bucket_num; ++i)
			  for (int j = 0; j < counter_per_bucket; ++j)
  				flow_set.insert(Flow(buckets[i].five_tuple[j], buckets[i].window[j], buckets[i].frequency[j]));


  		Detect_rank(flow_set, attackers, flow_map, AAE, ARE);		
  	}

  	
  	double PI_Probability(double f, double w)
  	{
      double wt = (double)WIN_THRE_DETECT;
      double st = SP_THRE_DETECT;
      return w*w/f*st/wt*100;
  	}

};
#endif