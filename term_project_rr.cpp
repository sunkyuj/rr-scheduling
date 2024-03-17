#include <iostream>
#include <fstream>
#include <vector>
#include <queue>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/time.h>
#include <cstdlib>
#include <ctime>
#include <cstring>
#define READY 2      // 준비상태
#define RUNNIG 3     // 실행상태
#define WAITING 4    // 대기상태(I/O 발생)
#define TERMINATED 5 // 종료상태

using namespace std;

struct PCB
{
    int idx;                // Work의 index 번호 (0~9)
    int pid;                // Child process id
    string processName;     // Work0 ~ Work9
    int state;              // Process state (READY, RUNNIG, WAITING, TERMINATED)
    int remainingCPUBurst;  // 남은 CPU burst
    int remainingIoBurst;   // 남은 I/O burst
    int ioStartTimeQuantum; // I/O가 시작될 time quantum
    int ioStartsIn;         // I/O 시작까지 남은 시간
};

// message structure for IPC
struct msgBuffer
{
    long mtype = 0; // Work의 pid나 work의 상태
    PCB process;
};

// Global variables
int msgQueueID;
const int childNum = 10;
int responseTimes[childNum] = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1}; // 응답시간
int turnaroundTimes[childNum];                                          // 반환시간
int totalCpuTime = 0;
int totalIoTime = 0;

// Process
int pid;               // process id (0이면 자식 프로세스)
PCB currentProcess;    // 현재 실행중인 프로세스
queue<PCB> readyQueue; // ready queue
PCB ioProcess;         // 현재 I/O 작업중인 프로세스
queue<PCB> ioQueue;    // I/O queue

// Time
int currentTime = 0;             // 현재 시간
const int timeQuantum = 3;       // time quantum (변경 가능)
const int timerTickInterval = 1; // Timer tick interval in seconds

// Initialization functions
void init();                 // 기본 설정 및 변수 초기화
void createChildProcesses(); // 자식 프로세스 생성

// Scheduling functions
void handleAlarmSignal(int signum);           // 문맥교환 함수 (알람 시그널 활용)
void roundRobin();                            // 라운드 로빈 알고리즘 수행
void ioScheduling(int &remainingTimeQuantum); // I/O 스케줄링 (FCFS)
void childProcess();                          // 자식 프로세스 실행

// Utilitiy functions
void sendMsg(int type, PCB process); // IPC 메세지 전송
string printProcess(PCB &process);   // 프로세스 정보 출력
string printQueue(queue<PCB> &q);    // 큐 정보 출력
void writeLog(string logDetailsStr); // 로그 파일에 기록

void childProcess()
{
    while (1)
    {
        msgBuffer msg;
        // 자식의 pid와 같은 mtype의 메세지 올때까지 기다림
        if (msgrcv(msgQueueID, &msg, sizeof(PCB), getpid(), 0) == -1)
        {
            perror("msgrcv");
            exit(1);
        }
        // 메세지 수신 성공시 CPU 할당 -> time quantum 할당
        PCB process = msg.process;
        int remainingTimeQuantum = timeQuantum;

        while (remainingTimeQuantum)
        {
            cout << printProcess(process);

            if (process.ioStartTimeQuantum == remainingTimeQuantum) // I/O 시작시간 도달
            {
                cout << " -> I/O start\n";
                process.state = WAITING;
                process.ioStartsIn = 0;
                process.ioStartTimeQuantum = 0;
                sendMsg(WAITING, process); // CPU 할당 해제시기키 위해 부모에게 메세지 전송
                break;                     // CPU 할당 해제
            }

            sleep(timerTickInterval); // 1초

            process.remainingCPUBurst--;
            remainingTimeQuantum--;
            cout << "->" << printProcess(process) << ", remaining time quantum: " << remainingTimeQuantum << endl;

            if (process.remainingCPUBurst == 0) // CPU burst 완료
            {
                process.state = TERMINATED;
                sendMsg(TERMINATED, process); // send msg to parent process
                return;
            }
        }
    }
}

// Function to simulate child process execution
void roundRobin()
{
    writeLog("Initial state: " + printQueue(readyQueue) + "\n");
    int remainingTimeQuantum = 0;

    while (currentProcess.remainingCPUBurst > 0 || ioProcess.remainingIoBurst > 0 || !readyQueue.empty() || !ioQueue.empty())
    {
        msgBuffer msg;

        // 0초
        cout << currentTime << " s" << endl;

        if (remainingTimeQuantum == 0) // time quantum 소진시 스케줄링
        {
            raise(SIGALRM); // SIGALRM 신호 발생 -> 자식 프로세스 실행
            writeLog("<<<Time slice!  " + to_string(currentTime) + " sec >>>\n");
            remainingTimeQuantum = timeQuantum;
        }

        writeLog("[Time: start of " + to_string(currentTime) + " sec]\n");
        writeLog("\tCPU Allocated Process: " + printProcess(currentProcess) + "\n");
        writeLog("\tReady Queue: " + printQueue(readyQueue) + "\n");
        writeLog("\tI/O Running: " + printProcess(ioProcess) + "\n");
        writeLog("\tI/O Queue: " + printQueue(ioQueue) + "\n");

        sleep(timerTickInterval); // 1초
        if (currentProcess.remainingCPUBurst > 0)
            currentProcess.remainingCPUBurst--;
        if (ioProcess.remainingIoBurst > 0)
            ioProcess.remainingIoBurst--;
        if (currentProcess.ioStartsIn > 0)
            currentProcess.ioStartsIn--;
        remainingTimeQuantum--;

        writeLog("[Time: end of " + to_string(currentTime) + " sec]\n");

        // 자식 프로세스 종료 메세지 수신
        if (msgrcv(msgQueueID, &msg, sizeof(PCB), TERMINATED, IPC_NOWAIT) != -1) // IPC_NOWAIT ← 큐 가득차면 기다리지 않고 에러 반환
        {
            writeLog(printProcess(msg.process) + " - finished\n");
            turnaroundTimes[msg.process.idx] = currentTime + 1;
            if (currentProcess.pid == msg.process.pid) // 메세지 늦게 온 경우 고려
            {
                currentProcess = PCB();
                remainingTimeQuantum = 0;
            }
        }
        // io 발생시 (time quantum 소진과 동시에 일어나지 않음)
        if (msgrcv(msgQueueID, &msg, sizeof(PCB), WAITING, IPC_NOWAIT) != -1) // IPC_NOWAIT ← 큐 가득차면 기다리지 않고 에러 반환
        {
            writeLog(printProcess(msg.process) + " - I/O started");
            if (ioProcess.remainingIoBurst > 0)
                writeLog(", but different I/O is running already. Enqueue to ioQueue.");
            ioQueue.push(msg.process); // 프로세스를 io queue에 넣음
            currentProcess = PCB();    // 현재 프로세스를 cpu에서 해제
            remainingTimeQuantum = 0;
            writeLog("\n");
        }

        // io 스케줄링(FCFS)
        ioScheduling(remainingTimeQuantum);

        writeLog("\tCPU Allocated Process: " + printProcess(currentProcess) + "\n");
        writeLog("\tReady Queue: " + printQueue(readyQueue) + "\n");
        writeLog("\tI/O Running: " + printProcess(ioProcess) + "\n");
        writeLog("\tI/O Queue: " + printQueue(ioQueue) + "\n");
        writeLog("\n");

        currentTime++;
    }

    writeLog("[Time: " + to_string(currentTime) + " sec] - end of simulation\n");
    return;
}

int main()
{
    init();       // 기본 설정, 변수 초기화
    roundRobin(); // 라운드 로빈 알고리즘 실행

    // 스케줄링 결과
    int totalResponseTime = 0, totalTurnaroundTime = 0;
    for (int i = 0; i < childNum; ++i)
    {
        totalResponseTime += responseTimes[i];
        totalTurnaroundTime += turnaroundTimes[i];
        writeLog("Work" + to_string(i) + " response time: " + to_string(responseTimes[i]) + " sec, turnaround time: " + to_string(turnaroundTimes[i]) + " sec\n");
    }
    writeLog("Average response time: " + to_string((double)totalResponseTime / childNum) + " sec, average turnaround time: " + to_string((double)totalTurnaroundTime / childNum) + " sec\n");
    writeLog("Throughput: " + to_string((double)childNum / totalTurnaroundTime) + " jobs/sec\n");
    writeLog("CPU utilization: " + to_string((double)totalCpuTime / currentTime * 100) + "%\n");
    writeLog("I/O utilization: " + to_string((double)totalIoTime / currentTime * 100) + "%\n");
    return 0;
}

void init()
{
    srand(time(NULL)); // random seed

    sigset(SIGALRM, handleAlarmSignal); // SIGALRM 신호 오면 해당 콜백함수 실행

    // Create the message queue
    key_t key = ftok(".", 1);
    if ((msgQueueID = msgget(key, 0666 | IPC_CREAT)) == -1)
    {
        perror("msgget");
        exit(1);
    }
    // delete msg queue
    if (msgctl(msgQueueID, IPC_RMID, NULL) == -1)
    {
        perror("msgctl");
        exit(1);
    }
    if ((msgQueueID = msgget(key, 0666 | IPC_CREAT | IPC_EXCL)) == -1)
    {
        perror("msgget");
        exit(1);
    }

    // Initialize child processes and their PCBs
    createChildProcesses();
}

// Function to create child processes
void createChildProcesses()
{

    for (int i = 0; i < childNum; ++i)
    {
        pid = fork();
        if (pid < 0)
        {
            printf("Error in fork()\n");
            exit(-1);
        }

        PCB newProcess;
        newProcess.idx = i;                               // Process 번호
        newProcess.pid = pid;                             // 자식의 pid 저장
        newProcess.processName = "Work" + to_string(i);   // Process 이름
        newProcess.state = READY;                         // Process 상태
        newProcess.remainingCPUBurst = (rand() % 10) + 1; // Random CPU burst 생성
        totalCpuTime += newProcess.remainingCPUBurst;

        if (pid == 0) // Child process
        {
            // newProcess.pid = getpid();
            childProcess();
            cout << "Work" << i << " end" << endl;
            exit(0);
        }
        else // Parent process
            readyQueue.push(newProcess);
    }
}

// Function to handle the alarm signal
void handleAlarmSignal(int signum)
{
    if (currentProcess.remainingCPUBurst > 0) // 만약 CPU burst가 남아있으면 ready queue에 넣음
        readyQueue.push(currentProcess);

    if (!readyQueue.empty()) // 메세지 큐에 타임슬라이스 보내기
    {
        PCB nextProcess = readyQueue.front();
        readyQueue.pop();
        nextProcess.state = RUNNIG;
        if (responseTimes[nextProcess.idx] == -1)
            responseTimes[nextProcess.idx] = currentTime;

        // IO 발생 여부
        if (nextProcess.remainingCPUBurst >= timeQuantum && rand() % 2 == 0) // 50% 확률로 I/O 발생 (whether to perform i/o)
        {
            // io start time radomly determined
            nextProcess.ioStartTimeQuantum = rand() % (timeQuantum - 1) + 1;
            nextProcess.ioStartsIn = timeQuantum - nextProcess.ioStartTimeQuantum;
            // io duration randomly determined
            nextProcess.remainingIoBurst = (rand() % 10) + 1;
            cout << nextProcess.processName << " - has I/O!" << printProcess(nextProcess) << endl;
            totalIoTime += nextProcess.remainingIoBurst;
        }

        // Dispatch the next process (for example, by sending an IPC message)
        currentProcess = nextProcess;
        sendMsg(nextProcess.pid, nextProcess); // send msg to child process
    }
}

void ioScheduling(int &remainingTimeQuantum)
{
    if (ioProcess.remainingIoBurst <= 0) // 수행중인 io 작업 없거나, 완료시
    {
        if (ioProcess.state == WAITING) // 대기상태 였는데 완료됨
        {
            writeLog(printProcess(ioProcess) + " - I/O finished");
            if (ioProcess.remainingCPUBurst > 0) // 만약 CPU burst가 남아있으면 ready queue에 넣음
            {
                writeLog(", returned to ready queue");
                ioProcess.state = READY;
                readyQueue.push(ioProcess);
            }
            writeLog("\n");
        }

        ioProcess = PCB();    // 현재 실행중인 io 작업 없음
        if (!ioQueue.empty()) // io queue에 프로세스가 있으면 즉시 dispatch
        {
            ioProcess = ioQueue.front();
            ioQueue.pop();
        }

        if (currentProcess.remainingCPUBurst <= 0 && !readyQueue.empty()) // CPU 작업을 dispatch 가능하다면
            remainingTimeQuantum = 0;                                     // 즉시 dispatch 할 수 있도록 time quantum 초기화
    }
}

// type에 자식의 pid가 들어가면 자식에게 메세지 전송, 아니면 부모에게 메세지 전송 (대기 or 종료 알림)
void sendMsg(int type, PCB process)
{
    msgBuffer msg;
    msg.mtype = type;
    msg.process = process;
    if (msgsnd(msgQueueID, &msg, sizeof(PCB), 0) == -1)
    {
        perror("msgsnd");
        exit(1);
    }
}

string printProcess(PCB &process)
{
    if (process.pid == 0)
        return "No process running";

    string result = "(";
    result += process.processName;
    result += ", CPU Burst: " + to_string(process.remainingCPUBurst);
    result += ", I/O Burst: " + to_string(process.remainingIoBurst);
    if (process.ioStartsIn > 0)
        result += ", I/O Starts in: " + to_string(process.ioStartsIn) + " sec";
    result += ")";
    return result;
}

string printQueue(queue<PCB> &q)
{
    string queueStr = "";
    PCB process;
    int n = q.size();

    for (int i = 0; i < n; i++)
    {
        process = q.front();
        q.pop();
        queueStr += printProcess(process);
        if (i != n - 1)
            queueStr += ", ";

        q.push(process);
    }
    return queueStr;
}

void writeLog(string logDetailsStr)
{
    ofstream logFile("schedule_dump.txt", ios_base::app);
    if (logFile.is_open())
    {
        logFile << logDetailsStr;
        logFile.close();
    }
}
