#include "common.h"

struct stat
{
  struct addr addr;
  uint16 iden;
  uint16 seqn;
  uint32 sent;
  uint32 recv;
  uint32 errs;
  struct timeval time;
  float rtt_min;
  float rtt_max;
  float rtt_prv;
  float rtt_sum;
  float jtr_min;
  float jtr_max;
  float jtr_sum;
};

extern sint32 libmping_open();
extern sint32 libmping_poll(struct mmsghdr *messages, uint32 offset, uint32 count);
extern sint32 libmping_print();
extern sint32 libmping_close();

sint32 timeval_sub(struct timeval *tv1, struct timeval *tv2, struct timeval *res)
{
  if ((tv1->tv_sec < tv2->tv_sec) || ((tv1->tv_sec == tv2->tv_sec) && (tv1->tv_usec < tv2->tv_usec)))
    return -1;

  res->tv_sec  = tv1->tv_sec  - tv2->tv_sec;
  res->tv_usec = tv1->tv_usec - tv2->tv_usec;

//  assert((res->tv_sec > 0) || ((res->tv_sec == 0) && (res->tv_nsec >= 0)));

  while (res->tv_usec < 0)
  {
    res->tv_usec += 1000000;
    res->tv_sec--;
  }

  return 0;
}

float timeval_msec(struct timeval *val)
{
  return (val->tv_sec * 1000000 + val->tv_usec) / 1000.0;
}
