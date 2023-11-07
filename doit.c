/* doit.c */
/*Author = Krishna Garg*/

#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/resource.h>
#include <time.h>
#include <sys/time.h>
#include<string.h>
#include<unistd.h>
#include <stdbool.h>
void print_stats(int pid, struct timeval result);

int main(int argc, char *argv[])
{
    //If arguements are passed into the command line the program will run like normal
    if(argc > 1){
        struct timeval start, stop, final;

        gettimeofday(&start, NULL); //Getting the start time
        char* command = argv[1]; //command is always the first entry
        char* arg_list[argc];
        int pid;
        //This fills the arg_list
        for(int i = 1; i < argc; i++){
            arg_list[i-1] = argv[i];
        }
        arg_list[argc-1] = NULL; //has to end with NULL
        

        //When there is a problem with forking
        if ((pid = fork()) < 0) {
            fprintf(stderr, "There was a problem forking\n");
            exit(1);
        }

        //if forking works
        else if (pid == 0) {
            /* you are now in the child process */
            if (execvp(command, arg_list) == -1) {
                fprintf(stderr, "Execve error\n");
                exit(1);
            }
        }


        else {
            /* parent */
            wait(0); /* wait for the child to finish */
            gettimeofday(&stop, NULL);
            timersub(&stop, &start, &final);
            print_stats(pid, final);
        }
    }
    //end of the if arguements are passed into the command line

    //pseudo shell start
    if(argc == 1){
        char input[128];
        char init_prompt[128] = "==>";
        printf("%s", init_prompt);

        while(fgets(input, sizeof(input), stdin) != NULL){
                //this way it isn't terminated when nothing is put in
                if(strcmp(input, "\n") == 0){
                    goto printer;
                }
                //exitting
                if(strcmp(input, "exit\n") == 0){ //make sure you have the \n or else it won't work
                    exit(0);
                }
                //setting new prompt
                if(strncmp("set prompt = ", input, 13) == 0){
                    strncpy(init_prompt, " ", strlen(input)); //gets rid of whatevers in the current prompt
                    strncpy(init_prompt, &input[13], strlen(input) - 14); //replaces with input
                    goto printer; //I KNOW THIS IS EVIL HOWEVER IT MAKES IT PRETTIER PLEASE DON'T DOCK ME FOR THIS
                }

                //Otherwise you will have to execute the command and its arguements
                else{
                    struct timeval start, stop, final;
                    gettimeofday(&start, NULL); //Getting the start time
                    int pid;
                    input[strcspn(input, "\n")] = '\0'; //have to replace the new line character with a terminating character
                    char *words[32];
                    int j;
                    
                    //tokenize the string
                    words[0] = strtok(input, " ");
                    
                    for(int j = 1; j < 32; j++){
                        //then save the tokenized strings to an array
                        words[j] = strtok(NULL, " ");
                    }
                    
                    
                    
                    //changing directory
                    if(strcmp("cd", words[0]) == 0){
                        char cwd[500];
                        if(chdir(words[1]) != 0){
                            printf("cd: no such file or directory: %s\n", words[1]);
                        }
                        else{
                            getcwd(cwd, sizeof(cwd));
                            printf("directory changed to %s\n", cwd);
                        }
                        goto printer;
                    }
                    //end of special case of changing directory





                    //When there is a problem with forking
                    if ((pid = fork()) < 0) {
                        fprintf(stderr, "There was a problem forking\n");
                        exit(1);
                    }

                    //if forking works
                    else if (pid == 0) {
                        /* you are now in the child process */
                        if (execvp(words[0], words) == -1) {
                            fprintf(stderr, "Execve error\n");
                            exit(1);
                        }
                    }//if the fork was successful and we want to do other stuff
                    
                    else {
                        /* parent */
                        wait(0); /* wait for the child to finish */
                        gettimeofday(&stop, NULL);
                        timersub(&stop, &start, &final);
                        print_stats(pid, final);
                    }
                }//end of the else
                printer:
                    //This prints the prompt for input again
                    printf("%s", init_prompt);
                    //DO NOT DELETE
         }//end of the while loop
    }//end of main if statement
    //pseudo shell end
}//end of main





void print_stats(int pid, struct timeval result){
        struct rusage usage;
        getrusage (RUSAGE_CHILDREN, &usage);
        //printing the User and System CPU time used
        printf ("1. CPU user time: %ld milliseconds \n CPU system time: %ld milliseconds\n",
            ((usage.ru_utime.tv_sec * 1000) + ((usage.ru_utime.tv_usec / 1000))),
            ((usage.ru_stime.tv_sec * 1000) + ((usage.ru_stime.tv_usec / 1000))));
        //Printing the wall clock time
        printf("2. Wall Clock Time: %ld milliseconds\n", (result.tv_sec * 1000) + (result.tv_usec / 1000));
        
        printf("3. Number of Involuntary Context Switches: %ld\n", usage.ru_nivcsw);
        printf("4. Number of Voluntary Context Switches: %ld\n", usage.ru_nvcsw);
        
        printf("5. Number of Major Page Faults: %ld\n", usage.ru_majflt);
        printf("6. Number of Minor Page Faults: %ld\n", usage.ru_minflt);
        
        printf("7. Maximum Resident Set Size: %ld\n", usage.ru_maxrss);
}




/*
    ru_nvcsw (since Linux 2.6)
                The number of times a context switch resulted due to a
                process voluntarily giving up the processor before its
                time slice was completed (usually to await availability of
                a resource).

    ru_nivcsw (since Linux 2.6)
                The number of times a context switch resulted due to a
                higher priority process becoming runnable or because the
                current process exceeded its time slice.
    ru_minflt
                The number of page faults serviced without any I/O
                activity; here I/O activity is avoided by “reclaiming” a
                page frame from the list of pages awaiting reallocation.

    ru_majflt
                The number of page faults serviced that required I/O
                activity.

    ru_maxrss (since Linux 2.6.32)
                This is the maximum resident set size used (in kilobytes).
                For RUSAGE_CHILDREN, this is the resident set size of the
                largest child, not the maximum resident set size of the
                process tree.
*/