//
//  main.c
//  processControl
//
//  Created by 肖智文 on 2017/9/27.
//  Copyright © 2017年 肖智文. All rights reserved.
//
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <stdlib.h>
#include <signal.h>
#define MAX 5
static int cout=0;
static int arr[MAX];
void child_exit(int sign)
{
    int status;
    while(waitpid(-1,&status,WNOHANG)>0);
    return ;
}
/*
 生产者消费者模型设置实现，vfork的原因数据共享，其实要等到其生产者将产品生产完成以后才可以将数据继续给消费者使用
 */
void Coustuemer()
{
    pid_t pid;
    pid=vfork();
    signal(SIGCHLD,child_exit );
    if(pid==0)
    {
        if(cout==0)
        for(int i=0;i<MAX;i++){
            arr[i]=i+1;
            cout++;
        }
        exit(0);
    }
    if(cout==MAX)
        return;
    else
        _exit(0);
}
void product(void)
{
    pid_t pid;
    pid=vfork();
    signal(SIGCHLD, child_exit);
    if(pid==0)
    {
        if(cout==MAX)
            for(int i=0;i<MAX;i++)
            {
                getchar();
                printf("output is %d\n",arr[i]);
                cout--;
            }
        exit(0);
    }
    getchar();
    if(cout==0)
        return ;
    else
        _exit(0);
}

int main(int argc, const char * argv[]) {
    // insert code here...
    Coustuemer();
    product();
 
    return 0;
}
