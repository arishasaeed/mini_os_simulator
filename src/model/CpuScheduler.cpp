#include "CpuScheduler.h"
#include <algorithm>
#include "../common/DataStructures.h"

// ==========================================
// 1. FCFS Strategy Implementation
// ==========================================
void FCFSStrategy::schedule(const std::vector<Process>& processes, 
                          std::vector<Process>& scheduledProcesses, 
                          std::vector<GanttSegment>& ganttChart, 
                          int quantum) {
    (void)quantum;
    scheduledProcesses = processes;
    if (scheduledProcesses.empty()) return;

    // Sort by arrival time
    std::sort(scheduledProcesses.begin(), scheduledProcesses.end(), [](const Process& a, const Process& b) {
        return a.arrivalTime < b.arrivalTime;
    });

    int currentTime = 0;
    for (auto& p : scheduledProcesses) {
        if (currentTime < p.arrivalTime) {
            currentTime = p.arrivalTime;
        }
        
        p.startTime = currentTime;
        p.responseTime = currentTime - p.arrivalTime;
        
        ganttChart.push_back({p.pid, p.name, currentTime, currentTime + p.burstTime});
        currentTime += p.burstTime;
        
        p.finishTime = currentTime;
        p.turnaroundTime = p.finishTime - p.arrivalTime;
        p.waitingTime = p.turnaroundTime - p.burstTime;
        p.remainingTime = 0;
        p.state = ProcessState::TERMINATED;
    }
}

// ==========================================
// 2. SJF Strategy Implementation
// ==========================================
void SJFStrategy::schedule(const std::vector<Process>& processes, 
                          std::vector<Process>& scheduledProcesses, 
                          std::vector<GanttSegment>& ganttChart, 
                          int quantum) {
    (void)quantum;
    scheduledProcesses = processes;
    if (scheduledProcesses.empty()) return;

    int n = scheduledProcesses.size();
    
    if (!m_preemptive) {
        // Non-Preemptive SJF
        std::vector<bool> completed(n, false);
        int currentTime = 0;
        int completedCount = 0;

        while (completedCount < n) {
            int selectIdx = -1;
            int minBurst = 1e9;

            for (int i = 0; i < n; ++i) {
                if (!completed[i] && scheduledProcesses[i].arrivalTime <= currentTime) {
                    if (scheduledProcesses[i].burstTime < minBurst) {
                        minBurst = scheduledProcesses[i].burstTime;
                        selectIdx = i;
                    }
                }
            }

            if (selectIdx == -1) {
                // Find next arrival time
                int nextArrival = 1e9;
                for (int i = 0; i < n; ++i) {
                    if (!completed[i]) {
                        nextArrival = std::min(nextArrival, scheduledProcesses[i].arrivalTime);
                    }
                }
                currentTime = nextArrival;
            } else {
                auto& p = scheduledProcesses[selectIdx];
                p.startTime = currentTime;
                p.responseTime = currentTime - p.arrivalTime;
                
                ganttChart.push_back({p.pid, p.name, currentTime, currentTime + p.burstTime});
                currentTime += p.burstTime;
                
                p.finishTime = currentTime;
                p.turnaroundTime = p.finishTime - p.arrivalTime;
                p.waitingTime = p.turnaroundTime - p.burstTime;
                p.remainingTime = 0;
                p.state = ProcessState::TERMINATED;
                
                completed[selectIdx] = true;
                completedCount++;
            }
        }
    } else {
        // Preemptive SJF (Shortest Remaining Time First)
        int currentTime = 0;
        int completedCount = 0;
        int activePid = -1;
        int segmentStart = 0;
        QString activeName = "";

        while (completedCount < n) {
            int selectIdx = -1;
            int minRemaining = 1e9;

            for (int i = 0; i < n; ++i) {
                if (scheduledProcesses[i].remainingTime > 0 && scheduledProcesses[i].arrivalTime <= currentTime) {
                    if (scheduledProcesses[i].remainingTime < minRemaining) {
                        minRemaining = scheduledProcesses[i].remainingTime;
                        selectIdx = i;
                    }
                }
            }

            if (selectIdx == -1) {
                if (activePid != -1) {
                    ganttChart.push_back({activePid, activeName, segmentStart, currentTime});
                    activePid = -1;
                }
                // Idle, advance to next arrival
                int nextArrival = 1e9;
                for (int i = 0; i < n; ++i) {
                    if (scheduledProcesses[i].remainingTime > 0) {
                        nextArrival = std::min(nextArrival, scheduledProcesses[i].arrivalTime);
                    }
                }
                currentTime = nextArrival;
            } else {
                auto& p = scheduledProcesses[selectIdx];
                if (p.pid != activePid) {
                    if (activePid != -1) {
                        ganttChart.push_back({activePid, activeName, segmentStart, currentTime});
                    }
                    activePid = p.pid;
                    activeName = p.name;
                    segmentStart = currentTime;
                }

                if (p.responseTime == -1) {
                    p.responseTime = currentTime - p.arrivalTime;
                    p.startTime = currentTime;
                }

                p.remainingTime--;
                currentTime++;

                if (p.remainingTime == 0) {
                    p.finishTime = currentTime;
                    p.turnaroundTime = p.finishTime - p.arrivalTime;
                    p.waitingTime = p.turnaroundTime - p.burstTime;
                    p.state = ProcessState::TERMINATED;
                    completedCount++;
                }
            }
        }
        if (activePid != -1) {
            ganttChart.push_back({activePid, activeName, segmentStart, currentTime});
        }
    }
}

// ==========================================
// 3. Priority Strategy Implementation
// ==========================================
void PriorityStrategy::schedule(const std::vector<Process>& processes, 
                              std::vector<Process>& scheduledProcesses, 
                              std::vector<GanttSegment>& ganttChart, 
                              int quantum) {
    (void)quantum;
    scheduledProcesses = processes;
    if (scheduledProcesses.empty()) return;

    int n = scheduledProcesses.size();

    if (!m_preemptive) {
        // Non-Preemptive Priority
        std::vector<bool> completed(n, false);
        int currentTime = 0;
        int completedCount = 0;

        while (completedCount < n) {
            int selectIdx = -1;
            int highestPriority = 1e9; // Smaller numerical value = higher priority

            for (int i = 0; i < n; ++i) {
                if (!completed[i] && scheduledProcesses[i].arrivalTime <= currentTime) {
                    if (scheduledProcesses[i].priority < highestPriority) {
                        highestPriority = scheduledProcesses[i].priority;
                        selectIdx = i;
                    }
                }
            }

            if (selectIdx == -1) {
                int nextArrival = 1e9;
                for (int i = 0; i < n; ++i) {
                    if (!completed[i]) {
                        nextArrival = std::min(nextArrival, scheduledProcesses[i].arrivalTime);
                    }
                }
                currentTime = nextArrival;
            } else {
                auto& p = scheduledProcesses[selectIdx];
                p.startTime = currentTime;
                p.responseTime = currentTime - p.arrivalTime;
                
                ganttChart.push_back({p.pid, p.name, currentTime, currentTime + p.burstTime});
                currentTime += p.burstTime;
                
                p.finishTime = currentTime;
                p.turnaroundTime = p.finishTime - p.arrivalTime;
                p.waitingTime = p.turnaroundTime - p.burstTime;
                p.remainingTime = 0;
                p.state = ProcessState::TERMINATED;
                
                completed[selectIdx] = true;
                completedCount++;
            }
        }
    } else {
        // Preemptive Priority
        int currentTime = 0;
        int completedCount = 0;
        int activePid = -1;
        int segmentStart = 0;
        QString activeName = "";

        while (completedCount < n) {
            int selectIdx = -1;
            int highestPriority = 1e9;

            for (int i = 0; i < n; ++i) {
                if (scheduledProcesses[i].remainingTime > 0 && scheduledProcesses[i].arrivalTime <= currentTime) {
                    if (scheduledProcesses[i].priority < highestPriority) {
                        highestPriority = scheduledProcesses[i].priority;
                        selectIdx = i;
                    }
                }
            }

            if (selectIdx == -1) {
                if (activePid != -1) {
                    ganttChart.push_back({activePid, activeName, segmentStart, currentTime});
                    activePid = -1;
                }
                int nextArrival = 1e9;
                for (int i = 0; i < n; ++i) {
                    if (scheduledProcesses[i].remainingTime > 0) {
                        nextArrival = std::min(nextArrival, scheduledProcesses[i].arrivalTime);
                    }
                }
                currentTime = nextArrival;
            } else {
                auto& p = scheduledProcesses[selectIdx];
                if (p.pid != activePid) {
                    if (activePid != -1) {
                        ganttChart.push_back({activePid, activeName, segmentStart, currentTime});
                    }
                    activePid = p.pid;
                    activeName = p.name;
                    segmentStart = currentTime;
                }

                if (p.responseTime == -1) {
                    p.responseTime = currentTime - p.arrivalTime;
                    p.startTime = currentTime;
                }

                p.remainingTime--;
                currentTime++;

                if (p.remainingTime == 0) {
                    p.finishTime = currentTime;
                    p.turnaroundTime = p.finishTime - p.arrivalTime;
                    p.waitingTime = p.turnaroundTime - p.burstTime;
                    p.state = ProcessState::TERMINATED;
                    completedCount++;
                }
            }
        }
        if (activePid != -1) {
            ganttChart.push_back({activePid, activeName, segmentStart, currentTime});
        }
    }
}

// ==========================================
// 4. Round Robin Strategy Implementation
// ==========================================
void RoundRobinStrategy::schedule(const std::vector<Process>& processes, 
                                std::vector<Process>& scheduledProcesses, 
                                std::vector<GanttSegment>& ganttChart, 
                                int quantum) {
    scheduledProcesses = processes;
    if (scheduledProcesses.empty()) return;

    // Sort initially by arrival time to handle queue entries correctly
    std::sort(scheduledProcesses.begin(), scheduledProcesses.end(), [](const Process& a, const Process& b) {
        return a.arrivalTime < b.arrivalTime;
    });

    int n = scheduledProcesses.size();
    CustomDS::Queue<int> readyQueue;
    std::vector<bool> inQueue(n, false);
    
    int currentTime = 0;
    int completedCount = 0;

    // Helper lambda to enqueue processes that have arrived up to current time
    auto enqueueArrived = [&](int time) {
        for (int i = 0; i < n; ++i) {
            if (!inQueue[i] && scheduledProcesses[i].arrivalTime <= time && scheduledProcesses[i].remainingTime > 0) {
                readyQueue.enqueue(i);
                inQueue[i] = true;
            }
        }
    };

    // Push the first batch of arrivals
    enqueueArrived(currentTime);

    while (completedCount < n) {
        if (readyQueue.isEmpty()) {
            // Queue empty, jump to the next arriving process
            int nextArrival = 1e9;
            for (int i = 0; i < n; ++i) {
                if (scheduledProcesses[i].remainingTime > 0) {
                    nextArrival = std::min(nextArrival, scheduledProcesses[i].arrivalTime);
                }
            }
            currentTime = nextArrival;
            enqueueArrived(currentTime);
        }

        int idx = readyQueue.dequeue();
        auto& p = scheduledProcesses[idx];

        if (p.responseTime == -1) {
            p.responseTime = currentTime - p.arrivalTime;
            p.startTime = currentTime;
        }

        int runTime = std::min(p.remainingTime, quantum);
        ganttChart.push_back({p.pid, p.name, currentTime, currentTime + runTime});
        
        currentTime += runTime;
        p.remainingTime -= runTime;

        // Enqueue any processes that arrived during our execution slice
        enqueueArrived(currentTime);

        if (p.remainingTime > 0) {
            // Put current process back to end of queue
            readyQueue.enqueue(idx);
        } else {
            p.finishTime = currentTime;
            p.turnaroundTime = p.finishTime - p.arrivalTime;
            p.waitingTime = p.turnaroundTime - p.burstTime;
            p.state = ProcessState::TERMINATED;
            completedCount++;
        }
    }
}

// ==========================================
// Scheduler Factory Implementation
// ==========================================
std::unique_ptr<CpuSchedulingStrategy> SchedulerFactory::createStrategy(AlgorithmType type) {
    switch (type) {
        case AlgorithmType::FCFS:
            return std::make_unique<FCFSStrategy>();
        case AlgorithmType::SJF_NON_PREEMPTIVE:
            return std::make_unique<SJFStrategy>(false);
        case AlgorithmType::SJF_PREEMPTIVE:
            return std::make_unique<SJFStrategy>(true);
        case AlgorithmType::PRIORITY_NON_PREEMPTIVE:
            return std::make_unique<PriorityStrategy>(false);
        case AlgorithmType::PRIORITY_PREEMPTIVE:
            return std::make_unique<PriorityStrategy>(true);
        case AlgorithmType::ROUND_ROBIN:
            return std::make_unique<RoundRobinStrategy>();
    }
    return nullptr;
}

// ==========================================
// CpuScheduler Class Implementation
// ==========================================
CpuScheduler::CpuScheduler() 
    : m_currentAlgo(SchedulerFactory::AlgorithmType::FCFS),
      m_timeQuantum(2),
      m_avgWaitingTime(0.0),
      m_avgTurnaroundTime(0.0),
      m_avgResponseTime(0.0),
      m_cpuUtilization(100.0),
      m_throughput(0.0) {}

void CpuScheduler::setProcesses(const std::vector<Process>& processes) {
    m_inputProcesses = processes;
    // Reset process states for scheduling run
    for (auto& p : m_inputProcesses) {
        p.remainingTime = p.burstTime;
        p.responseTime = -1;
        p.startTime = -1;
        p.finishTime = 0;
        p.waitingTime = 0;
        p.turnaroundTime = 0;
        p.state = ProcessState::READY;
    }
}

void CpuScheduler::setAlgorithm(SchedulerFactory::AlgorithmType type) {
    m_currentAlgo = type;
}

void CpuScheduler::setTimeQuantum(int quantum) {
    m_timeQuantum = quantum;
}

void CpuScheduler::runSimulation() {
    m_scheduledProcesses.clear();
    m_ganttChart.clear();

    if (m_inputProcesses.empty()) {
        m_avgWaitingTime = 0.0;
        m_avgTurnaroundTime = 0.0;
        m_avgResponseTime = 0.0;
        m_cpuUtilization = 0.0;
        m_throughput = 0.0;
        notifyObservers();
        return;
    }

    auto strategy = SchedulerFactory::createStrategy(m_currentAlgo);
    if (strategy) {
        strategy->schedule(m_inputProcesses, m_scheduledProcesses, m_ganttChart, m_timeQuantum);
        calculateMetrics();
    }
    notifyObservers();
}

void CpuScheduler::calculateMetrics() {
    double totalWaiting = 0;
    double totalTurnaround = 0;
    double totalResponse = 0;
    int maxFinishTime = 0;

    for (const auto& p : m_scheduledProcesses) {
        totalWaiting += p.waitingTime;
        totalTurnaround += p.turnaroundTime;
        totalResponse += p.responseTime;
        maxFinishTime = std::max(maxFinishTime, p.finishTime);
    }

    int n = m_scheduledProcesses.size();
    m_avgWaitingTime = totalWaiting / n;
    m_avgTurnaroundTime = totalTurnaround / n;
    m_avgResponseTime = totalResponse / n;

    // Throughput: jobs completed per unit of time
    m_throughput = (maxFinishTime > 0) ? (double)n / maxFinishTime : 0.0;

    // CPU Utilization: Calculate idle slices in Gantt Chart
    int activeExecutionTime = 0;
    for (const auto& seg : m_ganttChart) {
        activeExecutionTime += (seg.endTime - seg.startTime);
    }
    
    // Check if Gantt Chart segments overlap or leave gaps
    // Calculate total duration as maxEnd - minStart
    int minStartTime = 1e9;
    for (const auto& seg : m_ganttChart) {
        minStartTime = std::min(minStartTime, seg.startTime);
    }
    
    int totalSimulationTime = maxFinishTime - (minStartTime == 1e9 ? 0 : minStartTime);
    if (totalSimulationTime > 0) {
        // Find unique execute intervals
        // For simplicity: active / total time
        // Handle overlap bounds correctly
        m_cpuUtilization = ((double)activeExecutionTime / maxFinishTime) * 100.0;
        if (m_cpuUtilization > 100.0) m_cpuUtilization = 100.0;
    } else {
        m_cpuUtilization = 0.0;
    }
}

const std::vector<Process>& CpuScheduler::getScheduledProcesses() const {
    return m_scheduledProcesses;
}

const std::vector<GanttSegment>& CpuScheduler::getGanttChart() const {
    return m_ganttChart;
}

double CpuScheduler::getAvgWaitingTime() const { return m_avgWaitingTime; }
double CpuScheduler::getAvgTurnaroundTime() const { return m_avgTurnaroundTime; }
double CpuScheduler::getAvgResponseTime() const { return m_avgResponseTime; }
double CpuScheduler::getCpuUtilization() const { return m_cpuUtilization; }
double CpuScheduler::getThroughput() const { return m_throughput; }
SchedulerFactory::AlgorithmType CpuScheduler::getAlgorithmType() const { return m_currentAlgo; }

QString CpuScheduler::getAlgorithmName() const {
    switch (m_currentAlgo) {
        case SchedulerFactory::AlgorithmType::FCFS: return "FCFS";
        case SchedulerFactory::AlgorithmType::SJF_NON_PREEMPTIVE: return "SJF (Non-Preemptive)";
        case SchedulerFactory::AlgorithmType::SJF_PREEMPTIVE: return "SRTF (Preemptive SJF)";
        case SchedulerFactory::AlgorithmType::PRIORITY_NON_PREEMPTIVE: return "Priority (Non-Preemptive)";
        case SchedulerFactory::AlgorithmType::PRIORITY_PREEMPTIVE: return "Priority (Preemptive)";
        case SchedulerFactory::AlgorithmType::ROUND_ROBIN: return "Round Robin";
    }
    return "Unknown";
}
