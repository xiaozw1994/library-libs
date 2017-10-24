#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/shm.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <signal.h>
#include <errno.h>
#define PORT 34560
#define  MAX 100
const char * end="quit\n";
void information(const int acc,const struct sockaddr_in client)
{
    char buff[MAX];
    char recvi[MAX];
    int j,i;
    memset(buff, 0, sizeof(buff));
    memset(recvi, 0, sizeof(recvi));
    int num;
    while((num=(int)recv(acc, recvi, MAX, 0))>0)
    {
        recvi[num]='\0';
        printf("recvive is %s\n",recvi);
        if(strcmp(end,recvi)==0)
                exit(0);
        j=(int)strlen(recvi);
        /*
         这里还有一个办法，就是调用正则表达式处理字幕 {\w}--这就是表示有字母情况，头文件#include<regex.h>
        
        */
        for(int k=0;k<j;k++)
        {
            if((recvi[k]>='a'&&recvi[k]<='z')||(recvi[k]>='A'&&recvi[k]<='Z'))
            {
                recvi[k]+=4;
                if((recvi[k]>'Z'&&recvi[k]<='Z'+4)||(recvi[k]>'z'))
                    recvi[k]-=4;
            }
        }
        recvi[j]='\0';
        for(i=0;i<j;i++)
            buff[j-i-1]=recvi[i];
        buff[j]='\0';
        printf("send is %s\n",buff);
        send(acc, buff, MAX, 0);
    }
}
void child_sig(int sign)
{
    int status;
    while(waitpid(-1, &status, WNOHANG)>0);
    return ;
}
int main(int argc, const char * argv[]) {
    // insert code here...
    int socked=socket(AF_INET, SOCK_STREAM, 0);
    int opt=1;
    if(socked==-1)
    {
        printf("socket() error\n");
        return 0;
    }
    struct sockaddr_in server,client;
    setsockopt(socked,SOL_SOCKET,SO_REUSEADDR , &opt, sizeof(opt));
    bzero(&server,sizeof(server));
    server.sin_family=AF_INET;
    server.sin_port=htons(PORT);
    server.sin_addr.s_addr=htonl(INADDR_ANY);
    if(bind(socked, (struct sockaddr*)&server, sizeof(server))==-1)
    {
        printf("bind() error\n");
        return 0;
    }
    if(listen(socked, 5)==-1)
    {
        printf("listen error\n");
        return 0;
    }
    int conned;
    pid_t pid;
    socklen_t len=sizeof(client);
    signal(SIGCHLD, child_sig);
    //捕捉函数
    while(1)
    {
        if((conned=(int)accept(socked, (struct sockaddr *)&client, &len))<0)
        {
            /*
             防止慢系统调用来将accept函数进行中断处理，其返回是有可能会出现一个EINTR错误回来，有可能系统内核会进行系统中断的调用。
             我这样就可以用来捕获其返回的中断出错的错误时候，continue保证系统的继续运行
             */
            if(errno==EINTR)
                continue;
            else{
            printf("accept is errorr");
            return 0;
            }
        }
        printf("family is TCP,Client is %s,ip is %d\n",inet_ntoa(client.sin_addr),ntohs(client.sin_port));
        if((pid=fork())>0)
        {
            close(conned);
            continue;
        }
        else if(pid==0)
        {
            close(socked);
            information(conned, client);
            exit(0);
        }
        /*
         子进程结束返回其状态等信息，文件描述符等，其系统并没有进行释放，依然占据其相应的资源，我们的目的就是要进行捕捉其子进程最终结束的状态等信息，捕获函式wait等到其子进程完全释放以后就可以在再进行accept的阻塞。监听信息
         */
        else
        {
            printf("error");
            return 0;
        }
    }
    return 0;
}
