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
#define PORT 34560
void err_quit(const char *str)
{
   puts(str);
   exit(1);
}
const char * end="quit\n";
int main(int argc,char * argv[])
{
  struct hostent *hp;
  struct sockaddr_in client,server;
  int sockfd,num;
  char sen[MAX],rea[MAX];
  socklen_t addrlen=sizeof(server);
  if(argc!=2)
  {
  	err_quit("too many variables ");
  	exit(1);
  }
  if((hp=gethostbyname(argv[1]))==NULL)
  	 err_quit("error!");

  bzero(&server,sizeof(server));
  server.sin_family=AF_INET;
  server.sin_port=htons(PORT);
  memcpy(&server.sin_addr,hp->h_addr_list[0],sizeof(hp));
  sockfd=socket(AF_INET,SOCK_DGRAM,0);
  if(sockfd==-1)
  	err_quit("socket err_quit");
  while(1)
  {
  	printf("send:");
    if(fgets(sen,MAX,stdin)==NULL)
       err_quit("input err_quit");
    sendto(sockfd,sen,MAX,0,(struct sockaddr*)&server,sizeof(server));
    printf("send to server ---%s\n",sen);
    if(strcmp(end,sen)==0)
    {
    	printf("client is over!\n");
    	break;
    }
    num=recvfrom(sockfd,rea,MAX,0,(struct sockaddr*)&client,&addrlen);
    rea[num]='\0';
    if(num<=0||num>MAX)
    {
    	err_quit("error");
    }
    printf("from server  %s\n", rea);

  }
close(sockfd);
return 0;
}