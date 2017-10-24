#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <time.h>
#include <netdb.h>
#define MAX 100
#define PORT 29911
typedef struct data{
  char suffer[MAX];
  struct tm*ti;
}data;

data setValue()
{
  time_t s;
  data temp;
  struct tm * sp;
  char da[MAX];
  time(&s);
  sp=localtime(&s);
  printf("the data is :");
  fgets(da,MAX,stdin);
  memcpy(da,temp.suffer,MAX);
  temp.ti=sp;
  return temp;
}

int main(int argc, const char * argv[]) {
    // insert code here...
    struct sockaddr_in server;
    int socked,num,j=0;
    struct data rec[MAX],sen[MAX];
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
        while(1)
        {
            sen[j++]=setValue();
            int s;
            printf("------1,contine,---2 enqeue");
            scanf("%d",&s);
            getchar();
            if(s==1)
                continue;
            else break;
        }
        write(socked, sen, MAX);
        //发送数据，并且发送的数据如果是quit，就结束连接
        for(int i=0;i<j;i++)
        {
            printf("send information is %s,send time is %s",sen[i].suffer,asctime(sen[i].ti));
            rec[i]=sen[i];
        }
        num=(int)read(socked, rec, 100);
        //接收数据，数据如果是正常的现象则就显示
        if(num<=0||num>MAX)
        {
            printf("the data is excessive!\n");
            exit(0);
        }
        
    }
    close(socked);
    //关闭连接。
    return 0;
}
