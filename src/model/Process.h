#ifndef PROCESS_H
#define PROCESS_H

#include <QString>

enum class ProcessState {
    NEW,
    READY,
    RUNNING,
    WAITING,
    TERMINATED
};

struct Process {
    int pid;
    QString name;
    ProcessState state;
    int priority;          // Lower value can mean higher or lower priority; we will treat smaller values as higher priority
    int arrivalTime;
    int burstTime;
    int remainingTime;
    int waitingTime;
    int turnaroundTime;
    int responseTime;
    int finishTime;
    int memoryRequired;    // Memory required in MB
    int ioDuration;        // Simulated I/O duration if suspended
    int startTime;
    bool started;

    Process(int id = 0, const QString& pName = "", int arr = 0, int burst = 0, int prio = 0, int mem = 32)
        : pid(id),
          name(pName),
          state(ProcessState::NEW),
          priority(prio),
          arrivalTime(arr),
          burstTime(burst),
          remainingTime(burst),
          waitingTime(0),
          turnaroundTime(0),
          responseTime(-1),
          finishTime(0),
          memoryRequired(mem),
          ioDuration(0),
          startTime(-1),
          started(false) {}

    QString getStateString() const {
        switch (state) {
            case ProcessState::NEW: return "NEW";
            case ProcessState::READY: return "READY";
            case ProcessState::RUNNING: return "RUNNING";
            case ProcessState::WAITING: return "WAITING";
            case ProcessState::TERMINATED: return "TERMINATED";
        }
        return "UNKNOWN";
    }
};

#endif // PROCESS_H
