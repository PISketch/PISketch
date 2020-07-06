#ifndef _FLOW_H_
#define _FLOW_H_

struct FIVE_TUPLE
{
  char key[13];

  FIVE_TUPLE()  {
    memset(key, 0, 13);
  }

  FIVE_TUPLE(char *_key)  {
    memcpy(key, _key, 13);
  }

  FIVE_TUPLE(const FIVE_TUPLE &five_tuple)  {
    memcpy(key, five_tuple.key, 13);
  }

  bool operator==(const FIVE_TUPLE &five_tuple){
    return !memcmp(key, five_tuple.key, 13);
  }

  bool operator<(const FIVE_TUPLE &five_tuple) const {
    return memcmp(key, five_tuple.key, 13) < 0;
  }


};

struct Flow
{
  FIVE_TUPLE five_tuple;
  uint32_t window;
  uint32_t frequency;
  uint32_t last_win;
  Flow(FIVE_TUPLE five_tuple, uint32_t win, uint32_t freq)
  {
    this->five_tuple = five_tuple;
    window = win;
    frequency = freq;
    last_win = 0;
  }
  Flow(FIVE_TUPLE five_tuple, uint32_t win, uint32_t freq, uint32_t last_win)
  {
    this->five_tuple = five_tuple;
    window = win;
    frequency = freq;
    this->last_win = last_win;
  }
};

bool IsFiveTuple0(FIVE_TUPLE five_tuple)
{
  char zero[13];
  memset(zero, 0, 13);
  return !memcmp(five_tuple.key, zero, 13);
}

#endif