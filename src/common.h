#ifndef MPING_COMMON_H
#define MPING_COMMON_H

struct mmsghdr
{
  struct msghdr msg_hdr;      /* Actual message header.  */
  unsigned int msg_len;       /* Number of received bytes for the entry.  */
};


typedef signed char    sint8;
typedef unsigned char  uint8;
typedef signed short   sint16;
typedef unsigned short uint16;
typedef signed int     sint32;
typedef unsigned int   uint32;

#endif // MPING_COMMON_H
