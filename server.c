#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#define PORT 29911
#define MAX 100
#define LAN 5
typedef struct data{
  char suffer[MAX];
  struct tm*ti;
}data;
int main(int argc, char * argv[]) {
    // insert code here...
    struct sockaddr_in server,client;
    int sockfd;
    int conn,num;
    int j;
    const char *end="quit\n";
    data su_read[MAX],se_buff[MAX];
    
    //初始化buffer
    

    socklen_t len=sizeof(client);
    sockfd=socket(AF_INET,SOCK_STREAM,0);
    //创建socket连接的套接字，返回-1表示为连接错误。
    if(sockfd==-1)
    {
        puts("socket error!\n");
        exit(0);
    }

    bzero(&server, sizeof(server));
    server.sin_family=AF_INET;
    server.sin_port=htons(PORT);
    server.sin_addr.s_addr=htonl(INADDR_ANY);
    /*
     连接的服务器的服务类型ipv4，端口号，通配地址
     */
    if(bind(sockfd, (struct sockaddr*)&server, sizeof(server))==-1)
    {
        printf("the bind is error!\n");
        exit(0);
    }
    /*
     bind绑定函数出错返回的是-1，listen监听函数出错返回的是-1.这里都是被动连接部分。
     
     */
    if(listen(sockfd, LAN)==-1)
    {
        puts("listen is error\n");
        exit(0);
    }
    conn=accept(sockfd, (struct sockaddr*)&client, &len);
    if(conn==-1)
    {
        puts("error!\n");
        exit(0);
    }
    /*
     accept函数以后将被动连接转为主动坚挺，其中有来自客户端的信息，其中出错返回的是-1；
     
     */
    puts("--------TCP is already!-----------");
    while(1)
    {
        num=(int)read(conn, su_read, MAX);
        
        printf("the ip of client is %s,the port is %d\n",inet_ntoa(client.sin_addr),ntohs(client.sin_port));
        
        //接收来自客户端的数据，并且打印客户端的ip和端口号
        if(num<=0||num>MAX)
        {
            printf("the data is excessive!\n");
            exit(0);
        }
        //接收非法的长度，超过了其缓存区的大小,当有quit时候就会退出
        
        /*
         进行字符串的颠倒实现
         */
        for(j=0;j<num;j++){
           se_buff[j]=su_read[j];
         if(strcmp(se_buff[j].suffer," ")!=0)  
        printf("send information is %s,send time is %s",se_buff[j].suffer,asctime(se_buff[j].ti));
    }
        write(conn, se_buff, MAX);
    }
    close(conn);
    close(sockfd);
    //关闭连接，关闭套接字
    return 0;
}
