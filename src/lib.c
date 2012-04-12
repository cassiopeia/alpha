#include "lib.h"

s32 fd;
struct stat statistics[201];

s32 libmping_open()
{
  struct ifreq device;
  struct ifreq hwtstamp;
  struct hwtstamp_config hwconfig, hwconfig_requested;

  struct sockaddr_in addr;

  fd = socket (AF_INET, SOCK_RAW, IPPROTO_ICMP);

  if (fd < 0)
  {
    fprintf(stderr, "Unable open socket\n");
    return fd;
  }

  memset (&device, 0, sizeof(device));

  strncpy (device.ifr_name, "eth0", sizeof (device.ifr_name));
  if (ioctl (fd, SIOCGIFADDR, &device) < 0)
    fprintf (stderr, "getting interface IP address\n");

  memset (&hwtstamp, 0, sizeof (hwtstamp));
  strncpy (hwtstamp.ifr_name, "eth0", sizeof (hwtstamp.ifr_name));
  hwtstamp.ifr_data = (void *)&hwconfig;
  memset (&hwconfig, 0, sizeof (&hwconfig));

  hwconfig.tx_type = HWTSTAMP_TX_ON;
  hwconfig.rx_filter = HWTSTAMP_FILTER_ALL;

  hwconfig_requested = hwconfig;

  if (ioctl (fd, SIOCSHWTSTAMP, &hwtstamp) < 0)
  {
    if ((errno == EINVAL || errno == ENOTSUP) &&
        hwconfig_requested.tx_type == HWTSTAMP_TX_OFF &&
        hwconfig_requested.rx_filter == HWTSTAMP_FILTER_NONE)
    {
      printf ("SIOCSHWTSTAMP: disabling hardware time stamping not possible\n");
    }
    else
      perror ("SIOCSHWTSTAMP");
  }
  printf ("SIOCSHWTSTAMP: tx_type %d requested, got %d; rx_filter %d requested, got %d\n",
    hwconfig_requested.tx_type,
    hwconfig.tx_type,
    hwconfig_requested.rx_filter,
    hwconfig.rx_filter
  );

  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htonl (INADDR_ANY);
  addr.sin_port = htons (0);
  if (bind (fd, (struct sockaddr *)&addr, sizeof (struct sockaddr_in)) < 0)
    perror ("bind");


  s32 status;

  u32 recvbuf = 65535 + 65535 + 65535;
  status = setsockopt (fd, SOL_SOCKET, SO_SNDBUF, &recvbuf, sizeof (recvbuf));
  if (status != 0)
    fprintf (stderr, "sndbuf error\n");
  else
    fprintf (stdout, "sndbuf %u\n", recvbuf);

  status = setsockopt (fd, SOL_SOCKET, SO_RCVBUF, &recvbuf, sizeof (recvbuf));
  if (status != 0)
    fprintf (stderr, "rcvbuf error\n");
  else
    fprintf (stdout, "rcvbuf %u\n", recvbuf);


  int flags = 0;
  //flags |= SOF_TIMESTAMPING_TX_HARDWARE;
  flags |= SOF_TIMESTAMPING_TX_SOFTWARE;
  //flags |= SOF_TIMESTAMPING_RX_HARDWARE;
  flags |= SOF_TIMESTAMPING_RX_SOFTWARE;
  flags |= SOF_TIMESTAMPING_SOFTWARE;
  //flags |= SOF_TIMESTAMPING_SYS_HARDWARE;
  //flags |= SOF_TIMESTAMPING_RAW_HARDWARE;
  status = setsockopt (fd, SOL_SOCKET, SO_TIMESTAMPING, &flags, sizeof (flags));
  if (status < 0)
    perror ("SO_TIMESTAMPING");


  u8 tos = IPTOS_LOWDELAY;
  status = setsockopt (fd, IPPROTO_IP, IP_TOS, &tos, sizeof (tos));
  if (status != 0)
    fprintf (stderr, "IP_TOS error\n");


  memset (statistics, 0, sizeof (statistics));

  u32 i;
  for (i = 0; i < 201; ++i)
  {
    statistics[i].iden = 0;
    statistics[i].seqn = 0;

    statistics[i].rtt_min = -1;
    statistics[i].rtt_prv = -1;
    statistics[i].jtr_min = -1;
  }

  return fd;
}

s32 libmping_poll (struct mmsghdr *datagrams, u32 offset, u32 count)
{
  u32 i;

  char control_buffer[count][512];

  struct iovec iov[count][2];
  struct mmsghdr messages[count];
  struct sockaddr_in sock_addr[count];

  struct iphdr ip_hdr[count];
  struct icmphdr icmp_hdr[count];

  struct timespec ts_temp;

  struct cmsghdr     *cmsg;
  struct iphdr       *ip_ptr;
  struct icmphdr     *icmp_ptr;
  struct sockaddr_in *addr_ptr;

  memset(iov, 0, sizeof(iov));
  memset(messages, 0, sizeof(messages));
  memset (sock_addr, 0, sizeof (sock_addr));

  for (i = 0; i < count; ++i)
  {
    icmp_ptr = (struct icmphdr *)datagrams[i].msg_hdr.msg_iov[0].iov_base;

    icmp_ptr->checksum = 0;
    icmp_ptr->un.echo.sequence += 1;

    icmp_ptr->checksum = chksum ((u16 *)icmp_ptr, sizeof (*icmp_ptr));

    if (statistics[i].addr.full == 0)
    {
      addr_ptr = (struct sockaddr_in *)datagrams[i].msg_hdr.msg_name;
      memcpy (&statistics[i].addr.full, &addr_ptr->sin_addr, sizeof (u32));
    }

    statistics[i].iden = icmp_ptr->un.echo.id;
    statistics[i].seqn = icmp_ptr->un.echo.sequence;
    statistics[i].sent++;

    iov[i][0].iov_base = &ip_hdr[i];
    iov[i][0].iov_len = sizeof (struct iphdr);
    iov[i][1].iov_base = &icmp_hdr[i];
    iov[i][1].iov_len = sizeof (struct icmphdr);

    messages[i].msg_hdr.msg_name = (struct sockaddr *)&sock_addr[i];
    messages[i].msg_hdr.msg_namelen = sizeof (sock_addr[i]);
    messages[i].msg_hdr.msg_iov = iov[i];
    messages[i].msg_hdr.msg_iovlen = 2;
    messages[i].msg_hdr.msg_control = control_buffer[i];
    messages[i].msg_hdr.msg_controllen = sizeof (control_buffer[i]);
  }


  u16 j;
  s32 ret;
  float tmp, sub;

  ret = sendmmsg (fd, datagrams, count, 0);

  for (i = 0; i < count; ++i)
  {
    clock_gettime (CLOCK_REALTIME, &statistics[i].time);
  }

  sleep (1);

  ret = recvmmsg (fd, messages, count, MSG_DONTWAIT, NULL);

  for (i = 0; i < ret; ++i)
  {
    ip_ptr = (struct iphdr *)messages[i].msg_hdr.msg_iov[0].iov_base;
    icmp_ptr = (struct icmphdr *)messages[i].msg_hdr.msg_iov[1].iov_base;
    addr_ptr = (struct sockaddr_in *)messages[i].msg_hdr.msg_name;

    for (cmsg = CMSG_FIRSTHDR (&messages[i].msg_hdr); cmsg != NULL; cmsg = CMSG_NXTHDR (&messages[i].msg_hdr, cmsg))
    {
      if (cmsg->cmsg_level == SOL_SOCKET)
      {
        switch (cmsg->cmsg_type)
        {
          case SO_TIMESTAMPING:
            memcpy (&ts_temp, CMSG_DATA (cmsg), sizeof (ts_temp));
            break;
          default:
            fprintf (stderr, "Unknown %d\n", cmsg->cmsg_type);
            break;
        }
      }
    }

    j = icmp_ptr->un.echo.id - 1;

    if (j > count)
      continue;

    if (statistics[j].iden != icmp_ptr->un.echo.id)
      continue;

    if (statistics[j].seqn != icmp_ptr->un.echo.sequence)
      continue;

    tmp = timespec_diff (&statistics[j].time, &ts_temp) / 1.0e6;

    if (statistics[j].rtt_min == -1 || statistics[j].rtt_min > tmp)
      statistics[j].rtt_min = tmp;

    if (statistics[j].rtt_max < tmp)
      statistics[j].rtt_max = tmp;

    statistics[j].rtt_sum += tmp;

    if (statistics[j].rtt_prv != -1)
    {
      if (statistics[j].rtt_prv > tmp)
        sub = statistics[j].rtt_prv - tmp;
      else
        sub = tmp - statistics[j].rtt_prv;

      if (statistics[j].jtr_min == -1 || statistics[j].jtr_min > sub)
        statistics[j].jtr_min  = sub;

      if (statistics[j].jtr_max < sub)
        statistics[j].jtr_max = sub;

      statistics[j].jtr_sum += sub;
    }

    statistics[j].rtt_prv = tmp;

    if (icmp_ptr->type == ICMP_ECHOREPLY)
      statistics[j].recv++;
    else
      statistics[j].errs++;
  }

  return ret;
}

s32 libmping_close ()
{
  return close (fd);
}

s32 libmping_print ()
{
  f32 p, rtt_avg, jtr_avg, avail_sum;
  u32 i, recv, avail_cnt = 0;
  char peer[15];

  printf ("Statistics\n");

  for (i = 0; i < 201; ++i)
  {
    if (!statistics[i].sent) continue;

    recv = statistics[i].recv + statistics[i].errs;

    sprintf (peer, "%u.%u.%u.%u", statistics[i].addr.chunk.a, statistics[i].addr.chunk.b, statistics[i].addr.chunk.c, statistics[i].addr.chunk.d);

    p = (float)statistics[i].recv / (float)statistics[i].sent * 100.0;

    avail_cnt += 1;
    avail_sum += p;

    rtt_avg = 0.0;
    jtr_avg = 0.0;

    if (recv)
    {
      rtt_avg = statistics[i].rtt_sum / (f32) recv;
      jtr_avg = statistics[i].jtr_sum / (f32) recv;
    }

    if (statistics[i].rtt_min == -1)
      statistics[i].rtt_min = 0.0;

    if (statistics[i].jtr_min == -1)
      statistics[i].jtr_min = 0.0;

    printf ("%15s | %5u sent %5u recv %5u errs %3.0f loss | %6.2f %6.2f %6.2f rtt | %6.2f %6.2f %6.2f jtr\n",
      peer,
      statistics[i].sent,
      statistics[i].recv,
      statistics[i].errs,
      100.0 - p,
      statistics[i].rtt_min,
      rtt_avg,
      statistics[i].rtt_max,
      statistics[i].jtr_min,
      jtr_avg,
      statistics[i].jtr_max
    );
  }

  printf ("--------------------------------------------------------------\n");
  printf ("                 %5u sent, %5u recv, %3.0f avail, %5u errs\n", 0, 0, avail_sum / avail_cnt, 0);
}

void print_stats_peer (struct mmsghdr *messages, u32 count)
{
  u32 i, j = 0 ;
  char peer[1024];

  struct iphdr       *recv_iphdr;
  struct icmphdr     *recv_icmphdr;
  struct sockaddr_in *recv_addr;

  for (i = 0; i < count; ++i)
  {
    recv_iphdr = (struct iphdr *)messages[i].msg_hdr.msg_iov[0].iov_base;
    recv_icmphdr = (struct icmphdr *)messages[i].msg_hdr.msg_iov[1].iov_base;

    if (recv_iphdr->protocol != IPPROTO_ICMP)
    {
      printf("Unknown protocol\n");
      return;
    }

    recv_addr = (struct sockaddr_in *)messages[i].msg_hdr.msg_name;
    inet_ntop(AF_INET, &recv_addr->sin_addr, peer, 1024);

    for (j = 0; j < 1024; ++j)
    {
      if (recv_addr->sin_addr.s_addr == statistics[j].addr.full)
      {
        break;
      }
    }

    if (recv_icmphdr->type != ICMP_ECHOREPLY)
    {
      statistics[j].errs++;
    }

    if (recv_icmphdr->type == ICMP_ECHOREPLY)
    {
      statistics[j].recv++;

/*
      printf("Reply from %15s: %s seq=%u bytes=%u\n",
        peer,
        "Echo Reply",
        recv_icmphdr->un.echo.sequence,
        messages[i].msg_len
      );
*/

    }
    else if (recv_icmphdr->type == ICMP_DEST_UNREACH)
    {
      switch (recv_icmphdr->code)
      {
        case ICMP_UNREACH_NET:
          printf("Reply from %15s: Destination Unreachable (bad net)\n", peer);
          break;
        case ICMP_UNREACH_HOST:
          printf("Reply from %15s: Destination Unreachable (bad host)\n", peer);
          break;
        case ICMP_UNREACH_PROTOCOL:
          printf("Reply from %15s: Destination Unreachable (bad protocol)\n", peer);
          break;
        case ICMP_UNREACH_PORT:
          printf("Reply from %15s: Destination Unreachable (bad port)\n", peer);
          break;
        case ICMP_UNREACH_NEEDFRAG:
          printf("Reply from %15s: Destination Unreachable (IP_DF caused drop, set MTU %d)\n", peer, 0 /*ntohs(recv_iphdr->icmp_nextmtu)*/);
          break;
        case ICMP_UNREACH_SRCFAIL:
          printf("Reply from %15s: Destination Unreachable (src route failed)\n", peer);
          break;
        case ICMP_UNREACH_NET_UNKNOWN:
          printf("Reply from %15s: Destination Unreachable (unknown net)\n", peer);
          break;
        case ICMP_UNREACH_HOST_UNKNOWN:
          printf("Reply from %15s: Destination Unreachable (unknown host)\n", peer);
          break;
        case ICMP_UNREACH_ISOLATED:
          printf("Reply from %15s: Destination Unreachable (src host isolated)\n", peer);
          break;
        case ICMP_UNREACH_NET_PROHIB:
          printf("Reply from %15s: Destination Unreachable (net denied)\n", peer);
          break;
        case ICMP_UNREACH_HOST_PROHIB:
          printf("Reply from %15s: Destination Unreachable (host denied)\n", peer);
          break;
        case ICMP_UNREACH_TOSNET:
          printf("Reply from %15s: Destination Unreachable (bad tos for net)\n", peer);
          break;
        case ICMP_UNREACH_TOSHOST:
          printf("Reply from %15s: Destination Unreachable (bad tos for host)\n", peer);
          break;
        case ICMP_UNREACH_FILTER_PROHIB:
          printf("Reply from %15s: Destination Unreachable (admin prohib)\n", peer);
          break;
        case ICMP_UNREACH_HOST_PRECEDENCE:
          printf("Reply from %15s: Destination Unreachable (host prec vio.)\n", peer);
          break;
        case ICMP_UNREACH_PRECEDENCE_CUTOFF:
          printf("Reply from %15s: Destination Unreachable (prec cutoff)\n", peer);
          break;
        default:
          printf("Reply from %15s: Destination Unreachable (bad code)\n", peer);
          break;
      }
    }
    else if (recv_icmphdr->type == ICMP_SOURCE_QUENCH)
    {
      printf("Reply from %15s: %s\n", peer, "Source Quench");
    }
    else if (recv_icmphdr->type == ICMP_REDIRECT)
    {
      switch (recv_icmphdr->code)
      {
        case ICMP_REDIRECT_NET:
          printf("Reply from %15s: Redirect (for network)\n", peer);
          break;
        case ICMP_REDIRECT_HOST:
          printf("Reply from %15s: Redirect (for host)\n", peer);
          break;
        case ICMP_REDIRECT_TOSNET:
          printf("Reply from %15s: Redirect (for tos and net)\n", peer);
          break;
        case ICMP_REDIRECT_TOSHOST:
          printf("Reply from %15s: Redirect (for tos and host)\n", peer);
          break;
        default:
          printf("Reply from %15s: Redirect (bad code)\n", peer);
          break;
      }
    }
    else if (recv_icmphdr->type == ICMP_ECHO)
    {
      printf("Reply from %15s: %s\n", peer, "Echo Request");
    }
    else if (recv_icmphdr->type == ICMP_TIME_EXCEEDED)
    {
      switch (recv_icmphdr->code)
      {
        case ICMP_TIMXCEED_INTRANS:
          printf("Reply from %15s: Time Exceeded (in transit)\n", peer);
          break;
        case ICMP_TIMXCEED_REASS:
          printf("Reply from %15s: Time Exceeded (in reass)\n", peer);
          break;
        default:
          printf("Reply from %15s: Time Exceeded\n", peer);
          break;
      }
    }
    else if (recv_icmphdr->type == ICMP_PARAMETERPROB)
    {
      switch (recv_icmphdr->code)
      {
        case ICMP_PARAMPROB_OPTABSENT:
          printf("Reply from %15s: Parameter Problem (req. opt. absent)\n", peer);
          break;
        default:
          printf("Reply from %15s: Parameter Problem\n", peer);
          break;
      }
    }
    else if (recv_icmphdr->type == ICMP_TIMESTAMP)
    {
      printf("Reply from %15s: %s\n", peer, "Timestamp Request");
    }
    else if (recv_icmphdr->type == ICMP_TIMESTAMPREPLY)
    {
      printf("Reply from %15s: %s\n", peer, "Timestamp Reply");
    }
    else if (recv_icmphdr->type == ICMP_INFO_REQUEST)
    {
      printf("Reply from %15s: %s\n", peer, "Information Request");
    }
    else if (recv_icmphdr->type == ICMP_INFO_REPLY)
    {
      printf("Reply from %15s: %s\n", peer, "Information Reply");
    }
    else if (recv_icmphdr->type == ICMP_ADDRESS)
    {
      printf("Reply from %15s: %s\n", peer, "Address Mask Request");
    }
    else if (recv_icmphdr->type == ICMP_ADDRESSREPLY)
    {
      printf("Reply from %15s: %s\n", peer, "Address Mask Reply");
    }
    else
    {
      printf("Reply from %15s: Unknown (%u)\n", peer, recv_icmphdr->type);
    }
  }
}
