#include <mm_malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <pthread.h>
#include <errno.h>
#include <unistd.h>
#include <sys/errno.h>
#include <string.h>
typedef struct sockaddr SA;
pthread_mutex_t lock=PTHREAD_MUTEX_INITIALIZER;
#define PORT 34560
#define MAX 100
#define BLOG 5
pthread_key_t key;
pthread_once_t once=PTHREAD_ONCE_INIT;
//作为key结构的值的参数构建，其含义是保证其只调用一次
//保证其创建调用只用一次
static void free_data_area(void * arg)
{
    free(arg);
}
static void Create_key_once(void)
{
    pthread_key_create(&key, free_data_area);
}
//创建key-析构函数结构，保证再进程结束后将数据进行释放；
struct ARG{
    int con;
    struct sockaddr_in client;
    char suff[MAX];
};
struct Index{
    int index;
};
//该函数用于打印其结果
void Fprintf_screen(const char *s)
{
    fprintf(stdout, "output:%s\n",s);
    return ;
}
//该函数用于出错打印
void Err_quit(const char *s)
{
    fprintf(stdout, "error:%s\n",s);
    exit(0);
}
/*
 socket、bind、listen 函数进行了出错处理问题；
*/
int Socket(const int sockfd,const int AF,const int pro)
{
    int flag=0;
    flag=socket(sockfd, AF, pro);
    if(flag<=0)
    {
        Err_quit(strerror(errno));
    }
    return flag;
}
void Bzero(struct sockaddr_in * addr)
{
    bzero(addr, sizeof(*addr));
    addr->sin_family=AF_INET;
    addr->sin_port=htons(PORT);
    addr->sin_addr.s_addr=htonl(INADDR_ANY);
}
int Bind(const int sockfd,SA * server,const socklen_t len)
{
    int flag=0;
    if(bind(sockfd, server, len)<0)
    {
        Err_quit(strerror(errno));
    }
    else
        flag=1;
    return flag;
}
int Listen(const int sockfd,const int blogs)
{
    int flag=0;
    flag=listen(sockfd, blogs);
    if(flag<0)
    {
         Err_quit(strerror(errno));
    }
    else
        flag=1;
    return flag;
}
//保存数据函数
void save_data(char *data,char * sava,const int len)
{
    struct Index *index;
    pthread_once(&once, Create_key_once);
    if((index=pthread_getspecific(key))==NULL)
    {
        index=calloc(1,sizeof(struct Index));
        pthread_setspecific(key, index);
        index->index=0;
    }
    for(int i=0;i<len;i++)
    {
        sava[index->index++]=data[i];
    }
    sava[index->index]='\0';
}
//处理字符串函数
void get_str(const int con,const struct sockaddr_in client)
{
    char sen[MAX],rec[MAX],sava[MAX];
    int num,j;
    struct sockaddr_in clients;
    socklen_t len=sizeof(clients);
    memset(sen, 0, MAX);
    memset(rec, 0, MAX);
    while(1)
    {
        num=(int)read(con, rec, MAX);
        rec[num]='\0';
        if(num<0)
            continue;
        getpeername(con,(struct sockaddr*) &clients, &len);
        printf("from %s:%d\n",inet_ntoa(clients.sin_addr),ntohs(clients.sin_port));
        j=(int)strlen(rec);
        save_data(rec, sava, j);
        if(strcmp(rec, "quit\n")==0)
        {
            break;
        }
        Fprintf_screen(rec);
        for(int i=0;i<j;i++)
        {
            if((rec[i]>='a'&&rec[i]<='z')||(rec[i]>='A'&&rec[i]<='Z'))
            {
                rec[i]+=4;
                if((rec[i]>'Z'&&rec[i]<='Z'+4)||(rec[i]>'z'))
                    rec[i]-=26;
            }

        }
        for(int i=0;i<j;i++)
        {
            sen[i]=rec[j-i-1];
        }
        sen[j]='\0';
       if(num!=write(con, sen, MAX))
       {
           Err_quit("send error!");
       }
        Fprintf_screen(sen);

    }
    fprintf(stdout,"output _ sava data is %s\n",sava);
    return ;
}

void * pthread_func(void * arg)
{
    struct ARG test=*(struct ARG *)arg;
    free(arg);
    get_str(test.con, test.client);
    close(test.con);
    return NULL;
}
int main(int argc, const char * argv[]) {
    // insert code here...
    pthread_t pid;
    struct ARG * ptr;
    int sockfd,con,opt=1,err;
    struct sockaddr_in server,client;
    socklen_t len=sizeof(client);
    sockfd=Socket(AF_INET, SOCK_STREAM, 0);
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    Bzero(&server);
    Bind(sockfd, (SA *)&server, len);
    Listen(sockfd, BLOG);
    while(1)
    {

        if((con=accept(sockfd, (SA* )&client, &len))<0)
        {
         if(errno==EINTR)
             continue;
            else
                Err_quit("error--accept!");
        }
        //这里利用互斥量来进行malloc数据的保护，malloc和fress是一个静态数据，不可重人函数，如果在创建的时候再去free，中断后不可以恢复其后果是非常严重的
        pthread_mutex_lock(&lock);
        ptr=malloc(sizeof(struct ARG));
        ptr->con=con;
        ptr->client=client;
        memset(ptr->suff, 0, MAX);
        pthread_mutex_unlock(&lock);
        if((err=pthread_create(&pid, NULL, pthread_func, ptr))>0)
        {
            Err_quit(strerror(err));
        }
    }
    //close(sockfd);
    return 0;
}
