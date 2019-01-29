#include "stack.h"
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>

int main()
{
    int mypipe[2];
    pipe(mypipe);
    printf("Write exit to exit\n");
    pid_t pid = fork();
    
    if(pid == 0) // server
    {
        create();
        char cmdOut[80];
        do
        {
            read(mypipe[0], cmdOut, sizeof(cmdOut));
            
            char cmd[80];
            memcpy(cmd, &cmdOut, strlen(cmdOut) + 1); 
            
            const char delimeter[] = "(";
            strtok(cmdOut, delimeter);

            if (strcmp(cmdOut, "peek") == 0)
            {
                printf("Server say: %d\n", peek());
            }
            else if (strcmp(cmdOut, "push") == 0)
            {
                char* data_raw = malloc(10);
                strncpy(data_raw, cmd + strlen(cmdOut) + 1, strlen(cmd) - strlen(cmdOut) - 2);
                push(atoi(data_raw));
                printf("Server say: Pushed\n");
            }
            else if (strcmp(cmdOut, "pop") == 0)
            {
                pop();
                printf("Server say: Poped\n");
            }
            else if (strcmp(cmdOut, "empty") == 0)
            {
                printf("Server say: %d\n", empty());
            }
            else if (strcmp(cmdOut, "display") == 0)
            {
                printf("Server say:\n");
                display();
            }
            else if (strcmp(cmdOut, "create") == 0)
            {
                create();
                printf("Server say: Done\n");
            }
            else if (strcmp(cmdOut, "stack_size") == 0)
            {   
                printf("Server say: ");
                stack_size();
            }
            
        } while(strcmp(cmdOut, "exit") != 0);
    }
    else // client
    {
        char cmdIn[80];
        do
        {
            scanf("%s", cmdIn);
            write(mypipe[1], &cmdIn, strlen(cmdIn) + 1);
        } while(strcmp(cmdIn, "exit") != 0);
    }
    
    return 0;
}