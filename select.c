#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/select.h>
#include <time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#define  MAX 100
#define LAN 5
#define  PORT 34560
typedef struct Clinet{
    char data[MAX];
    struct sockaddr_in client;
    int fd;
}Client;
char Data[1000];
void err_quit(const char * s)
{
    puts(s);
    exit(1);
}
//处理函数--send to server的处理
void pro_str(Client c,char * rea,int len)
{
    char sen[MAX];
    int slen,j;
    rea[len]='\0';
    slen=(int)strlen(rea);
    printf("recvieve from client,ip is %s,the port is %d\n",inet_ntoa(c.client.sin_addr),ntohs(c.client.sin_port));
    printf("the information is %s",rea);
    memcpy(c.data, rea, len);
    for(int k=0;k<slen;k++)
    {
        if((sen[k]>='a'&&sen[k]<='z')||(sen[k]>='A'&&sen[k]<='Z'))
            {
                sen[k]+=4;
                if((sen[k]>'Z'&&sen[k]<='Z'+4)||(sen[k]>'z'))
                    sen[k]-=4;
            }
    }
    for(j=0;j<slen;j++)
        sen[j]=rea[slen-j-1];
    sen[slen]='\0';
    send(c.fd, sen,MAX, 0);
    printf("send information is %s\n",sen);
}
//保存数据的函数
void savedata(char * des,const char * res,const int len,int start)
{
    for(int i=0;i<len;++i)
        des[start+i]=res[i];
    des[start+len]='\0';
}
int main(int argc, const char * argv[]) {
    // insert code here...
    struct sockaddr_in server,clinet;
    Client cli[__DARWIN_FD_SETSIZE];
    fd_set rset,allset;
    char rea[MAX];
    int socked,num,conn,maxi,maxfd,i,nread,scok;
    socklen_t len=sizeof(clinet);
    bzero(&server, sizeof(server));
    memset(rea,0,MAX);
    server.sin_family=AF_INET;
    server.sin_port=htons(PORT);
    server.sin_addr.s_addr=htonl(INADDR_ANY);
    socked=socket(AF_INET, SOCK_STREAM, 0);
    if(socked==-1)
    {
        err_quit("socket error!");
    }
    if(bind(socked, (struct sockaddr*)&server, sizeof(server))==-1)
    {
        err_quit("bind error!");
    }
    if(listen(socked,LAN)==-1)
        err_quit("listen error");
    __DARWIN_FD_ZERO(&rset);
    __DARWIN_FD_ZERO(&allset);
    __DARWIN_FD_SET(socked, &allset);
    maxi=-1;
    maxfd=socked;
    //这里是对于描述字符的连接做初始化，这里是为了捕捉连接的描述字符
    for(i=0;i<__DARWIN_FD_SETSIZE;i++)
    {
        cli[i].fd=-1;
    }
    while(1)
    {
        rset=allset;
         nread=select(maxfd+1, &rset, NULL, NULL, NULL);
        if(nread==0)
        {
            if(errno!=EINPROGRESS)
                return -1;
        }
        if(__DARWIN_FD_ISSET(socked, &rset)){

        conn=accept(socked, (struct sockaddr *)&clinet, &len);
            if(conn<0)
            {
             if(errno==EINTR)
                 continue;
                else
                {
                    puts("error");
                    exit(0);
                }
            }
            for(i=0;i<__DARWIN_FD_SETSIZE;i++)
            if(cli[i].fd<0)
            {
                cli[i].fd=conn;
                cli[i].client=clinet;
                printf("the client's ip is %s,the id is %d\n",inet_ntoa(clinet.sin_addr),ntohs(clinet.sin_port));
                memset(cli[i].data, 0, MAX);
                break;
            }
            __DARWIN_FD_SET(conn, &allset);
            if(i>maxi)
                maxi=i;
            if(conn>maxfd)
                maxfd=conn;
            if(i==__DARWIN_FD_SETSIZE)
                err_quit("too many clients!");
            if(--nread<=0)
                continue;

        }
        for(i=0;i<=maxi;i++)
        {
            if((scok=cli[i].fd)<0)
                continue;
            if(__DARWIN_FD_ISSET(scok, &rset))
            {
                printf("test\n");
                if((num=(int)read(scok,rea,MAX))==0)
                {
                    close(scok);
                    printf("the client information %s\n",cli[i].data);
                    __DARWIN_FD_CLR(scok, &allset);
                    cli[i].fd=-1;
                }
                else{
                    int getlen=strlen(Data);
                    savedata(Data,rea,num,getlen);
                    if(strcmp(rea,"quit\n")==0)
                    {
                        printf("sava is %s\n",Data);
                    }
                    pro_str(cli[i], rea, MAX);
                }
                if(--nread<=0)
                    break;
             }
        }
    }
    return 0;
   }
