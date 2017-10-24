#include <pthread.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#define MAX 100
#define PORT 34560
#define LAN 5
struct infor{
    int conn;
    struct sockaddr_in client;
};
void err_quit(const char *s)
{
    puts(s);
    exit(1);
}
int Socket(int AF,int ap,int cp)
{
    int scokf=socket(AF, ap, cp);
    if(scokf==-1)
    {
        err_quit("socket error!");
    }
    return scokf;
}
void get_informa(int conn,struct sockaddr_in ap)
{
    char sen[MAX],rec[MAX],save[MAX];
    int num,len,j;
    memset(sen,0,MAX);
    memset(rec,0,MAX);
    memset(save, 0,MAX);
    while((num=(int)read(conn, rec, MAX))>0)
    {
        rec[num]='\0';
        len=(int)strlen(rec);
        if(strcmp(rec, "quit\n")==0)
            err_quit("the server is ending!");
        for(int i=0;i<len;i++)
        {
            if((rec[i]>='a'&&rec[i]<='z')||(rec[i]>='A'&&rec[i]<='Z'))
            {
                rec[i]+=4;
                if((rec[i]>'Z'&&rec[i]<='Z'+4)||(rec[i]>'z'))
                    rec[i]-=26;
            }
            
        }
        for(j=0;j<len;j++)
            sen[j]=rec[len-1-j];
        sen[j]='\0';
       // savadate(rec, num, save);
        printf("the recviev information is : %s\n",rec);
        send(conn, sen, MAX, 0);
        printf("send to the port client is %d,the information is :%s\n",ntohs(ap.sin_port),sen);
    }
    
}
void * function(void * arg)
{
    /*
        进入线程相关执行的函数，其函数的参数是void*可以试任意的，还有在该函数中的void*返回中还要注意一点，就是要结束其该线程。调用pthread_exit函数退出，或是结束线程
     */
    struct infor *ne=(struct infor*)arg;
    printf("from the information of client,ip is %s,and the port is %d\n",inet_ntoa(ne->client.sin_addr),ntohs(ne->client.sin_port));
    get_informa(ne->conn, ne->client);
    pthread_exit(NULL);
}

int main(int argc,char * argv[]) {
    // insert code here...
    int sockfd,conn,lag;
    struct sockaddr_in server,client;
    struct infor * arg;
    pthread_t pid;
    socklen_t len=sizeof(client);
    sockfd=Socket(AF_INET,SOCK_STREAM,0);
    server.sin_family=AF_INET;
    server.sin_port=htons(PORT);
    server.sin_addr.s_addr=htonl(INADDR_ANY);
    if(bind(sockfd, (struct sockaddr*)&server, sizeof(server))==-1)
        err_quit("bind(), error");
    if(listen(sockfd, LAN)==-1)
        err_quit("listen error!");
    while(1)
    {
        if((conn=accept(sockfd,(struct sockaddr*)&client,&len))<0)
           {
               if(errno==EINTR)
                   continue;
               else
               err_quit("connet error!");
           }
        arg=(struct infor *)malloc(sizeof(struct infor));
        arg->conn=conn;
        arg->client=client;
        if((lag=pthread_create(&pid, NULL, function, arg))>0)
        {
            if(lag==EAGAIN)
                err_quit("there are too many thread in the program!");
            else
                err_quit("create thread error!");
        }
        
    }
    return 0;
}
