#include <stdio.h>
#include <stdlib.h>

#include "common.h"
#include "chksum.h"
#include "file.h"

int main(int argc, char **argv)
{
  u32 i, t;

  struct iovec *iov;
  struct mmsghdr *messages;
  struct sockaddr_in *sock_addr;
  struct icmphdr *icmp_hdr;
  struct peers *hosts;


  hosts = file_fetch("misc/hosts");
  if (hosts == NULL || hosts->cnt == 0)
    return 1;

  printf("Loaded %u hosts\n", hosts->cnt);

  iov = calloc(hosts->cnt, sizeof(struct iovec));
  messages = calloc(hosts->cnt, sizeof(struct mmsghdr));
  sock_addr = calloc(hosts->cnt, sizeof(struct sockaddr_in));
  icmp_hdr = calloc(hosts->cnt, sizeof(struct icmphdr));

  for (i = 0; i < hosts->cnt; ++i)
  {
    sock_addr[i].sin_family = AF_INET;
    memcpy(&sock_addr[i].sin_addr, &hosts->itms[i].full, sizeof(u32));

    icmp_hdr[i].type = ICMP_ECHO;
    icmp_hdr[i].code = 0;
    icmp_hdr[i].un.echo.id = (i + 1) & 0xFFFF;
    icmp_hdr[i].un.echo.sequence = 0;
    icmp_hdr[i].checksum = 0;

    iov[i].iov_base = &icmp_hdr[i];
    iov[i].iov_len = sizeof(icmp_hdr[i]);

    messages[i].msg_hdr.msg_name = (struct sockaddr *)&sock_addr[i];
    messages[i].msg_hdr.msg_namelen = sizeof(sock_addr[i]);
    messages[i].msg_hdr.msg_iov = &iov[i];
    messages[i].msg_hdr.msg_iovlen = 1;
    messages[i].msg_hdr.msg_control = 0;
    messages[i].msg_hdr.msg_controllen = 0;
  }



  libmping_open();

  u32 cnt = 5;
  while (cnt--)
  {
    libmping_poll(messages, 0, 201);
  }

  libmping_print();

  libmping_close();


  free(hosts);
  free(iov);
  free(messages);
  free(sock_addr);
  free(icmp_hdr);

  return 0;
}
