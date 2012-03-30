#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <poll.h>
#include <sys/socket.h>
#include <bits/socket.h>
#include <netinet/in.h>
#include <netinet/ip_icmp.h>

#include "common.h"
#include "chksum.h"

int fd;

void print_stats_peer(struct mmsghdr *datagram, int count, int bytes)
{
  char peer[1024];

  int err = getnameinfo(datagram->msg_hdr.msg_name, datagram->msg_hdr.msg_namelen, peer, sizeof(peer), NULL, 0, 0);

  if (err != 0)
  {
    fprintf(stderr, "error using getnameinfo: %s\n", gai_strerror(err));
    return;
  }

  printf("    %d bytes received from %s in %d datagrams\n", bytes, peer, count);


/*
  char rbuf[sizeof(struct iphdr) + sizeof(struct icmphdr)];
  memcpy(rbuf, datagram->msg_hdr.msg_iov[0].iov_base, datagram->msg_hdr.msg_iov[0].iov_len);

  struct iphdr* recv_iphdr = (struct iphdr*)rbuf;
  struct icmphdr* recv_icmphdr = (struct icmphdr*)(rbuf + (recv_iphdr->ihl * 4));
*/

  struct iphdr* recv_iphdr = (struct iphdr *) datagram->msg_hdr.msg_iov[0].iov_base;
  struct icmphdr* recv_icmphdr = (struct icmphdr *)datagram->msg_hdr.msg_iov[1].iov_base;

  printf("    ");

  switch (recv_iphdr->protocol)
  {
    case IPPROTO_ICMP:
      printf("ICMP");
      break;
    default:
      printf("Unknown (%u)", recv_iphdr->protocol);
      break;
  }

  printf(", ");

  switch (recv_icmphdr->type)
  {
    case ICMP_ECHOREPLY:
      printf("Echo Reply");
      break;
    case ICMP_DEST_UNREACH:
      printf("Destination Unreachable");
      break;
    case ICMP_SOURCE_QUENCH:
      printf("Source Quench");
      break;
    case ICMP_REDIRECT:
      printf("Redirect (change route)");
      break;
    case ICMP_ECHO:
      printf("Echo Request");
      break;
    case ICMP_TIME_EXCEEDED:
      printf("Time Exceeded");
      break;
    case ICMP_PARAMETERPROB:
      printf("Parameter Problem");
      break;
    case ICMP_TIMESTAMP:
      printf("Timestamp Request");
      break;
    case ICMP_TIMESTAMPREPLY:
      printf("Timestamp Reply");
      break;
    case ICMP_INFO_REQUEST:
      printf("Information Request");
      break;
    case ICMP_INFO_REPLY:
      printf("Information Reply");
      break;
    case ICMP_ADDRESS:
      printf("Address Mask Request");
      break;
    case ICMP_ADDRESSREPLY:
      printf("Address Mask Reply");
      break;

    default:
      printf("Unknown (%u)", recv_icmphdr->type);
      break;
  }

  printf(": code: %u, id: %u, seq: %u\n", recv_icmphdr->code, recv_icmphdr->un.echo.id, recv_icmphdr->un.echo.sequence);
}

void *listener(void *arg)
{
  uint32 i;
  struct pollfd pfds[1];
  struct iovec iov[4][2];
  struct mmsghdr messages[4];
  struct sockaddr_in sock_addr[4];


  memset(pfds, 0, sizeof(pfds));

  pfds[0].fd = fd;
  pfds[0].events = POLLIN;

  memset(iov, 0, sizeof(iov));
  memset(messages, 0, sizeof(messages));
  memset(sock_addr, 0, sizeof(sock_addr));


  for (i = 0; i < 4; ++i)
  {
    struct iphdr ip_hdr;
    struct icmphdr icmp_hdr;

    iov[i][0].iov_base = &ip_hdr;
    iov[i][0].iov_len = sizeof(ip_hdr);
    iov[i][1].iov_base = &icmp_hdr;
    iov[i][1].iov_len = sizeof(icmp_hdr);

    messages[i].msg_hdr.msg_name = (struct sockaddr *)&sock_addr[i];
    messages[i].msg_hdr.msg_namelen = sizeof(sock_addr[i]);
    messages[i].msg_hdr.msg_iov=iov[i];
    messages[i].msg_hdr.msg_iovlen=2;
    messages[i].msg_hdr.msg_control=0;
    messages[i].msg_hdr.msg_controllen=0;
  }

  while (1)
  {
    if (poll(pfds, 1, -1) < 0)
      break;

    int nr_datagrams = recvmmsg(fd, messages, 4, MSG_DONTWAIT, NULL);

    if (nr_datagrams == 0)
    {
      perror("recvmmsg");
      break;
    }

    printf("nr_datagrams received: %d\n", nr_datagrams);

    int peer_count = 1;
    int peer_bytes = messages[0].msg_len;

    for (i = 1; i < nr_datagrams; ++i)
    {
      if (memcmp(messages[i - 1].msg_hdr.msg_name, messages[i].msg_hdr.msg_name, messages[i].msg_hdr.msg_namelen) == 0)
      {
        ++peer_count;
        peer_bytes += messages[i].msg_len;
        continue;
      }

      print_stats_peer(&messages[i - 1], peer_count, peer_bytes);

      peer_bytes = messages[i].msg_len;
      peer_count = 1;
    }

    print_stats_peer(&messages[nr_datagrams - 1], peer_count, peer_bytes);

    usleep(1);
  }

  return NULL;
}

int main(int argc, char **argv)
{
  uint32 i, b = 15;
  pthread_t pth;
  struct sockaddr_in sock_addr[b];
  struct in_addr dest_addr[b];
  struct iovec iov[b][1];
  struct mmsghdr messages[b];




  memset(dest_addr, 0, sizeof(dest_addr));

  dest_addr[0].s_addr = inet_addr("82.144.192.22");
  dest_addr[1].s_addr = inet_addr("82.144.192.23");
  dest_addr[2].s_addr = inet_addr("195.191.12.65");
  dest_addr[3].s_addr = inet_addr("195.191.12.69");
  dest_addr[4].s_addr = inet_addr("69.147.83.197");
  dest_addr[5].s_addr = inet_addr("87.250.251.3");
  dest_addr[6].s_addr = inet_addr("94.100.191.202");
  dest_addr[7].s_addr = inet_addr("209.85.148.102");
  dest_addr[8].s_addr = inet_addr("173.194.35.184");
  dest_addr[9].s_addr = inet_addr("188.127.231.181");
  dest_addr[10].s_addr = inet_addr("89.16.167.134");
  dest_addr[11].s_addr = inet_addr("87.106.54.147");
  dest_addr[12].s_addr = inet_addr("77.234.201.242");
  dest_addr[13].s_addr = inet_addr("89.184.65.135");
  dest_addr[14].s_addr = inet_addr("46.4.96.250");
  dest_addr[15].s_addr = inet_addr("207.97.227.239");
  dest_addr[16].s_addr = inet_addr("137.254.16.101");

  memset(sock_addr, 0, sizeof(sock_addr));


  for (i = 0; i < b; ++i)
  {
    sock_addr[i].sin_family = AF_INET;
    sock_addr[i].sin_port = htons(0);
    sock_addr[i].sin_addr = dest_addr[i];
  }


  fd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
  if (-1 == fd)
  {
    printf("Error 1\n");
    return 1;
  }


  pthread_create(&pth, NULL, listener, NULL);


  uint32 seq = 0;

  while (1)
  {
    seq++;

    memset(iov, 0, sizeof(iov));
    memset(messages, 0, sizeof(messages));

    for (i = 0; i < b; ++i)
    {
      struct icmphdr req;

      memset(&req, 0, sizeof(struct icmphdr));

      req.type = ICMP_ECHO;
      req.code = 0;
      req.un.echo.id = 50;
      req.un.echo.sequence = seq;
      req.checksum = chksum((unsigned short*) &req, sizeof(struct icmphdr));

      iov[i][0].iov_base = &req;
      iov[i][0].iov_len = sizeof(req);

      messages[i].msg_hdr.msg_name = (struct sockaddr *) &sock_addr[i];
      messages[i].msg_hdr.msg_namelen = sizeof(sock_addr[i]);
      messages[i].msg_hdr.msg_iov = iov[i];
      messages[i].msg_hdr.msg_iovlen = 1;
      messages[i].msg_hdr.msg_control = 0;
      messages[i].msg_hdr.msg_controllen = 0;
    }

    printf("\n\n");

    if (sendmmsg(fd, messages, b, 0) == -1)
    {
      perror("sendmsg");
    }

    sleep(1);
  }

  pthread_join(pth, NULL);

  close(fd);

  return 0;
}



/*
void recc()
{
  printf("recv...\n");

  // the received packet contains the IP header...
  char rbuf[sizeof(struct iphdr) + sizeof(struct icmphdr)];
  struct sockaddr_in raddr;
  socklen_t raddr_len;

  // receive the packet that we sent (since we sent it to ourselves,
  // and a raw socket sees everything...).
  int rc = recvfrom(fd, rbuf, sizeof(rbuf), 0, (struct sockaddr*)&raddr, &raddr_len);
  if (rc == -1) {
    perror("recvfrom 1:");
    return;
  }


  struct iphdr* iphdr = NULL;
  struct icmphdr* recv_icmphdr = NULL;

  // we got an IP packet - verify that it contains an ICMP message.
  iphdr = (struct iphdr*)rbuf;
  if (iphdr->protocol != IPPROTO_ICMP) {
    fprintf(stderr, "Expected ICMP packet, got %u\n", iphdr->protocol);
    return;
  }


  // verify that it's an ICMP echo request, with the expected seq. num + id.
  recv_icmphdr = (struct icmphdr*)(rbuf + (iphdr->ihl * 4));
  if (recv_icmphdr->type != ICMP_ECHOREPLY) {
    fprintf(stderr, "Expected ICMP echo-reply, got %u\n", recv_icmphdr->type);
    return;
  }

  if (recv_icmphdr->un.echo.sequence != 50) {
    fprintf(stderr,
            "Expected sequence 50, got %d\n", recv_icmphdr->un.echo.sequence);
    return;
  }

  if (recv_icmphdr->un.echo.id != 48) {
    fprintf(stderr,
            "Expected id 48, got %d\n", recv_icmphdr->un.echo.id);
    return;
  }

  printf("Got the expected ICMP echo-request\n");
}

*/
