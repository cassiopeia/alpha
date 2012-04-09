#ifndef MPING_COMMON_H
#define MPING_COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <poll.h>
#include <string.h>

#include <sys/types.h>
#include <sys/param.h>
#include <sys/ioctl.h>
#include <sys/socket.h>

#include <linux/net_tstamp.h>

#include <netinet/in.h>
#include <netinet/ip_icmp.h>

struct mmsghdr
{
  struct msghdr msg_hdr;
  unsigned int msg_len;
} mmsghdr_t;


typedef signed char    sint8;
typedef unsigned char  uint8;
typedef signed short   sint16;
typedef unsigned short uint16;
typedef signed int     sint32;
typedef unsigned int   uint32;

struct addr_p
{
  uint8 a;
  uint8 b;
  uint8 c;
  uint8 d;
};

struct addr
{
  union
  {
    uint32 full;
    struct addr_p chunk;
  };
};

struct peers
{
  uint32 cnt;
  struct addr *itms;
};

#endif // MPING_COMMON_H
