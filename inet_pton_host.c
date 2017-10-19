#include "ping.h"
/*
返回的点分十进制的ip地址
*/
char * inet_ntop_host(const struct sockaddr * sa,socklen_t len)
{
   char post[8];
   static char str[128];
   memset(post,0,sizeof(post));
   memset(str,0,sizeof(str));
   switch(sa->sa_family)
   {
        case AF_INET:
        {
            struct sockaddr_in * sofd=(struct sockaddr_in * )sa;
            if(inet_ntop(AF_INET,&sofd->sin_addr,str,sizeof(str))==NULL)
            {
            	return NULL;
            }
            else
            	return str;
            break;
        }
        case AF_INET6:
        {
           struct sockaddr_in6 * sf=(struct sockaddr_in6 *)sa;
           if(inet_ntop(AF_INET6,&sf->sin6_addr,str,sizeof(str))==NULL)
           {
           	return NULL;
           }
           else
           	return str;
          break;
        }
        default:
        break;
   }
   return NULL;
}
/*
      
*/
struct addrinfo * host_serv(const char *host,const char * serv,int family,int socktype)
{
	int n;
	struct addrinfo hints,*res;
	bzero(&hints,sizeof(hints));
	hints.ai_flags=AI_CANONNAME;
	hints.ai_family=family;
	hints.ai_socktype=socktype;
	if((n=getaddrinfo(host,serv,&hints,&res))!=0)
	{
		return NULL;
	}
	return res;
}
