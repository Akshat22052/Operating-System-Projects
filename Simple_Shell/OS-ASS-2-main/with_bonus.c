#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <fcntl.h>



// this is for storing history 
struct info *history[1000];
int cnt = -1;
int gv = 0;

// this is the structure for storing history
struct info
{
    int pid;
    char *start_time;
    char *end_time;
    double diff_time;
    char *command;
};

// this is for cleaning
void cleaning(char* commands[], struct info *history[],char input[])
{
    // freeing space for the commands array 
    for (int i = 0; i < 10; i++)
    {
        free(commands[i]);
         commands[i] = NULL;
    }
    // freeing space for the history global variable
    for (int i = 0; i <= cnt; i++)
    {
        free(history[i]->command);
        free(history[i]);
    }

    // freeing space for the input 
    free(input);
    // clearing the file 
    FILE *f;
    f = fopen("data.txt", "w");
    // error handling
    if (f == NULL) {
        printf("Error opening file");
        exit(2); 
    }
    fclose(f);    
}


// this function is to end the program
void terminate_program(int signal)
{
    printf("\n\n\n");
    FILE *f;
    f = fopen("data.txt", "r");
    // error handling
    if (f == NULL) {
        printf("Error opening file");   
        exit(2); 
    }
    char line[100]; 
    // printing the content of the file
    while (fgets(line, sizeof(line), f) != NULL) {
        printf("%s", line); 
    }
    // closing the file in read mode
    fclose(f); 
    // now opening it in reader's mode
    f = fopen("data.txt", "w");
    // error handling
    if (f == NULL) {
        printf("Error opening file");
        exit(2); 
    }
    fclose(f);
    printf("\n\nCtrl+C Pressed...\n");
    exit(0);
}

// this is for reading the input and separating it according to the need
void read_fn(char input[], char *commands[], int *f2)
{
    int flag = 0;
    *f2 = 0;
    int m = 0, n = 0, idx = 0;


    // Maximum length of a single command we are taking from the user
    int max_command_length = 1000;

    // i am separating the input by space 
    for (int i = 0; input[i] != '\0'; i++)
    {
        // Check for input length exceeding maximum
        // if (m >= 10 || n >= max_command_length)
        // {
        //     fprintf(stderr, "Input exceeded maximum length\n");
        //     exit(1); 
        // }


        if (input[i] == '-')
        {
            flag = 1;
        }
        if (input[i] == ' ' || input[i] == '\n')
        {
            commands[m][n] = '\0';
            n = 0;
            m++;
            if (strcmp(commands[0], "history") == 0)
            {
                idx = i;
                *f2 = 1;
                break;
            }
        }
        else
        {
            commands[m][n++] = input[i];
        }
    }
    if (*f2 == 0)
    {
    //    if (m < 10)
    //     {
            commands[m] = NULL;
        // }
        // else
        // {
        //     fprintf(stderr, "Memory allocation error for commands\n");
        //     exit(1); 
        // }
    }
    if (*f2 == 1)
    {
        int x = 0;
        for (int i = idx + 1; input[i] != '\0'; i++)
        {
            commands[1][x++] = input[i];
        }
        commands[1][x] = '\0';
        commands[2] = NULL;
    }
    // removing the \n from the input and making it as \0
    input[strlen(input) - 1] = '\0';
}



// removing new line character 
char* removing_new_line(char* str)
{
    int length = strlen(str);
    if (length > 0 && str[length - 1] == '\n') {
        str[length - 1] = '\0'; 
    }
    return str;
}

// this is the with the warning


// void updateHistory(int pid, char *input, char *command, char** commands, char **args /* ,char *xy */)
// {
//     FILE *file_pointer;

//     // opened the file 
//     file_pointer = fopen("data.txt", "a");


//     // error handling of file 
//     if (file_pointer == NULL)
//     {
//         perror("Error opening file");
//         return;
//     }

//     char optStr[20]; 

//     // converted pid in the form of string using 
//     snprintf(optStr, sizeof(optStr), "%d", pid);

//     // writing the command to file 
//     fprintf(file_pointer, "COMMAND: %s ", history[gv++]->command /* xy */);

//     // writing the pid to file
//     fprintf(file_pointer, "PID: %s", optStr);

//     // calculating the current time
//     time_t startTime1;
//     startTime1 = time(NULL);

//     // removing the /n character that automatically gets written by time function
//     char *x = removing_new_line(ctime(&startTime1));

//     // writing the start time to file 
//         fprintf(file_pointer, " START TIME: %s ", x);
    
//         fclose(file_pointer);


// //------------------------------------------------------------------
//     struct timespec st, endtm, elapstm;

//     // using clock_gettime function to calculate the time elapsed for the command
//     // Get the start time
//     clock_gettime(CLOCK_MONOTONIC, &st);
// //------------------------------------------------------------------



//     // creating a child process to run a parallel command to calculate its time
//     int rc = fork();

//     // error creating a child process
//     if (rc < 0)
//     {
//         perror("there is an error");
//         exit(1); 
//     }

//     // child process
//     else if (rc == 0)
//     {
        
//         // on commenting out this portion , we will not get the bin sh warnings. ( but with commenting this history is not printing properly)

//         //executing the execvp command and writing it to bin sh folder to avoid double print on terminal

        
//         char *arg[] = {"/bin/sh", "-c", commands ,NULL};

//         if (execvp("/bin/sh", arg ) == -1)
//         {
//             perror("execvp failed"); 
//             exit(1);
//         }
//     }
//     else
//     {
//         // opened the file 
//         file_pointer = fopen("data.txt", "a");


//         // error handling of file 
//         if (file_pointer == NULL)
//         {
//             perror("Error opening file");
//             return;
//         }

//         // waiting for the child process to get executed
//         int wc = wait(NULL);

//         // recording end time 
//         time_t endTime1;
//         endTime1 = time(NULL);

//         // removing the default \n character
//         char *x = removing_new_line(ctime(&endTime1));
//         // fprintf(file_pointer, " ENDTIME: %s ", x);
//         double diff_seconds = difftime(endTime1, startTime1);

// //-----------------------------------------------------------------------------

// // recording the end time of the timer
//         clock_gettime(CLOCK_MONOTONIC, &endtm);

//         // Calculate the elapsed time
//         if ((endtm.tv_nsec - st.tv_nsec) < 0) {
//             // Handle the case where the nanoseconds wrapped around
//             elapstm.tv_sec = endtm.tv_sec - st.tv_sec - 1;
//             elapstm.tv_nsec = 1000000000 + endtm.tv_nsec - st.tv_nsec;
//         } else {
//             elapstm.tv_sec = endtm.tv_sec - st.tv_sec;
//             elapstm.tv_nsec = endtm.tv_nsec - st.tv_nsec;
//         }

//         // Convert elapsed time to seconds with precision of 0.00001 seconds (10 microseconds)
//         double elapsed_seconds = elapstm.tv_sec + (double)elapstm.tv_nsec / 1000000000.0;


// //-----------------------------------------------------------------------------

//         // writing the elapsed_seconds to the file
//         fprintf(file_pointer, " EXECUTION TIME: %.5lf \n", elapsed_seconds);

//         fclose(file_pointer);
//     }
// }




// this function is without warning. 


void updateHistory(int pid, char *input, char *command, char** commands, char **args /* ,char *xy */)
{
    FILE *file_pointer;

    // opened the file 
    file_pointer = fopen("data.txt", "a");


    // error handling of file 
    if (file_pointer == NULL)
    {
        perror("Error opening file");
        return;
    }

    char optStr[20]; 

    // converted pid in the form of string using 
    snprintf(optStr, sizeof(optStr), "%d", pid);

    // writing the command to file 
    fprintf(file_pointer, "COMMAND: %s ", history[gv++]->command /* xy */);

    // writing the pid to file
    fprintf(file_pointer, "PID: %s", optStr);

    // calculating the current time
    time_t startTime1;
    startTime1 = time(NULL);

    // removing the /n character that automatically gets written by time function
    char *x = removing_new_line(ctime(&startTime1));

    // writing the start time to file 
        fprintf(file_pointer, " START TIME: %s ", x);
    
        fclose(file_pointer);


//------------------------------------------------------------------
    struct timespec st, endtm, elapstm;

    // using clock_gettime function to calculate the time elapsed for the command
    // Get the start time
    clock_gettime(CLOCK_MONOTONIC, &st);
//------------------------------------------------------------------



    // creating a child process to run a parallel command to calculate its time
    // int rc = fork();

    // // error creating a child process
    // if (rc < 0)
    // {
    //     perror("there is an error");
    //     exit(1); 
    // }

    // child process
    // else if (rc == 0)
    // {
        
        // on commenting out this portion , we will not get the bin sh warnings. ( but with commenting this history is not printing properly)

        //executing the execvp command and writing it to bin sh folder to avoid double print on terminal


        // char *arg[] = {"/bin/sh", "-c", commands ,NULL};

        // if (execvp("/bin/sh", arg ) == -1)
        // {
        //     perror("execvp failed"); 
        //     exit(1);
        // }
    // }
    // else
    // {
    //     // opened the file 
        file_pointer = fopen("data.txt", "a");


        // error handling of file 
        if (file_pointer == NULL)
        {
            perror("Error opening file");
            return;
        }

        // waiting for the child process to get executed
        // int wc = wait(NULL);

        // recording end time 
        time_t endTime1;
        endTime1 = time(NULL);

        // removing the default \n character
        char *y = removing_new_line(ctime(&endTime1));
        // fprintf(file_pointer, " ENDTIME: %s ", x);
        double diff_seconds = difftime(endTime1, startTime1);

//-----------------------------------------------------------------------------

// recording the end time of the timer
        clock_gettime(CLOCK_MONOTONIC, &endtm);

        // Calculate the elapsed time
        if ((endtm.tv_nsec - st.tv_nsec) < 0) {
            // Handle the case where the nanoseconds wrapped around
            elapstm.tv_sec = endtm.tv_sec - st.tv_sec - 1;
            elapstm.tv_nsec = 1000000000 + endtm.tv_nsec - st.tv_nsec;
        } else {
            elapstm.tv_sec = endtm.tv_sec - st.tv_sec;
            elapstm.tv_nsec = endtm.tv_nsec - st.tv_nsec;
        }

        // Convert elapsed time to seconds with precision of 0.00001 seconds (10 microseconds)
        double elapsed_seconds = elapstm.tv_sec + (double)elapstm.tv_nsec / 1000000000.0;


//-----------------------------------------------------------------------------

        // writing the elapsed_seconds to the file
        fprintf(file_pointer, " EXECUTION TIME: %.5lf \n", elapsed_seconds);

        fclose(file_pointer);
    }





void pipel(char input[])
{
    // firstly i am counting the number of instructions in the input
    int number_of_dande = 0;
    for (int i = 0; i < strlen(input); i++) {
        if (input[i] == '|') {
            number_of_dande++;
        }
    }

    int number_of_commands = number_of_dande + 1;

    // this is for breaking the input using strtok to separate the commands
    char *tk = strtok(input, "|");
    char *commands[100];
    int jk = 0;
    while (tk != NULL) {
        commands[jk++] = tk;
        tk = strtok(NULL, "|");
    }

    commands[jk] = NULL;

    // this command will work as shown == 
    // Input: cat helloworld.c | grep print | wc -l
    // Output: commands[0] = "cat helloworld.c", commands[1] = "grep print", commands[2] = "wc -l", commands[3] = NULL

    // Initializing the current pipe
    int current_pipe[2] = {0};  
    // Initializing the first pipe as stdin
    int previous_pipe[2] = {0};  
    

    for (int i = 0; i < number_of_commands; i++) {


        // error handling if the pipe does not formed
        if (pipe(current_pipe) == -1) {
            perror("pipe");
            return;
        }

        // creating the nrew process
        int pid = fork();
        if (pid < 0) {
            perror("fork");
            return;
        }

        if (pid==0) {
            // this is the Child process

            if (i < number_of_commands - 1) {
                // Redirect stdout to the current pipe's write end
                dup2(current_pipe[1], STDOUT_FILENO);
                close(current_pipe[0]);
                close(current_pipe[1]);
            }

            if (i > 0) {
                // Redirect stdin to the previous pipe's read end
                dup2(previous_pipe[0], STDIN_FILENO);
                close(previous_pipe[0]);
                close(previous_pipe[1]);
            }


            char *command = commands[i];
            char *args[100];
            int k = 0;

            // breaking the commands with spcae
            char *arg = strtok(command, " ");
            while (arg != NULL) {
                args[k++] = arg;
                arg = strtok(NULL, " ");
            }

            args[k] = NULL;
            execvp(args[0], args);
            perror("execvp");
            exit(1);
        } else {
            // this is the Parent process
            if (i > 0) {
                // Close the previous pipe's read and write ends in the parent
                close(previous_pipe[0]);
                close(previous_pipe[1]);
            }

            // Set the current pipe as the previous pipe for the next iteration
            previous_pipe[0] = current_pipe[0];
            previous_pipe[1] = current_pipe[1];
        }
    }

    // Close the last pipe's read and write ends in the parent
    close(current_pipe[0]);
    close(current_pipe[1]);

    // Waiting for all child processes to end before termination
    for (int i = 0; i < number_of_commands; i++) {
        wait(NULL);
    }
}



int main()
{
    char sample[5] = {'*','\n', '\0'};
    int MAX_LINES = 10;
    int MAX_LINE_LENGTH = 100;
    // printf("hi");
    int commandLineCnt = 0;
    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    char commandLines[MAX_LINES][MAX_LINE_LENGTH]; // Array of strings
    FILE *file12345;
    int line_count = 0;

    // Open the file for reading
    file12345 = fopen("input.sh", "r");
    if (file12345 == NULL) {
        perror("Error opening file");
        return 1;
    }

    // Read lines from the file and store them in the array
    while (fgets(commandLines[line_count], MAX_LINE_LENGTH, file12345) != NULL) {
        // Remove the newline character at the end of each line
        // printf("cwcce");
        // commandLines[line_count][strcspn(commandLines[line_count], "\n")] = '\0';
        line_count++;
    }
    // printf("%d", line_count);

    // Close the file
    fclose(file12345);
    // for (int i = 0; i < line_count-1; i++) {
    //     printf("Line %d: %s\n", i + 1, commandLines[i]);
    // }

    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    // this is the input char for taking the input (assuming it to be less than 1000 characters)
    char input[1000];
    // char x[1000];
    // this is for separating the commands
    char *commands[10];

    // this are for checking if there is piping or not in the command and hisotry pipe is for checking if the user has given history command
    int history_flag = 0,pipe_flag = 0;

    // assigning space to the commands
    for (int i = 0; i < 10; i++)
    {
        commands[i] = (char *)malloc(1000 * sizeof(char));
        // error handling
         if (commands[i] == NULL)
        {
            perror("Memory allocation for commands failed");
            return 1;
        }
    }

    int i = 0;
    int xyz = 0;

   do
    {   
        // cnt is for saving history 
        cnt++;
        if(xyz>=line_count)
        {
            printf("\n\n\n");
            FILE *f;
            f = fopen("data.txt", "r");
            // error handling
            if (f == NULL) {
                printf("Error opening file");   
                exit(2); 
            }
            char line[100]; 
            // printing the content of the file
            while (fgets(line, sizeof(line), f) != NULL) {
                printf("%s", line); 
            }
            // closing the file in read mode
            fclose(f); 
            // now opening it in reader's mode
            f = fopen("data.txt", "w");
            // error handling
            if (f == NULL) {
                printf("Error opening file");
                exit(2); 
            }
            fclose(f);
            // printf("hello there");
            return 4;
        }
        xyz++;
        // assigning space to the history array of structure
        history[cnt] = (struct info *)malloc(sizeof(struct info));
        if (history[cnt] == NULL)
        {
            // error handling
            perror("Memory allocation for history entry failed");
            return 1;
        }
        
        // printf("Command Shell ");
        // taking the input from the user using fgets
        
        // if (fgets(input, sizeof(input), stdin) == NULL) 
        // {
        // // error handling
        // perror("Error reading user input");

        // return 1;
        // }
        strcpy(input, commandLines[commandLineCnt]);
        commandLineCnt++;
        // strcpy(x,input);
        // x[strlen(x)-1] = '\0';

        
        // if (strcmp(input, sample) == 1)
        // {
        //     printf("akshat bhai yahan history print kardo");
        //     return 5;
        // }



        // storing the input in the history array 
        history[cnt]->command = strdup(input);
        history[cnt]->command[strlen(input)-1] = '\0';

        if (history[cnt]->command == NULL) 
        {
            perror("Memory allocation for command in history entry failed");
            // Handle the error, possibly by freeing previously allocated memory and exiting
            free(history[cnt]);
            return 1;
        }

        // this is fro separating commands and other error handlings
        read_fn(input, commands, &history_flag);
        // this loop is for checking if the pipeing is required or not 
        for (int i = 0;i<strlen(input);i++)
        {
            if (input[i]=='|')
            {
                pipe_flag = 1;
                break;
            }
        }

        int rc;
        int pd = 0;

        // error handling
        if (signal(SIGINT, terminate_program) == SIG_ERR)
        {
            perror("signal");
            return 1;
        }
        // if the piping is there then calling the pipel function to handle it 
        if (pipe_flag == 1)
        {
            pipel(input);
        }
        else
        {
            // calling the fork to create process
            rc = fork();
            if (rc < 0)
            {
                // error handling - if there is an error doing fork
                perror("There is an error ");
                exit(0);
            }
            else if (rc == 0)
            {
                // this is the child process it will execute the command
                int pc = getpid();
                // command is the command entered by the user
                char *command = commands[0];
                // args is for to pass it in the execvp system call.
                char *args[11];
                args[0] = command;
                // copying from the commands to the args
                for (int i = 0; i < 10 && commands[i + 1] != NULL; i++)
                {
                    args[i + 1] = commands[i + 1];
                }
                // calling execvp to execute the command and also error handling is done
                if (execvp(command, args) == -1)
                {
                    printf("Unknown command or incorrect usage: %s\n", command);
                    exit(1);
                }
            }
            else
            {
                // if (history_flag == 1)
                // {
                //     for (int i = 0; i <= cnt; i++)
                //     {
                //         printf("%s\n", history[i]->command);
                //     }
                // }
                int wc = wait(NULL);
                // error handling
                if (wc == -1)
                {
                    perror("Wait failed");
                }
            }
        }
        // if the user enter history then printing the history commands
         if (history_flag == 1)
                {
                    for (int i = 0; i <= cnt; i++)
                    {
                        printf("%s\n", history[i]->command);
                    }
                }
        // updating the history of the execution
        pd = rc; 
    
        updateHistory(pd, input, commands[0], commands, commands  /* ,x */);
        // again freeing the space for the commands for the next execution and also resseting the flags 
        history_flag = 0;
        for (int i = 0; i < 10; i++)
        {
            free(commands[i]);
            commands[i] = (char *)malloc(1000);
        }
    }
    while(1);
    // cleaning all the left out memory assigned
    cleaning(commands,history,input);
    return 0;
}


