#ifndef MEMORY_MANAGER_H
#define MEMORY_MANAGER_H

#include <vector>
#include <QString>
#include "../common/Subject.h"

struct MemoryBlock {
    int id;
    int startAddress;
    int size;
    bool isAllocated;
    int allocatedPid;
    QString allocatedPName;
    int internalFragmentation;
    
    int endAddress() const {
        return startAddress + size - 1;
    }
};

class MemoryManager : public CustomDS::Subject {
public:
    enum class PartitionType {
        FIXED,
        DYNAMIC
    };

    enum class AllocationAlgorithm {
        FIRST_FIT,
        BEST_FIT,
        WORST_FIT,
        NEXT_FIT
    };

private:
    PartitionType m_type;
    std::vector<MemoryBlock> m_blocks;
    int m_totalMemorySize;
    int m_nextBlockId;
    int m_nextFitLastIndex; // Tracks last allocated block for Next Fit

    void coalesceDynamicBlocks();

public:
    MemoryManager();
    ~MemoryManager() = default;

    void initializeMemory(PartitionType type, const std::vector<int>& partitionSizes = {});
    
    // Core Simulation Actions
    bool allocateProcess(int pid, const QString& name, int size, AllocationAlgorithm algo);
    bool deallocateProcess(int pid);
    void reset();

    // Getters for Stats
    const std::vector<MemoryBlock>& getBlocks() const;
    PartitionType getPartitionType() const;
    int getTotalMemorySize() const;
    int getFreeMemory() const;
    int getAllocatedMemory() const;
    int getInternalFragmentation() const;
    int getExternalFragmentation(int requestedSize) const;
};

#endif // MEMORY_MANAGER_H
