#ifndef _RANK_H_
#define _RANK_H_

#include "./src/param.h"
#include "./src/bucket.h"
#include "./src/flow.h"

using namespace std;

typedef struct
{
  bool operator ()(const Flow flow1, const Flow flow2)
  {
      return flow1.five_tuple < flow2.five_tuple;
  }
}KEY;


void Detect_rank(set<Flow, CMP_FILTER> &flow_set, vector<FIVE_TUPLE> &attackers, map<FIVE_TUPLE, Flow> &flow_map, double &AAE, double &ARE)
{
	set<Flow, CMP_FILTER>::iterator iter = flow_set.begin();
  map<FIVE_TUPLE, Flow>::iterator iter_actual;
  AAE = 0.0;
  ARE = 0.0;
	while (iter != flow_set.end())
	{

    if (iter->window < WIN_THRE || iter->frequency > (uint32_t)((double)iter->window * SP_THRE))
    {
      ++iter;
      continue;
    }

		attackers.push_back(iter->five_tuple);
    iter_actual = flow_map.find(iter->five_tuple);
    if (iter_actual != flow_map.end())
    {
      AAE += fabs((double)(iter_actual->second.window * iter_actual->second.window) / (double)iter_actual->second.frequency - (double)(iter->window * iter->window) / (double)iter->frequency);
      ARE += fabs((double)(iter_actual->second.window * iter_actual->second.window) / (double)iter_actual->second.frequency - (double)(iter->window * iter->window) / (double)iter->frequency) / ((double)(iter_actual->second.window * iter_actual->second.window) / (double)iter_actual->second.frequency);
    }
    else
    {
      printf("Detected Element Invalid!");
      exit(0);
    }
 					
 		++iter;
  }
  AAE /= (double)attackers.size();
  ARE /= (double)attackers.size();
}

void Total_rank(set<Flow, CMP_TOT> &flow_set, char *att_list)
{
  set<Flow, CMP_TOT>::iterator iter = flow_set.begin();

  
  FILE *flist = fopen(att_list,"wb");

  while (iter != flow_set.end())
  {

    if (iter->window < WIN_THRE || iter->frequency > (uint32_t)((double)iter->window * SP_THRE))
    {
      ++iter;
      continue;
    }
    fwrite(iter->five_tuple.key, 1, 13, flist);

    
    ++iter;
  }
  
  fclose(flist);
}
#endif