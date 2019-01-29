#include "stack.h"
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>

int main()
{
    int mypipe[2];
    pipe(mypipe);
    pid_t pid = fork();
    if(pid == 0) // server
    {
        create();
        int cmdOut = 0;
        while(cmdOut != -1){
            read(mypipe[0], &cmdOut, sizeof(cmdOut));
            push(cmdOut);
            printf("Out: %d\n", peek());
        }
    }
    else // client
    {
        int cmdIn = 0;
        while(cmdIn != -1)
        {
            scanf("%d", &cmdIn);
            write(mypipe[1], &cmdIn, sizeof(cmdIn));
        }
    }
    
    return 0;
}