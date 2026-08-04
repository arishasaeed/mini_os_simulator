#ifndef PAGE_REPLACER_H
#define PAGE_REPLACER_H

#include <vector>
#include <QString>
#include "../common/Subject.h"

// Struct storing simulation state at each character in the reference string
struct PageStep {
    int page;
    std::vector<int> frames; // Current pages in frames (-1 for empty)
    bool isHit;
    int faultCount;
    int hitCount;
};

class PageReplacer : public CustomDS::Subject {
public:
    enum class Algorithm {
        FIFO,
        LRU,
        OPTIMAL
    };

private:
    std::vector<int> m_referenceString;
    int m_frameCount;
    Algorithm m_algorithm;

    std::vector<PageStep> m_steps;
    int m_pageFaults;
    int m_pageHits;

    // Simulation Runners
    void runFIFO();
    void runLRU();
    void runOptimal();

public:
    PageReplacer();
    ~PageReplacer() = default;

    void setReferenceString(const std::vector<int>& refString);
    void setFrameCount(int count);
    void setAlgorithm(Algorithm algo);

    void runSimulation();

    // Getters for Results
    const std::vector<PageStep>& getSteps() const;
    int getPageFaults() const;
    int getPageHits() const;
    double getFaultRate() const;
    QString getAlgorithmName() const;
};

#endif // PAGE_REPLACER_H
