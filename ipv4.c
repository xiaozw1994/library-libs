#include "ping.h"
/*
ipv4-icmp 发送数据函数
*/
void send_v4(void)
{
	int len;
	struct icmp*icmp;
	icmp=(struct icmp *)sendbuf;
	icmp->icmp_type=ICMP_ECHO;
	icmp->icmp_code=0;
	icmp->icmp_id=pid;
	icmp->icmp_seq=nsent++;
	memset(icmp->icmp_data,0x5a,datalen);
	gettimeofday((struct timeval *)icmp->icmp_data,NULL);
	len=8+datalen;
	icmp->icmp_cksum=in_cksum((uint16_t *)icmp,len);
	sendto(sockfd,sendbuf,len,0,pr->sasend,pr->salen);
}
/*
ipv4回射函数
*/
void proc_v4(char * ptr,ssize_t len,struct msghdr * msg,struct timeval*tvrecv)
{
	int hlen1,icmplen;
	double rtt;
    struct ip *ip;
    struct icmp*icmp;
    struct timeval*tvsend;
    ip=(struct ip*)ptr;
    hlen1=ip->ip_hl<<2;
    //协议判断
    if(ip->ip_p != IPPROTO_ICMP)
    {
    	return ;
    }
    icmp=(struct icmp *)(ptr+hlen1);
    if((icmplen=len-hlen1)<8)
    {
    	return ;
    }
    //是回射返回
    if(icmp->icmp_type==ICMP_ECHOREPLY)
    {
    	if(icmp->icmp_id!=pid)
    	{
    		return ;
    	}
    	if(icmplen<16)
    		return ;
    tvsend=(struct timeval*)icmp->icmp_data;
    tv_sub(tvrecv,tvsend);
    rtt=tvrecv->tv_sec*1000.0+tvrecv->tv_usec/1000.0;
    printf("%d bytes from %s:seq=%u,ttl=%d,rtt=%.3f ms\n",icmplen,inet_ntop_host(pr->sarecv,pr->salen),icmp->icmp_seq,ip->ip_ttl,rtt);
   }
   else if(verbose)
   {
   	printf("%d bytes from %s:type=%d,code=%d\n",icmplen,inet_ntop_host(pr->sarecv,pr->salen),icmp->icmp_type,icmp->icmp_code);
   }
}