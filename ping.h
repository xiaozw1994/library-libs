#include <stdlib.h>
#include <netdb.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <string.h>
#include <mm_malloc.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <signal.h>
#include <time.h>
#include <netinet/ip_icmp.h>

#define MAX 1000
//出错函数处理
void err_quit(const char * str)
{
	fprintf(stderr, "error:%s\n", str);
	exit(0);
}
char * inet_ntop_host(const struct sockaddr * sa,socklen_t len);
struct addrinfo * host_serv(const char *host,const char * serv,int family,int socktype);
char sendbuf[MAX];
//保存数据的suffer
/* 
全局变量用来保存 ipv4 ipv6的一些信息--globals
 */
int  datalen;
char * host;
int nsent;
pid_t pid;
//保存该进程id使表示符编程进程ID
/*
    接下来的函数依次为初始化函数，处理ℹ️pv4，ipv6函数，发送数据函数，循环发送icmp，信号函数每一秒就会发送一次
*/
int sockfd;
int verbose;
void init_v6(void);
void proc_v4(char *,ssize_t ,struct msghdr*,struct timeval*);
void proc_v6(char *,ssize_t,struct msghdr*,struct timeval*);
void send_v4(void);
void send_v6(void);
void readloop(void);
void sig_alrm(int);
void tv_sub(struct timeval*,struct timeval*);
//构造的协议的结构题类型，结构题的类型包涵的数据处理函数指针，发送函数指针，初始化函数指针，发送数据的sock
//发送的从函数getaddrinfo得到，接收目的地址，大小，icmp协议值
struct proto{
   void (*fproc)(char *,ssize_t ,struct msghdr*,struct timeval*);
   void (*fsend)(void);
   void (*finit)(void);
   struct sockaddr *sasend;
   struct sockaddr *sarecv;
   socklen_t salen;
   int icmpproto;
}*pr;
#ifdef IPV6
#include <netinet/ip6.h>
#include <netinet/icmp6.h>
#endif
