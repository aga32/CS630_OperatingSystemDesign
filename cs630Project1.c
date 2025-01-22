#include <stdio.h>
#define MAX_PROCESSES 30
#define MAX_SIGNALS 30

// Define a struct that holds multiple variables
typedef struct {
    int pid;
    int priority;
    int burst;
    int signalCount;
    int fork;
} Process;

typedef struct{
    int pid;
    int signalCount;
} Signals;

// Function prototypes
void append(Process readyQueue[], int *size, Process elem);
void remove_Process(Process readyQueue[], int *size, int index);
void roundRobin(Process readyQueue[], int *size, int *totalTime, int *currentProcess, int *nextPid);
int findSmallestPriority(Process readyQueue[], int size);
void printReadyQueue(Process readyQueue[], int size); // <- Fix this line

// Function to find the process with the smallest priority
int findSmallestPriority(Process readyQueue[], int size) { // <- Fix this line
    if (size == 0) {
        printf("The queue is empty.\n");
        return -1; // Indicating that the queue is empty
    }

    while (size > 0 && readyQueue[size - 1].pid == -1) {
        size--;
    }

    int minIndex = 0;
    for (int i = 1; i < size; i++) {
        if (readyQueue[i].priority < readyQueue[minIndex].priority) {
            minIndex = i;
        }
    }
    return minIndex;
}

void printReadyQueue(Process readyQueue[], int size) { 
    printf("PID\t Priority\tBurst\tSignal Count\n");
    for (int i = 0; i < size; i++) {
        if (readyQueue[i].pid != -1) {
            printf("%d\t    %d\t       %d\t      %d\n", readyQueue[i].pid, readyQueue[i].priority, readyQueue[i].burst, readyQueue[i].signalCount);
        }
    }
}

// Round-robin scheduling algorithm
void roundRobin(Process readyQueue[], int *size, int *totalTime, int *currentProcess, int *nextPid){
    int x = 0;
    while (readyQueue[*currentProcess].burst > 0 && x < 4) {
        // Increment signal count every 3 seconds
        if ((*totalTime) % 3 == 0 && *totalTime != 0) {
            readyQueue[*currentProcess].signalCount++;
        }

        x++;
        (*totalTime)++;
        readyQueue[*currentProcess].fork++;
        readyQueue[*currentProcess].burst--;

        // Forking new processes every 3 bursts
        if (readyQueue[*currentProcess].fork % 3 == 0){
            Process newProcess;
            if (readyQueue[*currentProcess].priority == 3) {
                // Create a new process with priority 1, burst of 5, and signal count of 0
                newProcess = (Process){(*nextPid)++, 1, 5, 0};
                append(readyQueue, size, newProcess);
            }
            else if (readyQueue[*currentProcess].priority == 2) {
                // Create a new process with priority 3, burst of 7, and signal count of 0
                newProcess = (Process){(*nextPid)++, 3, 7, 0};
                append(readyQueue, size, newProcess);
            } 

        }
    }
}

// Function to append a process to the readyQueue
void append(Process readyQueue[], int *size, Process elem) {
    if (*size >= MAX_PROCESSES) {
        printf("Queue is full. Cannot append more processes.\n");
        return;
    }
    readyQueue[*size] = elem;
    (*size)++;
}

//function to append a signal to the array
void appendSignal(Signals array[], int *size, Signals elem) {
    if (*size >= MAX_SIGNALS) {
        printf("Signal array is full. Cannot append more signals.\n");
        return;
    }
    array[*size] = elem;
    (*size)++;
}

// Function to remove a process from the readyQueue by index
void remove_Process(Process readyQueue[], int *size, int index) {
    if (index < 0 || index >= *size) {
        printf("Index out of range.\n");
        return;
    }
    for (int i = index; i < *size - 1; i++) {
        readyQueue[i] = readyQueue[i + 1];
    }
    (*size)--;
}

int main() {
    Process readyQueue[MAX_PROCESSES];
    int size = 0;
    int startTime = 0;
    int totalTime = 0;
    int initialTime = 0;
    int nextPid = 3; // Next process id to be assigned
    int currentProcess = 0;

    Signals array[MAX_SIGNALS];
    int arraySize = 0;

    // Insert initial processes
    Process p1 = {1, 2, 10, 0, 0};
    Process p2 = {2, 3, 7, 0, 0};

    // Append initial processes
    append(readyQueue, &size, p1);
    append(readyQueue, &size, p2);

    // Print Gantt chart header
    printf("Gantt Chart:\n");
    printf("Process ID | Start Time | End Time\n");


    // Execute round robin until all processes are completed
    while(size > 0){
        currentProcess = findSmallestPriority(readyQueue, size);
        if(readyQueue[currentProcess].burst == 0){
            //append process id and signal count to signals arrya and remove the process from the queue
            appendSignal(array, &arraySize, (Signals){readyQueue[currentProcess].pid, readyQueue[currentProcess].signalCount});
            remove_Process(readyQueue, &size, currentProcess);
        }
        else
            roundRobin(readyQueue, &size, &totalTime, &currentProcess, &nextPid);
        if(initialTime != totalTime)
            printf("%d              %d             %d\n", readyQueue[currentProcess].pid, initialTime, totalTime);
        initialTime = totalTime;
        //call print ready queue function here to print the current state of the ready queue
    }

    // Print the signals array
    printf("\nSignal Counts:\n");
    printf("Process ID | Signal Count\n");
    for(int i = 0; i < arraySize; i++){
        printf("%d             %d\n", array[i].pid, array[i].signalCount);
    }

    return 0;
}


/*
OUTPUT:
Gantt Chart:
Process ID | Start Time | End Time
1              0             4
1              4             8
1              8             10
2              10            14
6              14            18
6              18            19
2              19            22
7              22            26
7              26            27
3              27            31
8              31            35
8              35            36
3              36            39
9              39            43
9              43            44
4              44            48
10             48            52
10             52            53
4              53            56
11             56            60
11             60            61
5              61            65
12             65            69
12             69            70
5              70            73
13             73            77
13             77            78

Signal Counts: 
Process ID | Signal Count
1             3
6             2
7             1
2             2
8             1
9             2
3             3
10            2
11            2
4             2
12            2
13            1
5             2
*/
