#include <stdio.h>

#define MAX_ELEMENTS 40
#define MAX_PROCESSES 7
#define MAX_BLOCKED 4

typedef struct {
    int pid;
} ReadyQueue;

typedef struct { 
    int pid;
    int ioArrival;
    int ioUnits;
    int ioRelease;
} Blocked;

typedef struct {
    int pid;
    int arrival;
    int burst;
    int previousEndTime;    // for waiting time
    int firstExecutionTime; // for waiting and response time
    int turnAroundTime;
    int responseTime;
    int waitingTime;
    int startTime;
} ProcessTable;

// append process to ready queue
void appendProcess(ReadyQueue readyQueue[], int *size, ProcessTable process) {
    if (*size >= MAX_ELEMENTS) {
        printf("Queue is full. Cannot append more processes.\n");
        return;
    }
    readyQueue[*size].pid = process.pid;
    (*size)++;
}

// remove process from ready queue
void remove_Process(ReadyQueue readyQueue[], int *size, int index) {
    if (index < 0 || index >= *size) {
        printf("Index out of range.\n");
        return;
    }
    for (int i = index; i < *size - 1; i++) {
        readyQueue[i] = readyQueue[i + 1];
    }
    (*size)--;
}

void moveToEnd(ReadyQueue readyQueue[], int *size) {
    if (*size <= 1) 
        return; // No need to move if array has 1 or fewer elements

    ReadyQueue firstElement = readyQueue[0];
    for (int i = 0; i < *size - 1; i++) {
        readyQueue[i] = readyQueue[i + 1];
    }
    readyQueue[*size - 1] = firstElement;
}

// round robin scheduling
int roundRobin(ReadyQueue readyQueue[], int *size, Blocked blockedQueue[], int *blockedSize, int *totalTime, int timeQuantum, int *processTableIndex, ProcessTable processTable[], int *initialTime, int *currentProcess, int *removed, int *j) {
    int x = 0;
    int check = 0;
    *currentProcess = 0;
    *removed = 0;

    // loop to find which pid in processTable has the same pid being executed in readyQueue
    while (readyQueue[0].pid != processTable[*currentProcess].pid) {
        (*currentProcess)++;
    }

    if (processTable[*currentProcess].firstExecutionTime == 0 && processTable[*currentProcess].previousEndTime == 0) {
        processTable[*currentProcess].firstExecutionTime = *totalTime;
        processTable[*currentProcess].startTime = *totalTime;

        // calculate response time
        processTable[*currentProcess].responseTime = *totalTime - processTable[*currentProcess].arrival;
        processTable[*currentProcess].waitingTime = processTable[*currentProcess].responseTime;
        check = 1;
    }
    else{
        processTable[*currentProcess].startTime = *totalTime;
    }

    while (processTable[*currentProcess].burst > 0 && x < timeQuantum && *removed != 1 && *removed != 2) {
        x++;
        (*totalTime)++;
        processTable[*currentProcess].burst--;

        // check if any processes have arrived at totalTime
        if (*processTableIndex < MAX_PROCESSES && processTable[*processTableIndex].arrival == *totalTime) {
            appendProcess(readyQueue, size, processTable[*processTableIndex]);
            (*processTableIndex)++;
        }

        // check if any processes are blocked
        for(int b = 0; b < *blockedSize; b++){
            if (blockedQueue[b].ioArrival == *totalTime){ 
                blockedQueue[b].pid = processTable[*currentProcess].pid;
                blockedQueue[b].ioRelease = blockedQueue[b].ioArrival + blockedQueue[b].ioUnits;
                *removed = 2; // Indicate that a process was blocked
            }
        }

        // If the process has completed its burst, remove it from the queue
        if (processTable[*currentProcess].burst == 0) {
            // calculate turnAroundTime
            processTable[*currentProcess].turnAroundTime = *totalTime - processTable[*currentProcess].arrival;
            *removed = 1; // Indicate that a process finished
        }

        // check if blocked processes have completed (ioRelease == *totalTime)
        for (int y = 0; y < *blockedSize; y++) {
            int k = 0;
            if (blockedQueue[y].ioRelease == *totalTime) {
                 //loop through procesTable to find same pid
                for (int p = 0; p < MAX_PROCESSES; p++) {
                    if (blockedQueue[y].pid == processTable[p].pid) {
                        k = p;
                    }
                }

                if(processTable[k].burst == 0){
                    //do not add to any queue
                    //calculate wait time
                    processTable[k].waitingTime += (*totalTime - processTable[k].previousEndTime); 

                }

                else
                    appendProcess(readyQueue, size, processTable[blockedQueue[y].pid - 1]);
                // Calculate waiting time in regard to blocked time
                processTable[k].waitingTime -= blockedQueue[y].ioUnits;
            }
        }
    }

    // calculate waiting time
    if(*j != processTable[*currentProcess].pid && check != 1){
        processTable[*currentProcess].waitingTime += (processTable[*currentProcess].startTime - processTable[*currentProcess].previousEndTime); 
    }

    processTable[*currentProcess].previousEndTime = *totalTime;
    *j = processTable[*currentProcess].pid;
    
    if(*removed != 1 && *removed != 2)
        moveToEnd(readyQueue, size);  // move the process at the front of the queue to the end

    return *removed;
}

// main function
int main() {
    // define variables
    int totalTime = 0;
    int processTableIndex = 0;
    int initialTime = 0;
    int blockedSize = MAX_BLOCKED;
    int size = 0;
    int currentProcess = 0;
    int removed = 0;
    int timeQuantum = 4;
    int j = 0; //check for consecutive processes in gantt chart

    // initialize array of process structures and insert process data
    ProcessTable processTable[MAX_PROCESSES] = {
        {1, 0, 19, 0, 0, 0, 0, 0, 0},
        {2, 5, 16, 0, 0, 0, 0, 0, 0},
        {3, 15, 27, 0, 0, 0, 0, 0, 0},
        {4, 35, 13, 0, 0, 0, 0, 0, 0},
        {5, 50, 10, 0, 0, 0, 0, 0, 0},
        {6, 65, 26, 0, 0, 0, 0, 0, 0},
        {7, 94, 19, 0, 0, 0, 0, 0, 0}
    };

    // initialize ready queue
    ReadyQueue readyQueue[MAX_ELEMENTS];

    // initialize blocked queue
    Blocked blockedQueue[MAX_BLOCKED] = {
        {0, 18, 18, 0},
        {0, 43, 22, 0}, 
        {0, 72, 25, 0}, 
        {0, 104, 14, 0}
    };

    // print header for gantt chart
    printf("Gantt Chart\n");
    printf("ProcessID   |    Start Time    |    End Time\n");

    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (processTable[i].arrival == 0) {
            appendProcess(readyQueue, &size, processTable[i]);
            processTableIndex++;
        }
    }

    while (size > 0) {
        // round robin scheduling
        removed = roundRobin(readyQueue, &size, blockedQueue, &blockedSize, &totalTime, timeQuantum, &processTableIndex, processTable, &initialTime, &currentProcess, &removed, &j);
        if(removed == 1 || removed == 2){
            printf("%d                    %d                  %d\n", processTable[currentProcess].pid, initialTime, totalTime);
            remove_Process(readyQueue, &size, 0); 
        }
        else if(initialTime == totalTime){}
        else
            printf("%d                    %d                  %d\n", readyQueue[size-1].pid, initialTime, totalTime);
        initialTime = totalTime;
    }
    // print waiting time, response time, and turnaround time for each process
    printf("\nProcess ID | Response Time | Waiting Time | Turnaround Time\n");
    for (int i = 0; i < MAX_PROCESSES; i++) {
        printf("%d               %d               %d             %d \n", processTable[i].pid, processTable[i].responseTime, processTable[i].waitingTime, processTable[i].turnAroundTime);
    }


    return 0;
}

/*
OUTPUT:
Gantt Chart
ProcessID   |    Start Time    |    End Time
1                    0                  4
1                    4                  8
2                    8                  12
1                    12                 16
2                    16                 18
3                    18                 22
1                    22                 26
3                    26                 30
1                    30                 33
3                    33                 37
4                    37                 41
2                    41                 43
3                    43                 47
4                    47                 51
3                    51                 55
5                    55                 59
4                    59                 63
3                    63                 67
5                    67                 71
4                    71                 72
6                    72                 76
2                    76                 80
3                    80                 83
5                    83                 85
6                    85                 89
2                    89                 93
6                    93                 97
7                    97                 101
6                    101                104
7                    104                108
7                    108                112
7                    112                116
7                    116                119
6                    119                123
6                    123                127
6                    127                130

Process ID | Response Time | Waiting Time | Turnaround Time
1               0               14             33 
2               3               32             88 
3               3               41             68 
4               2               24             37 
5               5               25             35 
6               7               25             65 
7               3               6              25 
*/
