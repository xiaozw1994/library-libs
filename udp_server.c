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
#define PORT 56789
void err_quit(const char *str)
{
   puts(str);
   exit(1);
}
const char * end="bye\n";
 int main(int argv,char * argc[])
 {
  struct sockaddr_in server,client;
  int sockfd,num,j,i;
  char sen[MAX],rea[MAX];
  socklen_t addrlen=sizeof(client);
  memset(sen,0,MAX);
  memset(rea,0,MAX);
  sockfd=socket(AF_INET,SOCK_DGRAM,0);
  if(sockfd==-1)
     err_quit("socket error!");
   bzero(&server,sizeof(server));
   server.sin_family=AF_INET;
   server.sin_port=htons(PORT);
   server.sin_addr.s_addr=htonl(INADDR_ANY);
  if(bind(sockfd, (struct sockaddr*)&server, sizeof(server))==-1)
  	err_quit("bind() error!");
  	while(1){
      num=recvfrom(sockfd,rea,MAX,0,(struct sockaddr *)&client,&addrlen);
      rea[num]='\0';
      if(num<=0||num>MAX)
      	err_quit("too many data");
      printf("information is from client,which its ip is %s,and port is %d\n",inet_ntoa(client.sin_addr),ntohs(client.sin_port));
      printf("information is from client ,which is %s\n",rea);
      if(strcmp(end,rea)==0)
      {
      	   printf("server is ending\n");
      	   break;
      }
      j=strlen(rea);
      for(i=0;i<j;i++)
 		sen[j-1-i]=rea[i];
 	  sen[j]='\0';
 	  sendto(sockfd,sen,MAX,0,(struct sockaddr*)&client,addrlen);
 	  printf("the sendto client  %s\n", sen);
  	}
  close(sockfd);
   return 0;
 }