#include <errno.h>
#include <stdio.h>
#include <sys/select.h>
#include <time.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <poll.h>
#include <limits.h>
#define PORT 34560
#define MAX 100
#ifndef LAN
#define LAN 5
#endif
int main(int argc, const char * argv[]) {
    // insert code here...
    struct sockaddr_in server,client;
    struct pollfd polls[OPEN_MAX];
    int sockfd,num,conn,maxi,i,nready,scok;
    char re_suff[MAX],se_suff[MAX];
    socklen_t len=sizeof(client);
    memset(re_suff, 0, 100);
    memset(se_suff, 0, MAX);
    sockfd=socket(AF_INET,SOCK_STREAM,0);
    server.sin_family=AF_INET;
    server.sin_port=htons(PORT);
    server.sin_addr.s_addr=htonl(INADDR_ANY);
    if(sockfd==-1)
    {
        puts("socket error!");
        exit(1);
    }
    if(bind(sockfd, (struct sockaddr *)&server, sizeof(server))==-1)
    {
        puts("bind error!");
        exit(1);
    }
    if(listen(sockfd, LAN)==-1)
    {
        puts("listen error");
        exit(1);
    }
    /*
      struct pollfd{
       int fd;    描述字符
       short events;  调用值
       short revents; 返回值
      };

    */
    polls[0].events=POLLRDBAND;
    polls[0].fd=sockfd;
    for(i=1;i<OPEN_MAX;i++)
        polls[i].fd=-1;
    maxi=0;
    while(1)
    {
        nready=poll(polls, maxi+1,INFTIM);
        if(polls[0].revents&POLLRDBAND)
        {
            conn=accept(sockfd, (struct sockaddr*)&client, &len);
            if(conn<0)
            {
                if(errno==EINTR)
            }
            for(i=1;i<OPEN_MAX;i++)
            {
                if(polls[i].fd<0)
                {
                    polls[i].fd=conn;
                    printf("its ip is %s,and the port is %d\n",inet_ntoa(client.sin_addr),ntohs(client.sin_port));
                    break;
                }
            }
            if(i==OPEN_MAX)
            {
                puts("too many clients");
                exit(1);
            }
            polls[i].events=POLLRDBAND;
            if(i>maxi)
                maxi=i;
            if(--nready<=0)
                continue;
        }
        for(i=1;i<OPEN_MAX;i++)
        {
            scok=polls[i].fd;
            if(scok<0)
                continue;
            if(polls[i].revents&(POLLRDBAND|POLLERR))
            {
                if((num=(int)read(scok, re_suff, MAX))==0)
                {
                    polls[i].fd=-1;
                    close(scok);
                }
                else if(num>0&&num<MAX)
                {
                    re_suff[num]='\0';
                    printf("recvieve from client is %s\n",re_suff);
                    memcpy(se_suff, re_suff, num);
                    printf("the send is %s\n",se_suff);
                }
                if(num<0)
                {
                    if(errno==ECONNRESET)
                    {
                        close(scok);
                        polls[i].fd=-1;
                    }
                }
                
                if(--nready<=0)
                    break;
            }
            
            
            
        }
        
        
    }
    return 0;
}