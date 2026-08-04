#include "DeadlockDetector.h"
#include <algorithm>

DeadlockDetector::DeadlockDetector() 
    : m_processCount(5), m_resourceCount(3) {
    // Default textbook configuration
    std::vector<int> totals = {10, 5, 7}; // Resources A, B, C
    initialize(5, 3, totals);

    // Initial allocations (Standard Banker's Example)
    setAllocationRow(0, {0, 1, 0});
    setAllocationRow(1, {2, 0, 0});
    setAllocationRow(2, {3, 0, 2});
    setAllocationRow(3, {2, 1, 1});
    setAllocationRow(4, {0, 0, 2});

    // Max resource claims
    setMaxRow(0, {7, 5, 3});
    setMaxRow(1, {3, 2, 2});
    setMaxRow(2, {9, 0, 2});
    setMaxRow(3, {2, 2, 2});
    setMaxRow(4, {4, 3, 3});
}

void DeadlockDetector::initialize(int processCount, int resourceCount, const std::vector<int>& totalResources) {
    m_processCount = processCount;
    m_resourceCount = resourceCount;
    m_totalResources = totalResources;

    m_allocation.assign(m_processCount, std::vector<int>(m_resourceCount, 0));
    m_max.assign(m_processCount, std::vector<int>(m_resourceCount, 0));
    m_need.assign(m_processCount, std::vector<int>(m_resourceCount, 0));
    m_request.assign(m_processCount, std::vector<int>(m_resourceCount, 0));
    m_available = m_totalResources;

    reset();
}

void DeadlockDetector::setAllocationRow(int processIdx, const std::vector<int>& allocRow) {
    if (processIdx >= 0 && processIdx < m_processCount) {
        m_allocation[processIdx] = allocRow;
        updateNeedMatrix();
        reset(); // Re-calculate Available resources
    }
}

void DeadlockDetector::setMaxRow(int processIdx, const std::vector<int>& maxRow) {
    if (processIdx >= 0 && processIdx < m_processCount) {
        m_max[processIdx] = maxRow;
        updateNeedMatrix();
    }
}

void DeadlockDetector::setRequestRow(int processIdx, const std::vector<int>& reqRow) {
    if (processIdx >= 0 && processIdx < m_processCount) {
        m_request[processIdx] = reqRow;
        rebuildGraph();
        notifyObservers();
    }
}

void DeadlockDetector::updateNeedMatrix() {
    for (int i = 0; i < m_processCount; ++i) {
        for (int j = 0; j < m_resourceCount; ++j) {
            m_need[i][j] = std::max(0, m_max[i][j] - m_allocation[i][j]);
        }
    }
}

void DeadlockDetector::rebuildGraph() {
    m_graph.clear();
    
    // Add nodes
    for (int i = 0; i < m_processCount; ++i) {
        m_graph.addNode("P" + std::to_string(i));
    }
    for (int j = 0; j < m_resourceCount; ++j) {
        m_graph.addNode("R" + std::to_string(j));
    }

    // Add edges
    for (int i = 0; i < m_processCount; ++i) {
        for (int j = 0; j < m_resourceCount; ++j) {
            // If resource is allocated to process: Edge Rj -> Pi
            if (m_allocation[i][j] > 0) {
                m_graph.addEdge("R" + std::to_string(j), "P" + std::to_string(i), false);
            }
            // If process requests resource: Edge Pi -> Rj
            if (m_request[i][j] > 0) {
                m_graph.addEdge("P" + std::to_string(i), "R" + std::to_string(j), true);
            }
        }
    }
}

bool DeadlockDetector::runBankers(std::vector<int>& safeSeq, QString& message) {
    std::vector<int> work = m_available;
    std::vector<bool> finish(m_processCount, false);
    safeSeq.clear();

    // Check if any process is terminated (allocation is all 0s, max is all 0s)
    // We treat terminated processes as already finished.
    for (int i = 0; i < m_processCount; ++i) {
        bool isTerminated = true;
        for (int j = 0; j < m_resourceCount; ++j) {
            if (m_allocation[i][j] > 0 || m_max[i][j] > 0) {
                isTerminated = false;
                break;
            }
        }
        if (isTerminated) {
            finish[i] = true;
        }
    }

    int completedCount = std::count(finish.begin(), finish.end(), true);
    int activeProcessCount = m_processCount - completedCount;

    for (int step = 0; step < m_processCount; ++step) {
        bool found = false;
        for (int i = 0; i < m_processCount; ++i) {
            if (!finish[i]) {
                // Check if Need[i] <= Work
                bool canAllocate = true;
                for (int j = 0; j < m_resourceCount; ++j) {
                    if (m_need[i][j] > work[j]) {
                        canAllocate = false;
                        break;
                    }
                }

                if (canAllocate) {
                    // Release resources
                    for (int j = 0; j < m_resourceCount; ++j) {
                        work[j] += m_allocation[i][j];
                    }
                    finish[i] = true;
                    safeSeq.push_back(i);
                    found = true;
                    break;
                }
            }
        }
        if (!found) break;
    }

    if ((int)safeSeq.size() == activeProcessCount) {
        QString seqStr = "";
        for (size_t i = 0; i < safeSeq.size(); ++i) {
            seqStr += QString("P%1").arg(safeSeq[i]);
            if (i < safeSeq.size() - 1) seqStr += " -> ";
        }
        message = "SAFE STATE: Safe sequence found: " + seqStr;
        return true;
    } else {
        message = "UNSAFE STATE: Deadlock risk detected! No safe sequence can be determined.";
        return false;
    }
}

std::vector<std::string> DeadlockDetector::detectDeadlockCycle() {
    rebuildGraph();
    return m_graph.detectCycle();
}

bool DeadlockDetector::recoverDeadlock(int processIdx) {
    if (processIdx < 0 || processIdx >= m_processCount) return false;

    // Release resources allocated to the process
    for (int j = 0; j < m_resourceCount; ++j) {
        m_available[j] += m_allocation[processIdx][j];
        m_allocation[processIdx][j] = 0;
        m_max[processIdx][j] = 0;
        m_need[processIdx][j] = 0;
        m_request[processIdx][j] = 0;
    }

    rebuildGraph();
    notifyObservers();
    return true;
}

void DeadlockDetector::reset() {
    // Recompute Available resources = Totals - Sum of allocations
    m_available = m_totalResources;
    for (int j = 0; j < m_resourceCount; ++j) {
        int allocatedSum = 0;
        for (int i = 0; i < m_processCount; ++i) {
            allocatedSum += m_allocation[i][j];
        }
        m_available[j] = std::max(0, m_totalResources[j] - allocatedSum);
    }
    updateNeedMatrix();
    rebuildGraph();
    notifyObservers();
}

int DeadlockDetector::getProcessCount() const { return m_processCount; }
int DeadlockDetector::getResourceCount() const { return m_resourceCount; }
const std::vector<std::vector<int>>& DeadlockDetector::getAllocation() const { return m_allocation; }
const std::vector<std::vector<int>>& DeadlockDetector::getMax() const { return m_max; }
const std::vector<std::vector<int>>& DeadlockDetector::getNeed() const { return m_need; }
const std::vector<std::vector<int>>& DeadlockDetector::getRequest() const { return m_request; }
const std::vector<int>& DeadlockDetector::getAvailable() const { return m_available; }
const std::vector<int>& DeadlockDetector::getTotalResources() const { return m_totalResources; }
const CustomDS::ResourceAllocationGraph& DeadlockDetector::getGraph() const { return m_graph; }
