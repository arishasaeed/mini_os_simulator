#include "DiskScheduler.h"
#include <algorithm>
#include <cmath>

// ==========================================
// 1. FCFS Disk Strategy
// ==========================================
void DiskFCFSStrategy::schedule(int startHead, const std::vector<int>& requests, std::vector<int>& seekSequence, int totalCylinders, bool movingRight) {
    (void)totalCylinders; (void)movingRight;
    seekSequence.push_back(startHead);
    for (int req : requests) {
        seekSequence.push_back(req);
    }
}

// ==========================================
// 2. SSTF Disk Strategy
// ==========================================
void DiskSSTFStrategy::schedule(int startHead, const std::vector<int>& requests, std::vector<int>& seekSequence, int totalCylinders, bool movingRight) {
    (void)totalCylinders; (void)movingRight;
    seekSequence.push_back(startHead);
    std::vector<int> temp = requests;
    int currentHead = startHead;

    while (!temp.empty()) {
        auto closestIt = temp.begin();
        int minDistance = std::abs(*closestIt - currentHead);
        
        for (auto it = temp.begin() + 1; it != temp.end(); ++it) {
            int dist = std::abs(*it - currentHead);
            if (dist < minDistance) {
                minDistance = dist;
                closestIt = it;
            }
        }

        currentHead = *closestIt;
        seekSequence.push_back(currentHead);
        temp.erase(closestIt);
    }
}

// ==========================================
// 3. SCAN Disk Strategy
// ==========================================
void DiskSCANStrategy::schedule(int startHead, const std::vector<int>& requests, std::vector<int>& seekSequence, int totalCylinders, bool movingRight) {
    seekSequence.push_back(startHead);
    if (requests.empty()) return;

    std::vector<int> left, right;
    for (int req : requests) {
        if (req < startHead) left.push_back(req);
        else right.push_back(req);
    }

    std::sort(left.begin(), left.end());
    std::sort(right.begin(), right.end());

    if (movingRight) {
        // Service right first
        for (int r : right) seekSequence.push_back(r);
        // Hit the boundary cylinder if there is anything left
        if (!left.empty()) {
            seekSequence.push_back(totalCylinders - 1);
            // Reverse direction and service left (descending)
            for (auto it = left.rbegin(); it != left.rend(); ++it) {
                seekSequence.push_back(*it);
            }
        }
    } else {
        // Service left first (descending)
        for (auto it = left.rbegin(); it != left.rend(); ++it) {
            seekSequence.push_back(*it);
        }
        // Hit the boundary 0 if there is anything on the right
        if (!right.empty()) {
            seekSequence.push_back(0);
            // Reverse direction and service right (ascending)
            for (int r : right) seekSequence.push_back(r);
        }
    }
}

// ==========================================
// 4. C-SCAN Disk Strategy
// ==========================================
void DiskCSCANStrategy::schedule(int startHead, const std::vector<int>& requests, std::vector<int>& seekSequence, int totalCylinders, bool movingRight) {
    seekSequence.push_back(startHead);
    if (requests.empty()) return;

    std::vector<int> left, right;
    for (int req : requests) {
        if (req < startHead) left.push_back(req);
        else right.push_back(req);
    }

    std::sort(left.begin(), left.end());
    std::sort(right.begin(), right.end());

    if (movingRight) {
        for (int r : right) seekSequence.push_back(r);
        if (!left.empty()) {
            seekSequence.push_back(totalCylinders - 1);
            seekSequence.push_back(0); // Jump to beginning
            for (int l : left) seekSequence.push_back(l);
        }
    } else {
        // Moving left
        for (auto it = left.rbegin(); it != left.rend(); ++it) {
            seekSequence.push_back(*it);
        }
        if (!right.empty()) {
            seekSequence.push_back(0);
            seekSequence.push_back(totalCylinders - 1); // Jump to end
            for (auto it = right.rbegin(); it != right.rend(); ++it) {
                seekSequence.push_back(*it);
            }
        }
    }
}

// ==========================================
// 5. LOOK Disk Strategy
// ==========================================
void DiskLOOKStrategy::schedule(int startHead, const std::vector<int>& requests, std::vector<int>& seekSequence, int totalCylinders, bool movingRight) {
    (void)totalCylinders;
    seekSequence.push_back(startHead);
    if (requests.empty()) return;

    std::vector<int> left, right;
    for (int req : requests) {
        if (req < startHead) left.push_back(req);
        else right.push_back(req);
    }

    std::sort(left.begin(), left.end());
    std::sort(right.begin(), right.end());

    if (movingRight) {
        for (int r : right) seekSequence.push_back(r);
        for (auto it = left.rbegin(); it != left.rend(); ++it) {
            seekSequence.push_back(*it);
        }
    } else {
        for (auto it = left.rbegin(); it != left.rend(); ++it) {
            seekSequence.push_back(*it);
        }
        for (int r : right) seekSequence.push_back(r);
    }
}

// ==========================================
// 6. C-LOOK Disk Strategy
// ==========================================
void DiskCLOOKStrategy::schedule(int startHead, const std::vector<int>& requests, std::vector<int>& seekSequence, int totalCylinders, bool movingRight) {
    (void)totalCylinders;
    seekSequence.push_back(startHead);
    if (requests.empty()) return;

    std::vector<int> left, right;
    for (int req : requests) {
        if (req < startHead) left.push_back(req);
        else right.push_back(req);
    }

    std::sort(left.begin(), left.end());
    std::sort(right.begin(), right.end());

    if (movingRight) {
        for (int r : right) seekSequence.push_back(r);
        for (int l : left) seekSequence.push_back(l);
    } else {
        for (auto it = left.rbegin(); it != left.rend(); ++it) {
            seekSequence.push_back(*it);
        }
        for (auto it = right.rbegin(); it != right.rend(); ++it) {
            seekSequence.push_back(*it);
        }
    }
}

// ==========================================
// Disk Scheduler Factory Implementation
// ==========================================
std::unique_ptr<DiskSchedulingStrategy> DiskSchedulerFactory::createStrategy(AlgorithmType type) {
    switch (type) {
        case AlgorithmType::FCFS: return std::make_unique<DiskFCFSStrategy>();
        case AlgorithmType::SSTF: return std::make_unique<DiskSSTFStrategy>();
        case AlgorithmType::SCAN: return std::make_unique<DiskSCANStrategy>();
        case AlgorithmType::CSCAN: return std::make_unique<DiskCSCANStrategy>();
        case AlgorithmType::LOOK: return std::make_unique<DiskLOOKStrategy>();
        case AlgorithmType::CLOOK: return std::make_unique<DiskCLOOKStrategy>();
    }
    return nullptr;
}

// ==========================================
// DiskScheduler Class Implementation
// ==========================================
DiskScheduler::DiskScheduler() 
    : m_startHead(53),
      m_totalCylinders(200),
      m_initialDirectionRight(true),
      m_currentAlgo(DiskSchedulerFactory::AlgorithmType::FCFS),
      m_totalHeadMovement(0),
      m_avgSeekTime(0.0) {
    m_requests = {98, 183, 37, 122, 14, 124, 65, 67}; // Default textbook sequence
}

void DiskScheduler::setStartHead(int head) {
    m_startHead = head;
}

void DiskScheduler::setRequests(const std::vector<int>& requests) {
    m_requests = requests;
}

void DiskScheduler::setTotalCylinders(int cylinders) {
    m_totalCylinders = cylinders;
}

void DiskScheduler::setInitialDirection(bool movingRight) {
    m_initialDirectionRight = movingRight;
}

void DiskScheduler::setAlgorithm(DiskSchedulerFactory::AlgorithmType type) {
    m_currentAlgo = type;
}

void DiskScheduler::runSimulation() {
    m_seekSequence.clear();
    m_totalHeadMovement = 0;
    m_avgSeekTime = 0.0;

    auto strategy = DiskSchedulerFactory::createStrategy(m_currentAlgo);
    if (strategy) {
        strategy->schedule(m_startHead, m_requests, m_seekSequence, m_totalCylinders, m_initialDirectionRight);
        calculateMetrics();
    }
    notifyObservers();
}

void DiskScheduler::calculateMetrics() {
    if (m_seekSequence.size() < 2) return;

    m_totalHeadMovement = 0;
    for (size_t i = 0; i < m_seekSequence.size() - 1; ++i) {
        // C-SCAN / C-LOOK might have "jump" movements which represent head resetting
        // Usually, in textbook metrics, we do not count the reset jump as seek movement, 
        // OR we count it depending on the question. 
        // We will calculate standard seek movement. If a jump goes from totalCylinders-1 to 0, 
        // we can count it or skip it. Let's do standard mathematical difference:
        m_totalHeadMovement += std::abs(m_seekSequence[i+1] - m_seekSequence[i]);
    }
    
    // Average seek time (assumes 1ms per cylinder seek, which is standard for simulation)
    // Average seek steps count is requests size
    int requestCount = m_requests.size();
    m_avgSeekTime = (requestCount > 0) ? (double)m_totalHeadMovement / requestCount : 0.0;
}

const std::vector<int>& DiskScheduler::getSeekSequence() const {
    return m_seekSequence;
}

int DiskScheduler::getTotalHeadMovement() const {
    return m_totalHeadMovement;
}

double DiskScheduler::getAvgSeekTime() const {
    return m_avgSeekTime;
}

DiskSchedulerFactory::AlgorithmType DiskScheduler::getAlgorithmType() const {
    return m_currentAlgo;
}

QString DiskScheduler::getAlgorithmName() const {
    switch (m_currentAlgo) {
        case DiskSchedulerFactory::AlgorithmType::FCFS: return "FCFS";
        case DiskSchedulerFactory::AlgorithmType::SSTF: return "SSTF";
        case DiskSchedulerFactory::AlgorithmType::SCAN: return "SCAN";
        case DiskSchedulerFactory::AlgorithmType::CSCAN: return "C-SCAN";
        case DiskSchedulerFactory::AlgorithmType::LOOK: return "LOOK";
        case DiskSchedulerFactory::AlgorithmType::CLOOK: return "C-LOOK";
    }
    return "Unknown";
}
