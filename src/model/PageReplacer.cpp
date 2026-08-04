#include "PageReplacer.h"
#include <algorithm>
#include <map>

PageReplacer::PageReplacer() 
    : m_frameCount(3),
      m_algorithm(Algorithm::FIFO),
      m_pageFaults(0),
      m_pageHits(0) {
    m_referenceString = {7, 0, 1, 2, 0, 3, 0, 4, 2, 3, 0, 3, 2, 1, 2, 0, 1, 7, 0, 1}; // Default textbook string
}

void PageReplacer::setReferenceString(const std::vector<int>& refString) {
    m_referenceString = refString;
}

void PageReplacer::setFrameCount(int count) {
    m_frameCount = std::max(1, count);
}

void PageReplacer::setAlgorithm(Algorithm algo) {
    m_algorithm = algo;
}

void PageReplacer::runSimulation() {
    m_steps.clear();
    m_pageFaults = 0;
    m_pageHits = 0;

    if (m_referenceString.empty()) {
        notifyObservers();
        return;
    }

    switch (m_algorithm) {
        case Algorithm::FIFO: runFIFO(); break;
        case Algorithm::LRU:  runLRU();  break;
        case Algorithm::OPTIMAL: runOptimal(); break;
    }
    notifyObservers();
}

void PageReplacer::runFIFO() {
    std::vector<int> currentFrames(m_frameCount, -1);
    int fifoPointer = 0; // Circular pointer to replace

    for (size_t i = 0; i < m_referenceString.size(); ++i) {
        int page = m_referenceString[i];
        bool isHit = false;

        // Check if page already in frames
        for (int framePage : currentFrames) {
            if (framePage == page) {
                isHit = true;
                break;
            }
        }

        if (isHit) {
            m_pageHits++;
        } else {
            m_pageFaults++;
            // Try to find an empty slot first
            int emptyIdx = -1;
            for (int k = 0; k < m_frameCount; ++k) {
                if (currentFrames[k] == -1) {
                    emptyIdx = k;
                    break;
                }
            }

            if (emptyIdx != -1) {
                currentFrames[emptyIdx] = page;
            } else {
                // Replace using FIFO pointer
                currentFrames[fifoPointer] = page;
                fifoPointer = (fifoPointer + 1) % m_frameCount;
            }
        }

        m_steps.push_back({page, currentFrames, isHit, m_pageFaults, m_pageHits});
    }
}

void PageReplacer::runLRU() {
    std::vector<int> currentFrames(m_frameCount, -1);
    std::map<int, int> lastUsed; // Tracks last index used for each page in frames

    for (size_t i = 0; i < m_referenceString.size(); ++i) {
        int page = m_referenceString[i];
        bool isHit = false;

        // Check hit
        for (int framePage : currentFrames) {
            if (framePage == page) {
                isHit = true;
                break;
            }
        }

        if (isHit) {
            m_pageHits++;
            lastUsed[page] = i; // Update last used time
        } else {
            m_pageFaults++;
            int emptyIdx = -1;
            for (int k = 0; k < m_frameCount; ++k) {
                if (currentFrames[k] == -1) {
                    emptyIdx = k;
                    break;
                }
            }

            if (emptyIdx != -1) {
                currentFrames[emptyIdx] = page;
                lastUsed[page] = i;
            } else {
                // Find least recently used page in frames
                int lruIdx = 0;
                int oldestTime = 1e9;
                for (int k = 0; k < m_frameCount; ++k) {
                    int p = currentFrames[k];
                    if (lastUsed[p] < oldestTime) {
                        oldestTime = lastUsed[p];
                        lruIdx = k;
                    }
                }
                
                // Remove oldest from map
                lastUsed.erase(currentFrames[lruIdx]);
                
                // Replace
                currentFrames[lruIdx] = page;
                lastUsed[page] = i;
            }
        }

        m_steps.push_back({page, currentFrames, isHit, m_pageFaults, m_pageHits});
    }
}

void PageReplacer::runOptimal() {
    std::vector<int> currentFrames(m_frameCount, -1);

    for (size_t i = 0; i < m_referenceString.size(); ++i) {
        int page = m_referenceString[i];
        bool isHit = false;

        // Check hit
        for (int framePage : currentFrames) {
            if (framePage == page) {
                isHit = true;
                break;
            }
        }

        if (isHit) {
            m_pageHits++;
        } else {
            m_pageFaults++;
            int emptyIdx = -1;
            for (int k = 0; k < m_frameCount; ++k) {
                if (currentFrames[k] == -1) {
                    emptyIdx = k;
                    break;
                }
            }

            if (emptyIdx != -1) {
                currentFrames[emptyIdx] = page;
            } else {
                // Predict the future: find which frame page is used farthest in future
                int optIdx = -1;
                int farthestOccur = -1;

                for (int k = 0; k < m_frameCount; ++k) {
                    int p = currentFrames[k];
                    int nextOccur = 1e9; // If not used, represents infinity

                    for (size_t nextIdx = i + 1; nextIdx < m_referenceString.size(); ++nextIdx) {
                        if (m_referenceString[nextIdx] == p) {
                            nextOccur = nextIdx;
                            break;
                        }
                    }

                    if (nextOccur > farthestOccur) {
                        farthestOccur = nextOccur;
                        optIdx = k;
                    }
                }
                currentFrames[optIdx] = page;
            }
        }

        m_steps.push_back({page, currentFrames, isHit, m_pageFaults, m_pageHits});
    }
}

const std::vector<PageStep>& PageReplacer::getSteps() const {
    return m_steps;
}

int PageReplacer::getPageFaults() const {
    return m_pageFaults;
}

int PageReplacer::getPageHits() const {
    return m_pageHits;
}

double PageReplacer::getFaultRate() const {
    if (m_referenceString.empty()) return 0.0;
    return (double)m_pageFaults / m_referenceString.size();
}

QString PageReplacer::getAlgorithmName() const {
    switch (m_algorithm) {
        case Algorithm::FIFO: return "FIFO";
        case Algorithm::LRU:  return "LRU";
        case Algorithm::OPTIMAL: return "Optimal";
    }
    return "Unknown";
}
