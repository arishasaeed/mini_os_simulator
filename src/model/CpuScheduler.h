#ifndef CPU_SCHEDULER_H
#define CPU_SCHEDULER_H

#include <vector>
#include <memory>
#include <QString>
#include "../common/Subject.h"
#include "Process.h"

// Struct representing a slice in the Gantt Chart visualization
struct GanttSegment {
    int pid;
    QString name;
    int startTime;
    int endTime;
};

// Strategy Pattern interface for CPU Scheduling Algorithms
class CpuSchedulingStrategy {
public:
    virtual ~CpuSchedulingStrategy() = default;
    virtual void schedule(const std::vector<Process>& processes, 
                          std::vector<Process>& scheduledProcesses, 
                          std::vector<GanttSegment>& ganttChart, 
                          int quantum = 2) = 0;
};

// 1. First Come First Served (FCFS)
class FCFSStrategy : public CpuSchedulingStrategy {
public:
    void schedule(const std::vector<Process>& processes, 
                  std::vector<Process>& scheduledProcesses, 
                  std::vector<GanttSegment>& ganttChart, 
                  int quantum = 2) override;
};

// 2. Shortest Job First (SJF) - Supports Preemptive and Non-Preemptive
class SJFStrategy : public CpuSchedulingStrategy {
private:
    bool m_preemptive;
public:
    SJFStrategy(bool preemptive) : m_preemptive(preemptive) {}
    void schedule(const std::vector<Process>& processes, 
                  std::vector<Process>& scheduledProcesses, 
                  std::vector<GanttSegment>& ganttChart, 
                  int quantum = 2) override;
};

// 3. Priority Scheduling - Supports Preemptive and Non-Preemptive
class PriorityStrategy : public CpuSchedulingStrategy {
private:
    bool m_preemptive;
public:
    PriorityStrategy(bool preemptive) : m_preemptive(preemptive) {}
    void schedule(const std::vector<Process>& processes, 
                  std::vector<Process>& scheduledProcesses, 
                  std::vector<GanttSegment>& ganttChart, 
                  int quantum = 2) override;
};

// 4. Round Robin (RR)
class RoundRobinStrategy : public CpuSchedulingStrategy {
public:
    void schedule(const std::vector<Process>& processes, 
                  std::vector<Process>& scheduledProcesses, 
                  std::vector<GanttSegment>& ganttChart, 
                  int quantum = 2) override;
};

// Factory Pattern for Scheduling Strategies
class SchedulerFactory {
public:
    enum class AlgorithmType {
        FCFS,
        SJF_NON_PREEMPTIVE,
        SJF_PREEMPTIVE,
        PRIORITY_NON_PREEMPTIVE,
        PRIORITY_PREEMPTIVE,
        ROUND_ROBIN
    };

    static std::unique_ptr<CpuSchedulingStrategy> createStrategy(AlgorithmType type);
};

// CpuScheduler class managing scheduler execution, metrics, and comparisons
class CpuScheduler : public CustomDS::Subject {
private:
    std::vector<Process> m_inputProcesses;
    std::vector<Process> m_scheduledProcesses;
    std::vector<GanttSegment> m_ganttChart;
    
    SchedulerFactory::AlgorithmType m_currentAlgo;
    int m_timeQuantum;
    
    double m_avgWaitingTime;
    double m_avgTurnaroundTime;
    double m_avgResponseTime;
    double m_cpuUtilization; // Simulated CPU util %
    double m_throughput;     // Jobs completed per time unit

    void calculateMetrics();

public:
    CpuScheduler();
    ~CpuScheduler() = default;

    void setProcesses(const std::vector<Process>& processes);
    void setAlgorithm(SchedulerFactory::AlgorithmType type);
    void setTimeQuantum(int quantum);
    
    void runSimulation();

    // Getters for results
    const std::vector<Process>& getScheduledProcesses() const;
    const std::vector<GanttSegment>& getGanttChart() const;
    double getAvgWaitingTime() const;
    double getAvgTurnaroundTime() const;
    double getAvgResponseTime() const;
    double getCpuUtilization() const;
    double getThroughput() const;
    SchedulerFactory::AlgorithmType getAlgorithmType() const;
    QString getAlgorithmName() const;
};

#endif // CPU_SCHEDULER_H
