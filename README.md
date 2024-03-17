# [OS] Round Robin Scheduler Implementation
2023 COM3005 Operating System Term Projects

## Goal
Implementing a simulator for RR scheduling, including the ready queue and I/O queue. The parent process acts as the scheduler.

### Basic explanations and requirements
Parent process :
>  - Parent process creates 10 child processes.
>  - Parent process periodically receives `ALARM` signal by registering timer event. Students may want to refer to setitimer system call. The `ALARM` signal serves as periodic timer interrupt (or time tick).
>  - Parent process manages the ready queue and I/O queue.
>    - Elements of the queue should be pointer variables to structures holding information about each child process (e.g. pid, remaining CPU burst time) (think of PCB, though simpler).
>  - The parent process performs scheduling of its child processes: The parent process accounts for the remaining time quantum of all the child processes.
>  - The parent process gives time slice to the child process by sending IPC message through msgq.
>  - Total running time should be more than 1 minute.

Child process :
>  - A child process simulates the execution of a user process.
>  - Workload consists of infinite loop of dynamic CPU-burst and I/O-burst.
>  - CPU burst is randomly determined at the time of child process creation.
>  - When a child process receives a start IPC message from the parent process, it is considered to be running.
>  - About I/O operation:
>    - At each dispatch, the 'creation' of I/O burst is determined randomly by a certain probability.
>    - If an i/o burst is created,
>      - The 'start time of I/O' is also randomly determined, as the 'I/O duration'.
>      - The 'start time of I/O' come before the end of the time quantum while the process is in running state.
>      - When 'start time of I/O' occurs, the next child process in the ready queue executes.
>      - Information about the i/o is sent to the parent process via IPC.
>  - If the CPU burst time of a child process ends before the time quantum, an IPC message is sent to the parent.

Logging:
>  - The following contents are output for every time tick `t`:
>  - (1) pid of the child process in running state, (2) list of processes in the ready queue and i/o queue, (3) remaining cpu burst time and i/o burst time for each process.
>  - Print out all the operations to the following file: schedule_dump.txt.

## Start
```bash
cd TermProject
g++ -std=c++17 "term_project_rr.cpp" -o "term_project_rr" && "./term_project_rr"
```

## Term Project Report
https://github.com/sunkyuj/rr-scheduling/blob/main/OS%20Term%20Project%20Report.pdf
