#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>
#define MAX 100
#define PORT 55550
int main(int argc, const char * argv[]) {
    // insert code here...
    struct sockaddr_in server,client;
    int scokdf,num,index;
    char sen[MAX],rec[MAX];
    socklen_t len=sizeof(client);
    memset(sen, 0, MAX);
    memset(rec, 0, MAX);
    //初始化变量
    scokdf=socket(AF_INET, SOCK_DGRAM, 0);
    if(scokdf==-1)
    {
        puts("socket is error !");
        exit(0);
    }
    //创建socket函数，出错-1返回
    bzero(&server, sizeof(server));
    server.sin_family=AF_INET;
    server.sin_port=htons(PORT);
    server.sin_addr.s_addr=htonl(INADDR_ANY);
    //初始化server信息，其中ipv4 ip 端口，其中的IP是通配地址
    if(bind(scokdf, (struct sockaddr*)&server, sizeof(server))==-1)
    {
        puts("the bind fails");
        exit(0);
    }
    //绑定函数返回处理-1-出错
    puts("----------------UDP is already---------");
    while(1)
    {
        num=(int)recvfrom(scokdf, rec, MAX, 0, (struct sockaddr*)&client,&len );
        //接收数据的大小，其中的长度是在可控范围之内的
        if(num<=0||num>MAX)
        {
            puts("the data is excessive!");
            exit(0);
        }
        rec[num]='\0';
        printf("the information of client is whose ip is %s,and the port is %d\n",inet_ntoa(client.sin_addr),ntohs(client.sin_port));
        printf("the receive from the client is %s \n",rec);
        //打印来自客户端的信息，其中如果有bye出现，结束服务器
        if(strcmp(rec,"bye\n")==0)
        {
            puts("server ended!bye");
            break;
        }
        index=(int)strlen(rec);
        for(int i=0;i<index;i++)
        {
            sen[index-i-1]=rec[i];
        }
        sen[index]='\0';
        //这里处理进行字符串的颠倒
        printf("send to client is :%s\n",sen);
        sendto(scokdf, sen, MAX, 0, (struct sockaddr *)&client, len);
        //再给发送回客户端去
    }
    close(scokdf);
    return 0;
}
