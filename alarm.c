#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
int main(int argc,const char * argv[])
{
    int seconds;
    char line[128];
    char message[128];
    while(1)
     {
        printf("alarm>");
        if(fgets(line,sizeof(line),stdin)==NULL)
        	exit(0);
        if(strlen(line)<=1)
        	continue;
        if(sscanf(line,"%d %64[^/n]",&seconds,message)<2)
        	exit(0);
        else
        {
        	printf("alarm is beginning!\n");
        	sleep(seconds);
        	printf("(%d) ---%s \n",seconds,message );
        	break;
        }

     }
	return 0;
}