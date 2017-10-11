//
//  main.c
//  mutilSelectTCPandUDp
//
//  Created by 肖智文 on 2017/9/24.
//  Copyright © 2017年 肖智文. All rights reserved.
//
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/types.h>
#include <time.h>
#include <sys/time.h>
#include <string.h>
#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <signal.h>
#define MAX 100
#define PORT 34567
#define LAN 5
const char * end="quit\n";
void funct(int sign);
/*
   子进程出错返回的状态等显示函数
 */
void err_child_progress(const int status)
{
    if(WIFEXITED(status))
    {
        fprintf(stdout,"其终止状态是按照正常终止的\n" );
        return;
    }
    else if(WIFSIGNALED(status))
    {
        printf("异常终止,终止的信号的编号是%d\n",WTERMSIG(status));
        return;
    }
    else if(WIFSTOPPED(status))
    {
        printf("当前暂停子进程，子进程暂停编号是：%d\n",WSTOPSIG(status));
        return ;
    }
}
void err_quit(const char *s)
{
    puts(s);
    exit(0);
}
int MAX_Variable(const int a,const int b)
{
    return a>b?a:b;
}
int Socket(const int AF,const int SO,const int a)
{
    int sockfd;
    sockfd=socket(AF,SO,a);
    if(sockfd==-1)
        err_quit("socket error!");
    return sockfd;
}

int Bind(const int Sockfd,const struct sockaddr *server,socklen_t len)
{
    int bin;
    bin=bind(Sockfd, server, len);
    if(bin==-1)
        err_quit("bind error!");
    return bin;
}
int Listen(const int sockfd,const int lan)
{
    int listened;
    listened=listen(sockfd, lan);
    if(listened==-1)
        err_quit("listen error");
    return listened;
}
void ControlingStringsTCP(const int con)
{
    /*
     防止服务器主机出现sigpipe信号的出现，造成服务器关闭，我们的设计就应该考虑的是用select函数防止服务器关闭出现。
     这样的情况是：一个进程向一个已收到RST的套接字上执行写操作内核就会发送一个sigpipe信号，终止信号。其捕获后应该做的是不加任何处理signal(sigpipe,sig_ign);返回的errno==EPiPE的
     */
    char sen[MAX],rec[MAX];
    int num,i,j;
    memset(sen, 0, MAX);
    memset(rec,0,MAX);
    while((num=(int)read(con, rec, MAX))>0)
    {
        puts("-------------TCP--------------");
        rec[num]='\0';
        fprintf(stdout, "recviev from client is :%s\n",rec);
        if(num>MAX)
            err_quit("too many data!");
        j=(int)strlen(rec);
        for(int k=0;k<j;k++)
        {
            if((rec[k]>='a'&&rec[k]<='z')||(rec[k]>='A'&&rec[k]<='Z'))
            {
                rec[k]+=4;
                if((rec[k]>'Z'&&rec[k]<='Z'+4)||(rec[k]>'z'))
                    rec[k]-=4;
            }
        }
        rec[j]='\0';
        for(i=0;i<j;i++)
            sen[j-i-1]=rec[i];
        sen[j]='\0';
         printf("send to client is %s\n",sen);
        getchar();
        write(con, sen, MAX);
       
    }
}
void ContorllingstringsUDP(const int udp,struct sockaddr * client,socklen_t len)
{
    char sen[MAX],rec[MAX];
    int num,i,j;
    memset(sen, 0, MAX);
    memset(rec,0,MAX);
    while((num=(int)recvfrom(udp, rec, MAX, 0, client, &len))>0)
    {
        puts("------------UDP-------------");
        printf("udp is ok ,length of udp is %d\n",client->sa_len);
        rec[num]='\0';
        //system(rec);
        j=(int)strlen(rec);
        if(strcmp(end, rec)==0)
            err_quit("server is ending!");
        if(num>MAX)
            err_quit("too many data!");
        if(strcmp(end, rec)==0)
        {
            puts("UDPserver is ending!");
            exit(0);
        }
        j=(int)strlen(rec);
        for(i=0;i<j;i++)
            sen[i]=rec[j-i-1];
        sen[j]='\0';
        sendto(udp, sen, MAX, 0, client, len);
        printf("send to client is %s\n",sen);
    }
}
void funct(int sign)
{
    int status;
    while(waitpid(-1, &status, WNOHANG) > 0);
    err_child_progress(status);
    return ;
}
int main(int argc, const char * argv[]) {
    // insert code here...
    int sockfd,conn,opt=1,udpf,maxfd,nready;
    struct sockaddr_in server,client;
    socklen_t len=sizeof(client);
    fd_set rset;
    __DARWIN_FD_ZERO(&rset);
    
    sockfd=Socket(AF_INET, SOCK_STREAM, 0);
    setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR , &opt, sizeof(opt));
    bzero(&server, sizeof(server));
    server.sin_family=AF_INET;
    server.sin_port=htons(PORT);
    server.sin_addr.s_addr=htonl(INADDR_ANY);
    Bind(sockfd, (struct sockaddr*)&server, sizeof(server));
    Listen(sockfd, LAN);
    udpf=Socket(AF_INET, SOCK_DGRAM, 0);
    bzero(&server, sizeof(server));
    server.sin_family=AF_INET;
    server.sin_port=htons(PORT);
    server.sin_addr.s_addr=htonl(INADDR_ANY);
    setsockopt(udpf,SOL_SOCKET,SO_REUSEADDR , &opt, sizeof(opt));
    Bind(udpf,(struct sockaddr*)&server, sizeof(server));
    signal(SIGCHLD, funct);
    maxfd=MAX_Variable(sockfd, udpf)+1;
    while(1)
    {
        __DARWIN_FD_SET(sockfd, &rset);
        __DARWIN_FD_SET(udpf, &rset);
        nready=select(maxfd, &rset, NULL, NULL, NULL);
        if(nready<0)
        {
            if(errno==EINTR)
                continue;
            else
                err_quit("select error");
        }
        if(__DARWIN_FD_ISSET(sockfd, &rset))
        {
         /*
          tcp mutilprocessProgram; parent ID means listen client,
          and child ID control the strings ,including recviev from client information and perversion dispose strings,do send to clients' message!
          */
            conn=accept(sockfd, (struct sockaddr*)&client, &len);
            if(conn<0)
            {
               if(errno==EINTR)
                   continue;
                /*
                 防止慢系统调用来将accept函数进行中断处理，其返回是有可能会出现一个EINTR错误回来，有可能系统内核会进行系统中断的调用。
                 我这样就可以用来捕获其返回的中断出错的错误时候，continue保证系统的继续运行
                 */
                else
                    err_quit("accept error!");
            }
            printf("family is TCP,Client is %s,ip is %d\n",inet_ntoa(client.sin_addr),ntohs(client.sin_port));
            pid_t pid=fork();
            if(pid==0)
            {
                close(sockfd);
                ControlingStringsTCP(conn);
                exit(0);
            }
            close(conn);
        }
        if(__DARWIN_FD_ISSET(udpf,&rset))
        {
            ContorllingstringsUDP(udpf, (struct sockaddr *)&client, len);
        }
    }
    return 0;
}
