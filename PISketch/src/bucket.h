#ifndef _BUCKET_H_
#define _BUCKET_H_

#include "./flow.h"

struct Bucket
{
	FIVE_TUPLE five_tuple[COUNTER_PER_BUCKET];
	uint32_t frequency[COUNTER_PER_BUCKET];
	uint32_t window[COUNTER_PER_BUCKET];
	uint32_t last_win[COUNTER_PER_BUCKET];
};

#endif