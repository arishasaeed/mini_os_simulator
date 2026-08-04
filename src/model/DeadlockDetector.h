#ifndef DEADLOCK_DETECTOR_H
#define DEADLOCK_DETECTOR_H

#include <vector>
#include <string>
#include <QString>
#include "../common/Subject.h"
#include "../common/DataStructures.h"

class DeadlockDetector : public CustomDS::Subject {
private:
    int m_processCount;
    int m_resourceCount;

    // Banker's Algorithm Matrices
    std::vector<std::vector<int>> m_allocation; // PxR
    std::vector<std::vector<int>> m_max;        // PxR
    std::vector<std::vector<int>> m_need;       // PxR
    std::vector<int> m_available;               // R
    std::vector<int> m_totalResources;          // R

    // Graph representation
    CustomDS::ResourceAllocationGraph m_graph;

    void updateNeedMatrix();
    void rebuildGraph();

public:
    DeadlockDetector();
    ~DeadlockDetector() = default;

    void initialize(int processCount, int resourceCount, const std::vector<int>& totalResources);
    
    // Setters for matrices
    void setAllocationRow(int processIdx, const std::vector<int>& allocRow);
    void setMaxRow(int processIdx, const std::vector<int>& maxRow);
    
    // Banker's Algorithm safe check
    bool runBankers(std::vector<int>& safeSeq, QString& message);

    // Deadlock Detection via Graph (Request matrix cycle search)
    // Here, we can define a request matrix explicitly
    std::vector<std::vector<int>> m_request; // PxR
    void setRequestRow(int processIdx, const std::vector<int>& reqRow);
    
    std::vector<std::string> detectDeadlockCycle();
    bool recoverDeadlock(int processIdx); // Terminate process to release its allocation

    // Getters
    int getProcessCount() const;
    int getResourceCount() const;
    const std::vector<std::vector<int>>& getAllocation() const;
    const std::vector<std::vector<int>>& getMax() const;
    const std::vector<std::vector<int>>& getNeed() const;
    const std::vector<std::vector<int>>& getRequest() const;
    const std::vector<int>& getAvailable() const;
    const std::vector<int>& getTotalResources() const;
    const CustomDS::ResourceAllocationGraph& getGraph() const;

    void reset();
};

#endif // DEADLOCK_DETECTOR_H
