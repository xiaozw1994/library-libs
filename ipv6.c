#include "ping.h"
/*
ipv6进行发送数据，icmp结构体--其结构体的选项是发送；
获取其中发送的时间分组；
*/
void send_v6(void)
{
	#ifdef IPV6
      int len;
      struct icmp6_hdr * icmp6;
      icmp6=(strcut icmp6_hdr*)sendbuf;
      icmp6->icmp6_code=0;
      icmp6->icmp6_type=ICMP6_ECHO_REQUEST;
      icmp6->icmp6_id=pid;
      icmp6->icmp6_seq=nsent++;
      memset((icmp6+1),0x5a,datalen);
      gettimeofday((struct timeval*)(icmp6+1),NULL);
      len=8+datalen;
      sendto(sockfd,sendbuf,len,0,pr->sasend,pr->salen);

	#endif
}
/*
    ipv6--ICMP6回射处理函数，步骤：1、先要判断其类型是回射，否则出错
    2、其icmpv6大小判断；
    3、计算rtt-算出来的ms；
    4、hlim是代表是跳限
    5、versboser用来标志是否有-v详尽列出
*/
void proc_v6(char * ptr,ssize_t len,struct msghdr * msg,struct timeval*tvrecv)
{
      #ifdef IPV6
      double rtt;
      struct icmp6_hdr* icmp6;
      struct timeval * tvsend;
      struct cmsghdr *cmsg;
      int hlim;
      icmp6=(struct icmp6_hdr*)ptr;
    if(len<8)
      return ;
    if(icmp6->icmp6_type==ICMP6_ECHO_REPLY)
    {
      if(icmp6->icmp6_id!=pid)
            return ;
      if(len<16)
            return ;
        tvsend=(struct timeval*)(icmp6+1);
        tv_sub(tvrecv,tvsend);
        rtt=tvrecv->tv_sec*1000.0+tvrecv->tv_usec/1000.0;
        hlim=-1;
        for(cmsg=CMSG_FIRSTHDR(msg);cmsg!=NULL;cmsg=CMSG_NXTHDR(msg,cmsg))
        {
            if(cmsg->cmsg_level==IPPROTO_IPV6&&cmsg->cmsg_type==IPV6_HOPLIMIT)
            {
                  hlim=*(uint32_t *)CMSG_DATA(cmsg);
                  break;
            }
        }
        printf("%d bytes from %s :seq=%u,hlim=",len,inet_ntop_host(pr->sarecv,pr->salen),icmp6->icmp6_seq);
        if(hlim==-1)
            printf("???");
        else
            printf("%d",hlim);
        printf(", rtt=%.3f ms \n",rtt);

    }
    else if(verbose)
    {
      printf("%d bytes from %s :type=%d,code=%d\n",len,inet_ntop_host(pr->sarecv,pr->salen),icmp6->icmp6_type,icmp6->icmp6_code);
    }
      #endif
}

/*
初始化：这里的代表是指定icmpv6通过其他阻塞-setsockopt设置
*/
void init_v6(void)
{
      #ifdef IPV6
    int con=1;
    if(verbose==0)
    {
      struct icmp6_filter myfilt;
      ICMP6_FILTER_SETBLOCKALL(&myfilt);
      ICMP6_FILTER_SETPASS(ICMP6_ECHO_REPLY,&myfilt);
      setsockopt(sockfd,IPPROTO_IPV6,ICMP6_FILTER,&myfilt,sizeof(myfilt));

    }
#ifdef  IPV6_RECVHOPLIMIT
  setsockopt(sockfd,IPPROTO_IPV6,IPV6_HOPLIMIT,&on,sizeof(on));
    #endif
    #endif
}