#ifndef MPING_COMMON_H
#define MPING_COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <poll.h>
#include <string.h>

#include <sys/types.h>
#include <sys/param.h>
#include <sys/ioctl.h>
#include <sys/socket.h>

#include <net/if.h>

#include <asm/types.h>
#include <linux/sockios.h>
#include <linux/net_tstamp.h>
#include <linux/errqueue.h>

#include <netinet/in.h>
#include <netinet/ip_icmp.h>




struct mmsghdr
{
  struct msghdr msg_hdr;
  unsigned int msg_len;
} mmsghdr_t;


typedef signed char         s8;
typedef unsigned char       u8;
typedef signed short       s16;
typedef unsigned short     u16;
typedef signed int         s32;
typedef unsigned int       u32;
typedef signed long int    s64;
typedef unsigned long int  u64;

typedef float              f32;
typedef double             f64;
typedef long double       f128;

struct addr_p
{
  u8 a;
  u8 b;
  u8 c;
  u8 d;
};

struct addr
{
  union
  {
    u32 full;
    struct addr_p chunk;
  };
};

struct peers
{
  u32 cnt;
  struct addr *itms;
};

#endif // MPING_COMMON_H
