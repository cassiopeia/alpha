#include "common.h"

struct stat
{
  struct addr addr;

  u16 iden;
  u16 seqn;

  u32 sent;
  u32 recv;
  u32 errs;

  struct timespec time;

  f32 rtt_min;
  f32 rtt_max;
  f32 rtt_prv;
  f32 rtt_sum;

  f32 jtr_min;
  f32 jtr_max;
  f32 jtr_sum;
};

extern s32 libmping_open ();
extern s32 libmping_poll (struct mmsghdr *messages, u32 offset, u32 count);
extern s32 libmping_print ();
extern s32 libmping_close ();

f64 timespec_diff (struct timespec *start, struct timespec *end)
{
  return (f64) (end->tv_sec - start->tv_sec) * 1.0e9 + (f64) (end->tv_nsec - start->tv_nsec);
}
