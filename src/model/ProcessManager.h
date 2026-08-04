#ifndef PROCESS_MANAGER_H
#define PROCESS_MANAGER_H

#include <vector>
#include "../common/Subject.h"
#include "Process.h"

class ProcessManager : public CustomDS::Subject {
private:
    std::vector<Process> m_processes;
    int m_nextPid;

public:
    ProcessManager();
    ~ProcessManager() = default;

    const std::vector<Process>& getProcesses() const;
    std::vector<Process>& getProcesses();

    // Core Features
    bool createProcess(const QString& name, int arrival, int burst, int priority, int memory);
    bool deleteProcess(int pid);
    bool suspendProcess(int pid); // Moves to WAITING state
    bool resumeProcess(int pid);  // Moves back to READY state
    
    // State transition helper
    void updateProcessState(int pid, ProcessState state);
    
    void reset();
};

#endif // PROCESS_MANAGER_H
