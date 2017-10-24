#include <stdio.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <netinet/in.h>

#define MAX 100
#define PORT 55550

int main(int argc, const char * argv[]) {
    // insert code here...
    struct sockaddr_in server,peer;
    socklen_t len=sizeof(server);
    struct hostent*hp;
    int sockdf,num;
    char sen[MAX],rec[MAX];
    memset(sen, 0, MAX);
    memset(rec, 0, MAX);
    //初始化suffer
    sockdf=socket(AF_INET,SOCK_DGRAM, 0);
    //socked 套接字连接创建
    if(argc>=3)//这里输入的变量的个数，这里是main函数来接受参数
    {
        puts("there are too many paraments!");
        exit(0);
    }
    hp=gethostbyname(argv[1]);
    //dns返回的hostent指针可以解析其中的IP等信息
    if(sockdf==-1)
    {
        //出错返回-1
        puts("the socket function creates failure");
        exit(0);
    }
    puts("----------------UDP is already---------");
    bzero(&server, sizeof(server));
    server.sin_family=AF_INET;
    server.sin_port=htons(PORT);
    //初始化server的连接，端口，IP，类型
    memcpy(&server.sin_addr, hp->h_addr_list[0], sizeof(hp));
    //这里将中的IP链表拷贝到server的ip地址里面
    while(1)
    {
        printf("send is :");
        if(fgets(sen, MAX, stdin)==NULL)
        {
            puts("error!");
            exit(0);
        }
        //发送数据到服务器，数据sen接收来自外部输入
        sendto(sockdf, sen, MAX, 0, (struct sockaddr*)&server, sizeof(server));
        if(strcmp(sen,"bye\n")==0)
        {
            puts("client ended!bye");
            break;
        }
        //如果是bye就结束
        printf("send to server is %s\n",sen);
        num=(int)recvfrom(sockdf, rec, MAX,0, (struct sockaddr*)&peer, &len);
        //接收来自服务区的数据长度，长度在可控范围内
        if(num<=0||num>MAX)
        {
            puts("the data is excessive!");
            exit(0);
        }
        /*
        if(len!=sizeof(server)||memcmp((const char *)&server, (const char *)&peer,len)!=0)
        {
            printf("send to message has some error,maybe it is from other server\n");
            continue;
        }
         //这里是进行判断其是否来自其他的服务器
         */
        rec[num]='\0';
        printf("from the server is :%s\n",rec);
        //打印在屏幕上
    }
    close(sockdf);
    return 0;
}
