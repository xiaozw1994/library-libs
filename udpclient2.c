#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <netdb.h>
#define MAX 100
#define PORT 45670
int main(int argc, const char * argv[]) {
    // insert code here...
    struct sockaddr_in server;
    int socked,num;
    char sen[MAX],rec[MAX];
    const char *end="quit\n";
    struct hostent * hp;
    hp=gethostbyname(argv[1]);
    //hostent利用DNS来获取其中信息
    socked=socket(AF_INET, SOCK_STREAM, 0);
    if(socked==-1)
    {
        puts("socket is error\n");
        exit(0);
    }
    //socket函数其中出错则为-1，创建一个套接字
    if(argc==3)
    {
        puts("there are too many paraments\n");
        exit(0);
    }
    bzero(&server, sizeof(server));
    server.sin_family=AF_INET;
    server.sin_port=htons(PORT);
    memcpy(&server.sin_addr, hp->h_addr_list[0], sizeof(hp));
    /*
     初始化其中的server信息，还有需要设置server的服务类型，服务的端口，服务的ip地址
     */
    if(connect(socked, (struct sockaddr *)&server, sizeof(server))==-1)
    {
        puts("error!\n");
        exit(0);
    }
    puts("--------TCP is already!-----------");
    //connect函数返回-1，出错
    while(1)
    {
        printf("the send is :");
        if(fgets(sen, 100, stdin)==NULL)
            break;
        write(socked, sen, 100);
        //发送数据，并且发送的数据如果是quit，就结束连接
        if(strcmp(sen, end)==0)
        {
            puts("quit,close the connection.\n");
            break;
        }
        printf("send to server is %s\n",sen);
        num=(int)read(socked, rec, 100);
        rec[num]='\0';
        //接收数据，数据如果是正常的现象则就显示
        if(num<=0||num>MAX)
        {
            printf("the data is excessive!\n");
            exit(0);
        }
        printf("from the server is %s\n",rec);
    }
    close(socked);
    //关闭连接。
    return 0;
}