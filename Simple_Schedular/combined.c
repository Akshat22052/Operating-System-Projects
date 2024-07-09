#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <sys/time.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <time.h>



//---------------Declaring Shared Memory -------------------//
#define max_processes 100
#define maximum_command_length 1024
#define SHM_NAME "/my_shared_memory"
#define SHM_NAME2 "/my_shared_memory2"
#define SHM_NAME3 "/my_shared_memory3"
#define SHM_NAME4 "/my_shared_memory4"
#define SHM_NAME5 "/my_shared_memory5"
#define SHM_NAME6 "/my_shared_memory6"
#define SHM_NAME7 "/my_shared_memory7"




//---------------Declaring global variable and pointers-------------------//
struct Process* process_queue;
struct Process* processes;
int *queue_front_shared;
int *queue_rear_shared;
int *flag;
int NCPUS = 0;
int TSLICE = 0;
float *avg_execution_time = 0;
float *avg_wait_time = 0;
int num = 0;
int queue_front = -1;
int queue_rear = -1;
int queue_front_two = -1;
int queue_rear_two = -1;
int time_slice = 100;
int process_count = 0;

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

//---------------Structure of user defined Process--------------//
struct Process
{
    int process_state;
    int pid;
    double execution_time;
    double wait_time;
    int termination_status;
    char command[100];
    int priority;
    double start_time;
    double end_time;
};
void displayProcessesQueue() {
    // if (queue_front_two == -1) {
    //     printf("Queue is empty\n");
    //     return;
    // }

    printf("Processes History:\n----------------------------\n");
    int current_index = queue_front_two;

    while (current_index != (queue_rear_two + 1) % max_processes) {
        struct Process process_id = processes[current_index];
        printf("PID: %d\n", process_id.pid);
        printf("Process State: %d\n", process_id.process_state);
        printf("Execution Time: %.2lf seconds\n", process_id.execution_time);
        printf("Wait Time: %.2lf seconds\n", process_id.wait_time);
        printf("Command: %s\n", process_id.command);
        printf("Priority: %d\n", process_id.priority);
        printf("Start Time: %.2lf seconds\n", process_id.start_time);
        printf("End Time: %.2lf seconds\n", process_id.end_time);
        printf("--------------------------------------------\n\n");
        current_index = (current_index + 1) % max_processes;
    }
}

void swap(int i, int j)
{
    struct Process t = process_queue[i];
    process_queue[i] = process_queue[j];
    process_queue[j] = t;
}
//---------------Function to sleep for t milliseconds for accuracy--------------//
void sleep_ms(int milliseconds) {
    struct timespec ts;
    ts.tv_sec = milliseconds / 1000;
    ts.tv_nsec = (milliseconds % 1000) * 1000000;

    while (nanosleep(&ts, &ts) == -1)
        continue;
}
long long current_time_millis() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (long long)tv.tv_sec * 1000 + tv.tv_usec / 1000;
}
//---------------Enqueuing in process_queue--------------//
void enqueue(struct Process process_id) {
    if ((queue_rear + 1) % max_processes == queue_front) {
        fprintf(stderr, "Queue is full\n");
        exit(EXIT_FAILURE);
    }
    if (queue_front == -1) {
        queue_front = 0;
        queue_rear = 0;
    } else {
        queue_rear = (queue_rear + 1) % max_processes;
    }
    process_queue[queue_rear] = process_id;
    // printf("Inside function %d %d \n",queue_front, queue_rear);
    *queue_front_shared = queue_front;
    *queue_rear_shared = queue_rear;

}
//---------------Dequeueing in process_queue--------------//
struct Process dequeue() {
    if (queue_front == -1) {
        fprintf(stderr, "Queue is empty\n");
        exit(EXIT_FAILURE);
    }

    struct Process process_id = process_queue[queue_front];

    if (queue_front == queue_rear) {
        queue_front = -1;
        queue_rear = -1;
    } else {
        queue_front = (queue_front + 1) % max_processes;
    }
    *queue_front_shared = queue_front;
    *queue_rear_shared = queue_rear;

    return process_id;
}
//---------------Enqueuing in process_queue--------------//
void enqueuenew(struct Process process_id) {
    if ((queue_rear + 1) % max_processes == queue_front) {
        fprintf(stderr, "Queue is full\n");
        exit(EXIT_FAILURE);
    }
    if (queue_front == -1) {
        queue_front = 0;
        queue_rear = 0;
    } else {
        queue_rear = (queue_rear + 1) % max_processes;
    }
    process_queue[queue_rear] = process_id;
    // printf("Inside function %d %d \n",queue_front, queue_rear);
    *queue_front_shared = queue_front;
    *queue_rear_shared = queue_rear;

    int t = queue_rear;
    while (t > queue_front && process_queue[t - 1].priority > process_queue[t].priority) {
        swap(t, t - 1);
        t--;
    }
}
//---------------Dequeueing in process_queue--------------//
struct Process dequeuenew() {
    if (queue_front == -1) {
        fprintf(stderr, "Queue is empty\n");
        exit(EXIT_FAILURE);
    }

    struct Process process_id = process_queue[queue_front];

    if (queue_front == queue_rear) {
        queue_front = -1;
        queue_rear = -1;
    } else {
        queue_front = (queue_front + 1) % max_processes;
    }
    *queue_front_shared = queue_front;
    *queue_rear_shared = queue_rear;

    return process_id;
}
//---------------Equeueing in processes--------------//
void enqueue2(struct Process process_id) {
    if ((queue_rear_two + 1) % max_processes == queue_front_two) {
        fprintf(stderr, "Queue is full\n");
        exit(EXIT_FAILURE);
    }

    if (queue_front_two == -1) {
        queue_front_two = 0;
        queue_rear_two = 0;
    } else {
        queue_rear_two = (queue_rear_two + 1) % max_processes;
    }

    processes[queue_rear_two] = process_id;

}
//---------------Dequeueing in processes--------------//
struct Process dequeue2() {
    if (queue_front_two == -1) {
        fprintf(stderr, "Queue is empty\n");
        exit(EXIT_FAILURE);
    }

    struct Process process_id = processes[queue_front_two];

    if (queue_front_two == queue_rear_two) {
        queue_front_two = -1;
        queue_rear_two = -1;
    } else {
        queue_front_two = (queue_front_two + 1) % max_processes;
    }

    return process_id;
}
//---------------Function to read a command and split it accoringly btw spaces--------------//
void read_fn(char input[], char *commands[], int *f2,int *submitflag,int *sche)
{
    int flag = 0;
    *f2 = 0;
    *sche = 0;
    *submitflag = 0;
    int m = 0, n = 0, idx = 0;

    int maximum_command_length2 = 100;

    for (int i = 0; input[i] != '\0'; i++)
    {
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
        commands[m] = NULL;
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
    if (strcmp(commands[0],"submit") == 0)
    {
        *submitflag = 1;
    }
    if (strcmp(commands[0],"schedular") == 0)
    {
        *sche = 1;
    }
    // removing the \n from the input and making it as \0
    input[strlen(input) - 1] = '\0';
}
char* removing_new_line(char* str)
{
    int length = strlen(str);
    if (length > 0 && str[length - 1] == '\n') {
        str[length - 1] = '\0';
    }
    return str;
}
// pipe implementation
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
//---------------Updating history--------------//
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
// fprintf(file_pointer, "COMMAND: %s ", history[gv++]->command /* xy */);

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

    struct timespec st, endtm, elapstm;

    // using clock_gettime function to calculate the time elapsed for the command
    // Get the start time
    clock_gettime(CLOCK_MONOTONIC, &st);


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

    // writing the elapsed_seconds to the file
    fprintf(file_pointer, " EXECUTION TIME: %.5lf \n", elapsed_seconds);

    fclose(file_pointer);
}


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
    // char line[100];
    // // printing the content of the file
    // while (fgets(line, sizeof(line), f) != NULL) {
    // printf("%s", line);
    // }
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
    printf("\n\nCtrl+C Pressed...\n\n\n");

    FILE *his = fopen("history.txt","r");
    if (his == NULL)
    {
        perror("error opening file\n");
        exit(1);
    }
    char line[1000];
    printf("Contents of history.txt:\n\n");
    while (fgets(line, sizeof(line), his) != NULL) {
        printf("%s", line);
    }
    fclose(his);
    FILE *h = fopen("history.txt","w");
    if (h == NULL)
    {
        perror("error opening the file\n");

    }
    fclose(h);
    // printf("num: %d\n",num);
    printf("average execution time due to round robbin: %f\n",0.01*(*avg_execution_time)/(num));
    printf("average wait time due to round robbin: %f\n",0.01*(*avg_wait_time)/(num));



//    displayProcessesQueue();
    exit(0);
}





//---------------Scheduling processes in round robin considering time and cpus--------------//
void start_sched2()
{
    *flag = 1;

    // copying the queue pointers from shared memory
    queue_front = *queue_front_shared;  
    queue_rear = *queue_rear_shared;

    // looping the scheduler
    while (queue_front != -1)
    {
        // forking all the processes and creating a grandchild process for executable to run
        int cur = queue_front;
        int cur2 = queue_rear;
        while (cur!=cur2+1)
        {
            struct Process p = dequeue();
            // if the process is not being created till now
            if (p.pid == -1)
            {
                int child_pid = fork();

                p.pid = child_pid;

                if (child_pid == 0)
                {
                    execlp(p.command, p.command, (char *)NULL);
                    perror("execlp");
                    exit(EXIT_FAILURE);
                }
                else{
                    // passing a sigstop signal to stop the child from execution
                    kill(child_pid, SIGSTOP);
                    // This is the parent process
                    printf("Process with pid - %d created.\n", child_pid);
                    enqueue(p);
                }
            }
                // if the process is being created already
            else
            {
                enqueue(p);
            }
            cur++;
        }

        // initializing running queue
        int running_queue_rear = -1;
        int running_queue_front = -1;
        struct Process* running_queue;
        running_queue = (struct Process*)malloc(100 * sizeof(struct Process));
        // initialization of running queue ended
        // printf("before ncpu loop\n");


        int x = 0;
        // taking out n cpu process from ready and put them is running continue them using sigcont and stop them again after the time slice
        while (x < NCPUS)
        {
            if (queue_front == -1)
            {
                break;
            }
            struct Process current_process = dequeue();
            x++;
            kill(current_process.pid, SIGCONT);
            printf("PID: %d\n", current_process.pid);

            // enqueue in runnning queue
            if (running_queue_front == -1) {
                running_queue_front = 0;
                running_queue_rear = 0;
            } else {
                running_queue_rear = (running_queue_rear + 1) % max_processes;
            }

            running_queue[running_queue_rear] = current_process;
        }
        //sleeping
        // printf("hhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhh\n\n")
        // long long time_ms = current_time_millis();
        // printf("Current time in milliseconds: %lld\n", time_ms);
        sleep_ms(TSLICE);
        // time_ms = current_time_millis();
        // printf("Current time in milliseconds: %lld\n", time_ms);

        // again stoping the process if not completed and add them in the ready queue
        int y = 0;
        while (y<x)
        {

            y++;
            struct Process p = running_queue[running_queue_front];

            if (running_queue_front == running_queue_rear) {
                running_queue_front = -1;
                running_queue_rear = -1;
            } else {
                running_queue_front = (running_queue_front + 1) % max_processes;
            }

            int status;
            p.execution_time +=  (TSLICE / 1000.0);
            if (waitpid(p.pid, &status, WNOHANG) == 0)
            {
                kill(p.pid, SIGSTOP);
                p.process_state = 0;

                enqueuenew(p);
            }
            else
            {
                // Record the end time when the process is stopped (finished)
                double current_time = (double)time(NULL);
                p.end_time = current_time;
                // Check if the process has finished
                double elapsed_seconds = p.end_time - p.start_time;
                p.wait_time = elapsed_seconds - p.execution_time;
                p.process_state = 2;
                enqueue2(p);
                // Open the file for appending
                *avg_execution_time+=p.execution_time;
                *avg_wait_time+=p.wait_time;
                FILE *file = fopen("history.txt", "a");
                if (file != NULL) {
                    fprintf(file, "PID: %d\n", p.pid);
                    fprintf(file, "Process State: %d\n", p.process_state);
                    fprintf(file, "Execution Time: %.2lf seconds\n", p.execution_time);
                    fprintf(file, "Wait Time: %.2lf seconds\n", p.wait_time);
                    // fprintf(file, "Termination Status: %d\n", p.termination_status);
                    fprintf(file, "Command: %s\n", p.command);
                    fprintf(file, "Priority: %d\n", p.priority);
                    fprintf(file, "Start Time: %.2lf seconds\n", p.start_time);
                    fprintf(file, "End Time: %.2lf seconds\n", p.end_time);
                    fprintf(file, "--------------------------------------------\n\n");
                    // Close the file
                    fclose(file);
                } else {
                    printf("Error opening the file for appending.\n");
                }
            }
        }
        running_queue_rear = -1;
        running_queue_front = -1;
        free(running_queue);
        printf("timeslice\n");
    }

    // updating the shared memory pointers here to avoid race condition
    *queue_front_shared = queue_front;
    *queue_rear_shared = queue_rear;
    *flag = 0;
}

//---------------Signal handler for the daemon process--------------//
// handling the timer signal so that the scheduler process gets the signal and runs the sched2 function after getting from SIGALRM signal
void handle_timer(int signum) {

    if (signum == SIGALRM)
    {
        // printf("signalrecieved\n");
        // printf("inside the handler\n");
        // printf("%s %s\n", process_queue[0].command,process_queue[1].command);
        start_sched2();
    }
}

// Runs a signal if keeps sending the signal to sched after t time to start scheduling
//---------------Mechanism to auto generate signal after particular time slice--------------//
void send_signal(int pid, float ts)
{
    sleep_ms(ts);

    if (queue_front == -1)
    {
        kill(pid, SIGALRM);
    }
    send_signal(pid, ts);
}


//  command line arguements
int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s NCPUS TSLICE\n", argv[0]);
        return 1;
    }
    NCPUS = atoi(argv[1]);
    TSLICE = atoi(argv[2]);


//shm setup
//------------------------------------------------------
    int shm_fd;
    int shm_fd2;
    int shm_fd3;
    int shm_fd4;
    int shm_fd5;
    float shm_fd6;
    float shm_fd7;


    // Create a shared memory segment
    shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    shm_fd2 = shm_open(SHM_NAME2, O_CREAT | O_RDWR, 0666);
    shm_fd3 = shm_open(SHM_NAME3, O_CREAT | O_RDWR, 0666);
    shm_fd4 = shm_open(SHM_NAME4, O_CREAT | O_RDWR, 0666);
    shm_fd5 = shm_open(SHM_NAME5, O_CREAT | O_RDWR, 0666);
    shm_fd6 = shm_open(SHM_NAME6, O_CREAT | O_RDWR, 0666);
    shm_fd7 = shm_open(SHM_NAME7, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open");
        exit(EXIT_FAILURE);
    }
    if (shm_fd2 == -1) {
        perror("shm_open");
        exit(EXIT_FAILURE);
    }
    if (shm_fd3 == -1) {
        perror("shm_open");
        exit(EXIT_FAILURE);
    }
    if (shm_fd4 == -1) {
        perror("shm_open");
        exit(EXIT_FAILURE);
    }
    if (shm_fd5 == -1) {
        perror("shm_open");
        exit(EXIT_FAILURE);
    }
    if (shm_fd6 == -1) {
        perror("shm_open");
        exit(EXIT_FAILURE);
    }
    if (shm_fd7 == -1) {
        perror("shm_open");
        exit(EXIT_FAILURE);
    }

    // Set the size of the shared memory segment
    ftruncate(shm_fd, max_processes * sizeof(struct Process));
    ftruncate(shm_fd2, max_processes * sizeof(struct Process));
    ftruncate(shm_fd3, sizeof(int));
    ftruncate(shm_fd4, sizeof(int));
    ftruncate(shm_fd5, sizeof(int));
    ftruncate(shm_fd6, sizeof(float));
    ftruncate(shm_fd7, sizeof(float ));


    // Map the shared memory segment into the address space of the process
    process_queue = (struct Process *)mmap(NULL, max_processes * sizeof(struct Process), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    processes = (struct Process *)mmap(NULL, max_processes * sizeof(struct Process), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd2, 0);
    queue_front_shared = (int *)mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd3, 0);
    queue_rear_shared = (int *)mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd4, 0);
    flag = (int *)mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd5, 0);
    avg_execution_time = (float *)mmap(NULL, sizeof(float), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd6, 0);
    avg_wait_time = (float *)mmap(NULL, sizeof(float), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd7, 0);

    if (process_queue == MAP_FAILED) {
        perror("mmap");
        exit(EXIT_FAILURE);
    }
    if (processes == MAP_FAILED) {
        perror("mmap");
        exit(EXIT_FAILURE);
    }
//------------------------------------------------------





// and here the daemon arrives
    *queue_front_shared = -1;
    *queue_rear_shared = -1;
    *flag = 0;
    int daemonPid = fork();
    // creating an independent schedular process
    if (daemonPid == 0)
    {
        while(1)
        {
            signal(SIGALRM, handle_timer);
            // printf("just entered daemon\n");
            pause();
            // printf("Now inside the daemon\n");
            // sleep(10);
        }
    }

    // creating a seperate process to send signals manually after a particular t millisecons
    // ideaaly it should be time slice but for showing the round robin schedilng  and other functionalities it is 15 sec
    int signalSenderPid = fork();

    if (signalSenderPid == 0)
    {
        send_signal(daemonPid, 15000);
    }

    char command[maximum_command_length];
    // shell loop
    while (1) {
        // cnt is for saving history
        cnt++;

        // assigning space to the history array of structure
        history[cnt] = (struct info *)malloc(sizeof(struct info));
        if (history[cnt] == NULL)
        {
            // error handling
            perror("Memory allocation for history entry failed");
            return 1;
        }

        printf("command shell> ");
        fgets(command, maximum_command_length, stdin);
        command[strcspn(command, "\n")] = 0;

        // storing the input in the history array
        history[cnt]->command = strdup(command);
        history[cnt]->command[strlen(command)-1] = '\0';

        if (history[cnt]->command == NULL)
        {
            perror("Memory allocation for command in history entry failed");
            // Handle the error, possibly by freeing previously allocated memory and exiting
            free(history[cnt]);
            return 1;
        }

        if (signal(SIGINT, terminate_program) == SIG_ERR)
        {
            perror("signal");
            return 1;
        }
        queue_front = *queue_front_shared;
        queue_rear = *queue_rear_shared;

        if (strcmp(command, "exit") == 0) {

            printf("Exiting the shell...\n");
            break;
        }
            // condition for submit command
        else if (strncmp(command, "submit", 6) == 0) {
            char executable[maximum_command_length];
            int priority = 1;
            if (sscanf(command, "submit %s %d", executable, &priority) == 1) {
                strcpy(executable, command + 7); // Extract the executable
            } else if (sscanf(command, "submit %s %d", executable, &priority) != 2) {
                printf("Invalid input format. Usage: submit <executable> [<priority>]\n");
            }
            num++;
            struct Process p;
            p.pid = -1;
            p.priority = priority;
            process_count++;

            double current_time = (double)time(NULL);
            p.start_time = current_time;
            strcpy(p.command, executable);
            p.execution_time = 0.0;
            p.wait_time = 0.0;
            if (*flag == 1)
            {
                if (fork() == 0)
                {
                    execlp(p.command, p.command, NULL);
                }
                else
                {
                    printf("PID: %d", getpid());
                }
            }

            // enqueue it in ready queue
            enqueuenew(p);
            // printf("printing for testing %d %d\n", *queue_front_shared, *queue_rear_shared);

        }

            // not used as we have done signal handling
        else if (strcmp(command, "run") == 0) {
            for (int i=0; i<3; i++)
            {
                processes[i] = process_queue[queue_front+i];
            }
            kill(daemonPid, SIGALRM);
        }

            // printing the process in ready queueu
        else if(strcmp(command, "print") == 0){
            for(int i= queue_front; i<=queue_rear; i++)
            {
                printf("%s ", process_queue[i].command);
            }
            printf("\n");

        }
        else {
            printf("Unknown command: %s\n", command);
        }
    }





//shm close
//-----------------------------------------------------

    munmap(process_queue, max_processes * sizeof(struct Process));
    munmap(processes, max_processes * sizeof(struct Process));
    close(shm_fd);
    close(shm_fd2);

    // Remove the shared memory segment (optional, comment out if not needed)
    shm_unlink(SHM_NAME);
    shm_unlink(SHM_NAME2);
    shm_unlink(SHM_NAME3);
    shm_unlink(SHM_NAME4);
    shm_unlink(SHM_NAME5);
    shm_unlink(SHM_NAME6);
    shm_unlink(SHM_NAME7);


//-----------------------------------------------------
    return 0;
}

