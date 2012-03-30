#ifndef MPING_CHKSUM_H
#define MPING_CHKSUM_H

#include "common.h"

uint16 chksum(uint16 *p, uint32 len)
{
  uint32 sum = 0;
  sint32 nwords = len >> 1;

  while (nwords-- != 0)
    sum += *p++;

  if (len & 1)
  {
    union
    {
      uint16 w;
      uint8 c[2];
    } u;

    u.c[0] = *(u_char *)p;
    u.c[1] = 0;
    sum += u.w;
  }

  // end-around-carry
  sum = (sum >> 16) + (sum & 0xffff);
  sum += (sum >> 16);
  return (~sum);
}

#endif // MPING_CHKSUM_H
