// Yael Simhis - 209009604

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX_LEN 100

typedef struct {
    pid_t pid;
    char historyCommand[MAX_LEN];
} History;

//there will be no more then 100 commands
History historyArray[MAX_LEN];
int arrayIndex = 0;

//
// print all the history or all the jobs
//
void printAll(int flag) {
    int i, pidChild, running = 0;
    // only jobs
    if (flag == 0) {
        for (i = 0; i < arrayIndex; i++) {
            pid_t checkPid = waitpid(historyArray[i].pid, &pidChild, WNOHANG);
            // if the process running - print
            if (checkPid == 0) {
                printf("%d ", historyArray[i].pid);
                printf("%s ", historyArray[i].historyCommand);
                printf("\n");
            }
        }
    } else {
        // print all history
        for (i = 0; i < arrayIndex; i++) {
            pid_t checkPid = waitpid(historyArray[i].pid, &pidChild, WNOHANG);
            // if the process running - print
            if (checkPid == 0) {
                //print with "RUNNING"
                printf("%d ", historyArray[i].pid);
                printf("%s ", historyArray[i].historyCommand);
                printf("RUNNING\n");
            } else {
                //print with "DONE"
                printf("%d ", historyArray[i].pid);
                printf("%s ", historyArray[i].historyCommand);
                // last history print as running
                if ((strcmp(historyArray[i].historyCommand, "history") == 0) && (i == (arrayIndex - 1))) {
                    printf("RUNNING\n");
                } else {
                    printf("DONE\n");
                }
            }
        }
    }
}

//
// parse the buffer to command array
//
int fromStrToArr(char *buf, char **command) {
    char *token = NULL;
    int i = 0, flag = 0, len = 0;
    // separate by spaces and check background
    token = strtok(buf, " ");
    while (token != NULL) {
        // check if need to run in background
        if (strcmp(token, "&") == 0) {
            flag = 1;
            // no need to insert to the command array
            break;
        }
        command[i] = token;
        // save the command to history array
        strcpy(&(historyArray[arrayIndex].historyCommand[len]), token);
        len += strlen(token);
        token = strtok(NULL, " ");
        // space
        if (token != NULL) {
            strcpy(&(historyArray[arrayIndex].historyCommand[len]), " ");
            len++;
        }
        i++;
    }
    return flag;
}

//
// the main of the project
//
int main() {
    pid_t pid;
    int check, childPid, backgroundFlag;
    char prevPath[MAX_LEN];
    //initialize
    getcwd(prevPath, sizeof(prevPath));
    while (1) {
        //initialize
        char buffer[MAX_LEN] = {};
        char *command[MAX_LEN] = {};
        // prompt
        printf("> ");

        // get the command
        fgets(buffer, MAX_LEN, stdin);

        //remove the end line
        if (buffer[strlen(buffer) - 1] == '\n') {
            buffer[strlen(buffer) - 1] = '\0';
        }

        //parse the buffer, and check background
        backgroundFlag = fromStrToArr(buffer, command);

        if (strcmp(command[0], "exit") == 0) {
            //print the pid and exit
            printf("%d \n", getpid());
            break;
        } else if (strcmp(command[0], "cd") == 0) {
            // add pid to history array
            historyArray[arrayIndex].pid = getpid();
            arrayIndex++;
            printf("%d \n", getpid());
            // more then one argument it is an error
            if (command[2] != NULL) {
                fprintf(stderr , "Too many arguments\n");
            }
            if (strcmp(command[1], "~") == 0) {
                // save prevPath
                getcwd(prevPath, sizeof(prevPath));
                // default
                chdir(getenv("HOME"));
            } else if (strcmp(command[1], "-") == 0) {
                // go back to the previous library
                chdir(prevPath);
                // save prevPath
                getcwd(prevPath, sizeof(prevPath));
            } else if (strcmp(command[1], "..") == 0) {
                // save prevPath
                getcwd(prevPath, sizeof(prevPath));
                // inside library
                check = chdir(command[1]);
                //check if directory exist
                if (check == -1) {
                    fprintf(stderr, "Error: No such file or directory\n");
                }
            } else {
                // save prevPath
                getcwd(prevPath, sizeof(prevPath));
                // directory
                check = chdir(command[1]);
                //check if directory exist
                if (check == -1) {
                    fprintf(stderr, "Error: No such file or directory\n");
                }
            }

        } else if (strcmp(command[0], "jobs") == 0) {
            // add pid to history array
            historyArray[arrayIndex].pid = getpid();
            arrayIndex++;
            //print all the jobs
            printAll(0);
        } else if (strcmp(command[0], "history") == 0) {
            // add pid to history array
            historyArray[arrayIndex].pid = getpid();
            arrayIndex++;
            //print all the history
            printAll(1);
        } else {
            // if the command is echo need to take off the " "
            if(strcmp(command[0], "echo") == 0) {
                char *firstToken = NULL, *lastToken = NULL;
                //check the len of command array
                int len = 1;
                while (command[len] != NULL) {
                    len++;
                }
                // separate by spaces and check background
                firstToken = strtok(command[1], "\"\"");
                strcpy(command[1], firstToken);
                //remove " in the last word
                if (len > 2) {
                    lastToken = strtok(command[len - 1], "\"\"");
                    strcpy(command[len - 1], lastToken);
                }
            }
            pid = fork();
            if (pid < 0) {
                printf("fail in fork\n");
            } else if (pid == 0) {
                // the child process
                check = execvp(command[0], command);
                if (check < 0) {
                    fprintf(stderr, "Error in system call\n");
                }
            } else {
                // add pid to history array
                historyArray[arrayIndex].pid = pid;
                arrayIndex++;
                // the parent process
                printf("%d\n", pid);
                // if not runs in background - wait for child
                if (backgroundFlag == 0) {
                    waitpid(pid, &childPid, 0);
                }
            }
        }
    }
    return 0;
}