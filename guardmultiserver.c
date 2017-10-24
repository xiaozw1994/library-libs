#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/select.h>
#include <time.h>
#include <unistd.h>
#include <sys/time.h>
#include <syslog.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
const char * end;
#define MAX 100
#define PORT 34561
#define LAN 8
#define OPEN_LINKER 56
void err_quit(const char *s)
{
    syslog(LOG_INFO|LOG_LOCAL3,"%s", s);
    exit(0);
}
void SyslogInfor(const char * s)
{
    syslog(LOG_INFO|LOG_LOCAL3,"%s", s);
    return ;
}
void err_child_progress(const int status)
{
    char buff[MAX];
    if(WIFEXITED(status))
    {
        err_quit("其终止状态是按照正常终止的\n");
        return;
    }
    else if(WIFSIGNALED(status))
    {
        snprintf(buff, MAX, "异常终止,其异常终止的ID是%d\n",WTERMSIG(status));
        err_quit(buff);
        return;
    }
    else if(WIFSTOPPED(status))
    {
        snprintf(buff, MAX, "当前暂停子进程,且子进程id是%d\n",WSTOPSIG(status));
        err_quit(buff);
        return ;
    }
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
void guard_structure(const char *panem,int facility)
{
    pid_t pid;
    if((pid==fork())>0)
        _exit(0);
    else if(pid<0)
        err_quit("创建进程失败！\n");
    if(setsid()<0)
        err_quit("创建session leader失败\n");
    signal(SIGHUP, SIG_IGN);
    //忽略信号
    /*
     这里你还要考虑一下就是一个问题，如果你在打开一个终端的话依然可能造成其终止进程
     */
    if((pid=fork())>0)
        _exit(0);
    else if(pid<0)
        err_quit("二次守护进程失败！\n");
    //改变工作的路径到根目录下
    chdir("/");
    umask(0);
    /*
     关闭文件描述符，防止有描述符的占用
     */
    for(int i=0;i<OPEN_LINKER;++i)
        close(i);
    
    open("/dev/null",O_RDONLY);
    open("/dev/null", O_WRONLY);
    open("/dev/null",O_RDWR);
    
    //纪录标注的输入输出
    openlog(panem, LOG_PID, facility);
    return ;
}
void sig_child(int signo)
{
    int status;
    pid_t pid;
    while((pid=waitpid(-1,&status,WNOHANG))>0);
    //err_child_progress(status);
    return ;
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
        rec[num]='\0';
        if(num>MAX)
            return ;
        j=strlen(rec);
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
        for(i=0;i<j;++i)
            sen[j-i-1]=rec[i];
        sen[j]='\0';
       // memcpy(sen,rec,sizeof(rec));
        write(con, sen, MAX);
    }
}
int main(int argc, char * argv[]) {
    // insert code here...
    int sockfd,num,con,i,j;
    pid_t pid;
    char buf[MAX];
    char sen[MAX],rec[MAX];
    time_t ti;
    struct sockaddr_in server,client;
    memset(sen, 0, MAX);
    memset(rec,0,MAX);
    guard_structure(argv[0], 0);
    socklen_t len=sizeof(client);
    memset(buf, 0, len);
    sockfd=Socket(AF_INET, SOCK_STREAM, 0);
    bzero(&server, sizeof(server));
    server.sin_family=AF_INET;
    server.sin_port=htons(PORT);
    server.sin_addr.s_addr=htonl(INADDR_ANY);
    Bind(sockfd, (struct sockaddr*)&server, sizeof(server));
    Listen(sockfd, LAN);
    signal(SIGCHLD,sig_child);
    while(1)
    {
       con=accept(sockfd, (struct sockaddr *)&client, &len);
       if(con<0)
        {
            if(errno==EINTR)
                continue;
            else
                err_quit("Description file description error\n");
        }
        /*
       ti=time(NULL);
       snprintf(buf,sizeof(buf),"conect is :%s\n",ctime(&ti));
       num=write(con,buf,sizeof(buf));
       if(num<0)
        err_quit("time error!");
       if((pid=fork())==0)
       {
            close(sockfd);
            ControlingStringsTCP(con);
            exit(0);
        }
        */
       if((pid=fork())==0)
       {
            close(sockfd);
            ControlingStringsTCP(con);
            exit(0);
       }
       close(con);
   }
    /*
    signal(SIGCHLD, sig_child);
    while(1)
    {
        if((con=accept(sockfd, (struct sockaddr *)&client, &len))<0)
        {
            if(errno==EINTR)
                continue;
            else
                err_quit("Description file description error\n");
        }
        if(con>0)
        {
            snprintf(buf, MAX, "from the client:%s:%d\n",inet_ntoa(client.sin_addr),ntohs(PORT));
            SyslogInfor(buf);
        }
        pid=fork();
        if(pid==0)
        {
            close(sockfd);
            ControlingStringsTCP(con);
            exit(0);
        }
        close(con);
    }
    */
    return 0;
}
