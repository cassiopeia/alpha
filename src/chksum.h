#ifndef MPING_CHKSUM_H
#define MPING_CHKSUM_H

#include "common.h"

u16 chksum(u16 *p, u32 len)
{
  u32 sum = 0;
  s32 nwords = len >> 1;

  while (nwords-- != 0)
    sum += *p++;

  if (len & 1)
  {
    union
    {
      u16 w;
      u8 c[2];
    } u;

    u.c[0] = *(u8 *)p;
    u.c[1] = 0;
    sum += u.w;
  }

  // end-around-carry
  sum = (sum >> 16) + (sum & 0xffff);
  sum += (sum >> 16);
  return (~sum);
}

#endif // MPING_CHKSUM_H
