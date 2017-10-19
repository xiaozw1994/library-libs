#include "ping.h"

int Socket(int af,int ap,int flags)
{
	int flag=socket(af,ap,flags);
	if(flag<0)
	{
		err_quit("socket_eorror!");
	}
	else
		return flag;
}

/*
信号处理函数，信号处理函数功能：发送一个ICMP回报请求，同时设置下一次的调度在1s后产生
sigalarm信号
*/
void sig_alrm(int signo)
{
	(*pr->fsend)();
	alarm(1);
	return ;
}
/*
两个结构时间想减
*/
void tv_sub(struct timeval *out,struct timeval*in)
{
	if((out->tv_usec-=in->tv_usec)<0)
    {
    	--out->tv_sec;
    	out->tv_usec+=1000000;
    }
    out->tv_sec=in->tv_sec;

}
/*
该函数实现以下功能：1、创建套接字；
2、执行特定协议的初始化，设置套套接字的缓冲区借用iov，设置msghdr结构；
3、读入的icmp进入无限循环状态，gettimeofday记录收发分组的时间，调用函数进行处理
*/
void readloop(void)
{
	int size;
	char recvbuf[MAX];
    char controlbuf[MAX];
    struct msghdr msg;
    struct iovec iov;
    ssize_t n;
    struct timeval tval;
    sockfd=Socket(pr->sasend->sa_family,SOCK_RAW,pr->icmpproto);
    setuid(getuid());
    if(pr->finit)
    	(*pr->finit)();
    size=60*1024;
    setsockopt(sockfd,SOL_SOCKET,SO_RCVBUF,&size,sizeof(size));

    sig_alrm(SIGALRM);
//数据缓冲区，辅助数据缓冲区，
    iov.iov_base=recvbuf;
    iov.iov_len=sizeof(recvbuf);
    msg.msg_name=pr->sarecv;
    msg.msg_iov=&iov;
    msg.msg_iovlen=1;
    msg.msg_control=controlbuf;
    while(1)
    {
       msg.msg_namelen=pr->salen;
       msg.msg_controllen=sizeof(controlbuf);
       n=recvmsg(sockfd,&msg,0);
       if(n<0)
       {
          if(errno==EINTR)
          	continue;
          else
          	err_quit("recieve error");
       }
       gettimeofday(&tval,NULL);
       (*pr->fproc)(recvbuf,n,&msg,&tval);

    }
}
/*
网际和交验方法-ICMP校验
该函数是由mike muuss提供的功用的函数方法
*/
uint16_t in_cksum(uint16_t * addr,int len)
{
	int nleft=len;
	uint32_t sum=0;
	uint16_t * w=addr;
	uint16_t answer=0;
	while(nleft>1)
	{
		sum+=*w++;
		nleft-=2;
	}
	if(nleft==1)
	{
		*(unsigned char *)(&answer)=*(unsigned char*)w;
		sum+=answer;
	}
	sum=(sum>>16)+(sum & 0xffff);
	sum+=(sum>>16);
	answer=~sum;
	return answer;
}



