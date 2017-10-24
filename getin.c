#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
int main(int argc,const char *argv[])
{
 // ulong_t addr;
  struct hostent  * hp;
  char **p;  
  if(argc!=2)
    return 0;
  hp=gethostbyname(argv[1]);
  if(hp==NULL)
  {
 
   printf("error1\n");
   return 0;


}
   for(p=hp->h_addr_list;*p!=0;p++)
    {
    struct in_addr in;
    char **q;
   memcpy(&in.s_addr,*p,sizeof(in.s_addr));
   printf("the ip : %s\t",inet_ntoa(in));
   for(q=hp->h_aliases;*q!=0;q++) 
       printf("address: %s\n",*q);
}

  return 0;
}
