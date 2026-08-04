#ifndef DISK_SCHEDULER_H
#define DISK_SCHEDULER_H

#include <vector>
#include <memory>
#include <QString>
#include "../common/Subject.h"

// Strategy Pattern interface for Disk Scheduling
class DiskSchedulingStrategy {
public:
    virtual ~DiskSchedulingStrategy() = default;
    virtual void schedule(int startHead, 
                          const std::vector<int>& requests, 
                          std::vector<int>& seekSequence, 
                          int totalCylinders = 200, 
                          bool movingRight = true) = 0;
};

// 1. FCFS
class DiskFCFSStrategy : public DiskSchedulingStrategy {
public:
    void schedule(int startHead, const std::vector<int>& requests, std::vector<int>& seekSequence, int totalCylinders = 200, bool movingRight = true) override;
};

// 2. SSTF (Shortest Seek Time First)
class DiskSSTFStrategy : public DiskSchedulingStrategy {
public:
    void schedule(int startHead, const std::vector<int>& requests, std::vector<int>& seekSequence, int totalCylinders = 200, bool movingRight = true) override;
};

// 3. SCAN
class DiskSCANStrategy : public DiskSchedulingStrategy {
public:
    void schedule(int startHead, const std::vector<int>& requests, std::vector<int>& seekSequence, int totalCylinders = 200, bool movingRight = true) override;
};

// 4. C-SCAN (Circular SCAN)
class DiskCSCANStrategy : public DiskSchedulingStrategy {
public:
    void schedule(int startHead, const std::vector<int>& requests, std::vector<int>& seekSequence, int totalCylinders = 200, bool movingRight = true) override;
};

// 5. LOOK
class DiskLOOKStrategy : public DiskSchedulingStrategy {
public:
    void schedule(int startHead, const std::vector<int>& requests, std::vector<int>& seekSequence, int totalCylinders = 200, bool movingRight = true) override;
};

// 6. C-LOOK (Circular LOOK)
class DiskCLOOKStrategy : public DiskSchedulingStrategy {
public:
    void schedule(int startHead, const std::vector<int>& requests, std::vector<int>& seekSequence, int totalCylinders = 200, bool movingRight = true) override;
};

// Factory for Disk strategies
class DiskSchedulerFactory {
public:
    enum class AlgorithmType {
        FCFS,
        SSTF,
        SCAN,
        CSCAN,
        LOOK,
        CLOOK
    };

    static std::unique_ptr<DiskSchedulingStrategy> createStrategy(AlgorithmType type);
};

// DiskScheduler class managing the simulation execution and metrics
class DiskScheduler : public CustomDS::Subject {
private:
    int m_startHead;
    std::vector<int> m_requests;
    int m_totalCylinders;
    bool m_initialDirectionRight; // true = towards larger numbers, false = towards 0
    
    DiskSchedulerFactory::AlgorithmType m_currentAlgo;
    std::vector<int> m_seekSequence;
    int m_totalHeadMovement;
    double m_avgSeekTime;

    void calculateMetrics();

public:
    DiskScheduler();
    ~DiskScheduler() = default;

    void setStartHead(int head);
    void setRequests(const std::vector<int>& requests);
    void setTotalCylinders(int cylinders);
    void setInitialDirection(bool movingRight);
    void setAlgorithm(DiskSchedulerFactory::AlgorithmType type);

    void runSimulation();

    // Getters
    const std::vector<int>& getSeekSequence() const;
    int getTotalHeadMovement() const;
    double getAvgSeekTime() const;
    DiskSchedulerFactory::AlgorithmType getAlgorithmType() const;
    QString getAlgorithmName() const;
};

#endif // DISK_SCHEDULER_H
