#include "ProcessManager.h"
#include <algorithm>

ProcessManager::ProcessManager() : m_nextPid(1) {}

const std::vector<Process>& ProcessManager::getProcesses() const {
    return m_processes;
}

std::vector<Process>& ProcessManager::getProcesses() {
    return m_processes;
}

bool ProcessManager::createProcess(const QString& name, int arrival, int burst, int priority, int memory) {
    QString pName = name.isEmpty() ? QString("P%1").arg(m_nextPid) : name;
    Process p(m_nextPid++, pName, arrival, burst, priority, memory);
    p.state = ProcessState::READY; // Created processes default to READY state
    m_processes.push_back(p);
    notifyObservers();
    return true;
}

bool ProcessManager::deleteProcess(int pid) {
    auto it = std::remove_if(m_processes.begin(), m_processes.end(), [pid](const Process& p) {
        return p.pid == pid;
    });
    if (it != m_processes.end()) {
        m_processes.erase(it, m_processes.end());
        notifyObservers();
        return true;
    }
    return false;
}

bool ProcessManager::suspendProcess(int pid) {
    for (auto& p : m_processes) {
        if (p.pid == pid) {
            if (p.state == ProcessState::RUNNING || p.state == ProcessState::READY || p.state == ProcessState::NEW) {
                p.state = ProcessState::WAITING;
                notifyObservers();
                return true;
            }
        }
    }
    return false;
}

bool ProcessManager::resumeProcess(int pid) {
    for (auto& p : m_processes) {
        if (p.pid == pid) {
            if (p.state == ProcessState::WAITING) {
                p.state = ProcessState::READY;
                notifyObservers();
                return true;
            }
        }
    }
    return false;
}

void ProcessManager::updateProcessState(int pid, ProcessState state) {
    for (auto& p : m_processes) {
        if (p.pid == pid) {
            p.state = state;
            notifyObservers();
            break;
        }
    }
}

void ProcessManager::reset() {
    m_processes.clear();
    m_nextPid = 1;
    notifyObservers();
}
