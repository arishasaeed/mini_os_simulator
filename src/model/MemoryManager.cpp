#include "MemoryManager.h"
#include <algorithm>

MemoryManager::MemoryManager() 
    : m_type(PartitionType::FIXED), 
      m_totalMemorySize(1000), 
      m_nextBlockId(1), 
      m_nextFitLastIndex(0) {
    initializeMemory(PartitionType::FIXED);
}

void MemoryManager::initializeMemory(PartitionType type, const std::vector<int>& partitionSizes) {
    m_type = type;
    m_blocks.clear();
    m_nextBlockId = 1;
    m_nextFitLastIndex = 0;

    if (m_type == PartitionType::FIXED) {
        std::vector<int> sizes = partitionSizes;
        if (sizes.empty()) {
            sizes = {100, 150, 200, 250, 300}; // Default partition sizes
        }
        
        int currentAddr = 0;
        for (int sz : sizes) {
            MemoryBlock block;
            block.id = m_nextBlockId++;
            block.startAddress = currentAddr;
            block.size = sz;
            block.isAllocated = false;
            block.allocatedPid = 0;
            block.allocatedPName = "";
            block.internalFragmentation = 0;

            m_blocks.push_back(block);
            currentAddr += sz;
        }
        m_totalMemorySize = currentAddr;
    } else {
        // Dynamic Partitioning starts with one large free block of size 1000
        m_totalMemorySize = 1000;
        MemoryBlock block;
        block.id = m_nextBlockId++;
        block.startAddress = 0;
        block.size = m_totalMemorySize;
        block.isAllocated = false;
        block.allocatedPid = 0;
        block.allocatedPName = "";
        block.internalFragmentation = 0;

        m_blocks.push_back(block);
    }
    notifyObservers();
}

bool MemoryManager::allocateProcess(int pid, const QString& name, int size, AllocationAlgorithm algo) {
    if (m_blocks.empty() || size <= 0) return false;

    // Check if process already allocated
    for (const auto& block : m_blocks) {
        if (block.isAllocated && block.allocatedPid == pid) {
            return false;
        }
    }

    int n = m_blocks.size();
    int chosenIdx = -1;

    if (m_type == PartitionType::FIXED) {
        // ==========================================
        // FIXED ALLOCATION
        // ==========================================
        if (algo == AllocationAlgorithm::FIRST_FIT) {
            for (int i = 0; i < n; ++i) {
                if (!m_blocks[i].isAllocated && m_blocks[i].size >= size) {
                    chosenIdx = i;
                    break;
                }
            }
        } else if (algo == AllocationAlgorithm::BEST_FIT) {
            int minDifference = 1e9;
            for (int i = 0; i < n; ++i) {
                if (!m_blocks[i].isAllocated && m_blocks[i].size >= size) {
                    int diff = m_blocks[i].size - size;
                    if (diff < minDifference) {
                        minDifference = diff;
                        chosenIdx = i;
                    }
                }
            }
        } else if (algo == AllocationAlgorithm::WORST_FIT) {
            int maxDifference = -1;
            for (int i = 0; i < n; ++i) {
                if (!m_blocks[i].isAllocated && m_blocks[i].size >= size) {
                    int diff = m_blocks[i].size - size;
                    if (diff > maxDifference) {
                        maxDifference = diff;
                        chosenIdx = i;
                    }
                }
            }
        } else if (algo == AllocationAlgorithm::NEXT_FIT) {
            for (int count = 0; count < n; ++count) {
                int i = (m_nextFitLastIndex + count) % n;
                if (!m_blocks[i].isAllocated && m_blocks[i].size >= size) {
                    chosenIdx = i;
                    m_nextFitLastIndex = i;
                    break;
                }
            }
        }

        if (chosenIdx != -1) {
            m_blocks[chosenIdx].isAllocated = true;
            m_blocks[chosenIdx].allocatedPid = pid;
            m_blocks[chosenIdx].allocatedPName = name;
            m_blocks[chosenIdx].internalFragmentation = m_blocks[chosenIdx].size - size;
            notifyObservers();
            return true;
        }

    } else {
        // ==========================================
        // DYNAMIC ALLOCATION
        // ==========================================
        if (algo == AllocationAlgorithm::FIRST_FIT) {
            for (int i = 0; i < n; ++i) {
                if (!m_blocks[i].isAllocated && m_blocks[i].size >= size) {
                    chosenIdx = i;
                    break;
                }
            }
        } else if (algo == AllocationAlgorithm::BEST_FIT) {
            int minSize = 1e9;
            for (int i = 0; i < n; ++i) {
                if (!m_blocks[i].isAllocated && m_blocks[i].size >= size) {
                    if (m_blocks[i].size < minSize) {
                        minSize = m_blocks[i].size;
                        chosenIdx = i;
                    }
                }
            }
        } else if (algo == AllocationAlgorithm::WORST_FIT) {
            int maxSize = -1;
            for (int i = 0; i < n; ++i) {
                if (!m_blocks[i].isAllocated && m_blocks[i].size >= size) {
                    if (m_blocks[i].size > maxSize) {
                        maxSize = m_blocks[i].size;
                        chosenIdx = i;
                    }
                }
            }
        } else if (algo == AllocationAlgorithm::NEXT_FIT) {
            for (int count = 0; count < n; ++count) {
                int i = (m_nextFitLastIndex + count) % n;
                if (!m_blocks[i].isAllocated && m_blocks[i].size >= size) {
                    chosenIdx = i;
                    m_nextFitLastIndex = i;
                    break;
                }
            }
        }

        if (chosenIdx != -1) {
            MemoryBlock& block = m_blocks[chosenIdx];
            
            // If block is larger than needed, split it
            if (block.size > size) {
                MemoryBlock freeBlock;
                freeBlock.id = m_nextBlockId++;
                freeBlock.startAddress = block.startAddress + size;
                freeBlock.size = block.size - size;
                freeBlock.isAllocated = false;
                freeBlock.allocatedPid = 0;
                freeBlock.allocatedPName = "";
                freeBlock.internalFragmentation = 0;

                block.size = size;
                
                // Insert free block right after chosen allocated block
                m_blocks.insert(m_blocks.begin() + chosenIdx + 1, freeBlock);
            }

            block.isAllocated = true;
            block.allocatedPid = pid;
            block.allocatedPName = name;
            block.internalFragmentation = 0; // Dynamic allocation has no internal fragmentation
            
            notifyObservers();
            return true;
        }
    }

    return false;
}

bool MemoryManager::deallocateProcess(int pid) {
    bool deallocated = false;
    for (auto& block : m_blocks) {
        if (block.isAllocated && block.allocatedPid == pid) {
            block.isAllocated = false;
            block.allocatedPid = 0;
            block.allocatedPName = "";
            block.internalFragmentation = 0;
            deallocated = true;
            break; // A process should only occupy one block
        }
    }

    if (deallocated) {
        if (m_type == PartitionType::DYNAMIC) {
            coalesceDynamicBlocks();
        }
        notifyObservers();
        return true;
    }
    return false;
}

void MemoryManager::coalesceDynamicBlocks() {
    if (m_blocks.size() < 2) return;

    for (size_t i = 0; i < m_blocks.size() - 1; ) {
        if (!m_blocks[i].isAllocated && !m_blocks[i+1].isAllocated) {
            // Merge block i+1 into block i
            m_blocks[i].size += m_blocks[i+1].size;
            m_blocks.erase(m_blocks.begin() + i + 1);
            
            // Adjust Next Fit index pointer
            if (m_nextFitLastIndex > (int)i) {
                m_nextFitLastIndex = std::max(0, m_nextFitLastIndex - 1);
            }
        } else {
            ++i;
        }
    }
}

void MemoryManager::reset() {
    initializeMemory(m_type);
}

const std::vector<MemoryBlock>& MemoryManager::getBlocks() const {
    return m_blocks;
}

MemoryManager::PartitionType MemoryManager::getPartitionType() const {
    return m_type;
}

int MemoryManager::getTotalMemorySize() const {
    return m_totalMemorySize;
}

int MemoryManager::getFreeMemory() const {
    int freeMem = 0;
    for (const auto& block : m_blocks) {
        if (!block.isAllocated) {
            freeMem += block.size;
        }
    }
    return freeMem;
}

int MemoryManager::getAllocatedMemory() const {
    int allocMem = 0;
    for (const auto& block : m_blocks) {
        if (block.isAllocated) {
            allocMem += block.size;
        }
    }
    return allocMem;
}

int MemoryManager::getInternalFragmentation() const {
    int totalFrag = 0;
    for (const auto& block : m_blocks) {
        if (block.isAllocated) {
            totalFrag += block.internalFragmentation;
        }
    }
    return totalFrag;
}

int MemoryManager::getExternalFragmentation(int requestedSize) const {
    if (m_type == PartitionType::FIXED) return 0; // External fragmentation is a dynamic partitioning concept
    
    // External fragmentation happens when there is enough total free space,
    // but no single contiguous free block is large enough for the process
    int totalFree = getFreeMemory();
    if (totalFree >= requestedSize) {
        // Check if any single free block is >= requestedSize
        bool hasLargeEnoughBlock = false;
        for (const auto& block : m_blocks) {
            if (!block.isAllocated && block.size >= requestedSize) {
                hasLargeEnoughBlock = true;
                break;
            }
        }
        if (!hasLargeEnoughBlock) {
            return totalFree;
        }
    }
    return 0;
}
