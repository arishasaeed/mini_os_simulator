#include "VirtualFileSystem.h"
#include <algorithm>

VirtualFileSystem::VirtualFileSystem() : m_rootDir(nullptr), m_currentDir(nullptr) {
    initializeDisk(64);
    reset();
}

VirtualFileSystem::~VirtualFileSystem() {
    delete m_rootDir;
}

void VirtualFileSystem::initializeDisk(int blockCount) {
    m_diskBlockCount = blockCount;
    m_diskBlocks.assign(m_diskBlockCount, "");
}

bool VirtualFileSystem::allocateSequential(int size, std::vector<int>& allocated, int& start) {
    int contiguousFree = 0;
    int candidateStart = -1;

    for (int i = 0; i < m_diskBlockCount; ++i) {
        if (m_diskBlocks[i].isEmpty()) {
            if (contiguousFree == 0) {
                candidateStart = i;
            }
            contiguousFree++;
            if (contiguousFree == size) {
                break;
            }
        } else {
            contiguousFree = 0;
            candidateStart = -1;
        }
    }

    if (contiguousFree == size && candidateStart != -1) {
        start = candidateStart;
        for (int i = 0; i < size; ++i) {
            allocated.push_back(start + i);
        }
        return true;
    }
    return false;
}

bool VirtualFileSystem::allocateLinked(int size, std::vector<int>& allocated, int& start, int& end) {
    int freeCount = getFreeBlockCount();
    if (freeCount < size) return false;

    for (int i = 0; i < m_diskBlockCount && (int)allocated.size() < size; ++i) {
        if (m_diskBlocks[i].isEmpty()) {
            allocated.push_back(i);
        }
    }

    if ((int)allocated.size() == size) {
        start = allocated.front();
        end = allocated.back();
        return true;
    }
    allocated.clear();
    return false;
}

bool VirtualFileSystem::allocateIndexed(int size, std::vector<int>& allocated, int& indexBlock) {
    // Requires size + 1 free blocks
    int freeCount = getFreeBlockCount();
    if (freeCount < size + 1) return false;

    std::vector<int> gathered;
    for (int i = 0; i < m_diskBlockCount && (int)gathered.size() < size + 1; ++i) {
        if (m_diskBlocks[i].isEmpty()) {
            gathered.push_back(i);
        }
    }

    if ((int)gathered.size() == size + 1) {
        indexBlock = gathered.front();
        for (size_t i = 1; i < gathered.size(); ++i) {
            allocated.push_back(gathered[i]);
        }
        return true;
    }
    return false;
}

void VirtualFileSystem::freeBlocks(const std::vector<int>& blocks) {
    for (int b : blocks) {
        if (b >= 0 && b < m_diskBlockCount) {
            m_diskBlocks[b] = "";
        }
    }
}

CustomDS::TreeNode<FileEntry>* VirtualFileSystem::findNodeByName(CustomDS::TreeNode<FileEntry>* parent, const QString& name) const {
    if (!parent) return nullptr;
    for (auto* child : parent->children) {
        if (child->data.name == name) {
            return child;
        }
    }
    return nullptr;
}

bool VirtualFileSystem::createDirectory(const QString& name) {
    if (name.isEmpty() || findNodeByName(m_currentDir, name)) return false;

    FileEntry entry = {name, true, 0, "", -1, -1, -1, {}};
    auto* newNode = new CustomDS::TreeNode<FileEntry>(entry);
    m_currentDir->addChild(newNode);
    notifyObservers();
    return true;
}

bool VirtualFileSystem::createFile(const QString& name, int sizeInBlocks, const QString& allocationMethod) {
    if (name.isEmpty() || sizeInBlocks <= 0 || findNodeByName(m_currentDir, name)) return false;

    std::vector<int> allocated;
    int start = -1, end = -1, index = -1;
    bool success = false;

    if (allocationMethod == "Sequential") {
        success = allocateSequential(sizeInBlocks, allocated, start);
    } else if (allocationMethod == "Linked") {
        success = allocateLinked(sizeInBlocks, allocated, start, end);
    } else if (allocationMethod == "Indexed") {
        success = allocateIndexed(sizeInBlocks, allocated, index);
    }

    if (success) {
        // Mark disk blocks
        QString fileLabel = name + " (" + allocationMethod[0] + ")";
        for (int b : allocated) {
            m_diskBlocks[b] = fileLabel;
        }
        if (index != -1) {
            m_diskBlocks[index] = name + " (IDX)";
        }

        FileEntry entry = {name, false, sizeInBlocks, allocationMethod, start, end, index, allocated};
        auto* newNode = new CustomDS::TreeNode<FileEntry>(entry);
        m_currentDir->addChild(newNode);
        
        notifyObservers();
        return true;
    }
    return false;
}

// Helper to recursively deallocate children of directory
void deallocateRecursive(CustomDS::TreeNode<FileEntry>* node, std::vector<QString>& diskBlocks) {
    if (!node) return;
    
    // Deallocate this node if file
    if (!node->data.isDirectory) {
        for (int b : node->data.allocatedBlocks) {
            diskBlocks[b] = "";
        }
        if (node->data.indexBlock != -1) {
            diskBlocks[node->data.indexBlock] = "";
        }
    }

    // Recurse children
    for (auto* child : node->children) {
        deallocateRecursive(child, diskBlocks);
    }
}

bool VirtualFileSystem::deleteItem(const QString& name) {
    auto* node = findNodeByName(m_currentDir, name);
    if (!node) return false;

    // Free disk blocks
    deallocateRecursive(node, m_diskBlocks);

    // Remove node from tree
    m_currentDir->removeChild(node);
    delete node; // Destructor deletes children recursively

    notifyObservers();
    return true;
}

bool VirtualFileSystem::renameItem(const QString& oldName, const QString& newName) {
    if (newName.isEmpty() || findNodeByName(m_currentDir, newName)) return false;
    auto* node = findNodeByName(m_currentDir, oldName);
    if (!node) return false;

    QString oldLabel = oldName + " (" + node->data.allocationMethod[0] + ")";
    QString newLabel = newName + " (" + node->data.allocationMethod[0] + ")";
    QString oldIdxLabel = oldName + " (IDX)";
    QString newIdxLabel = newName + " (IDX)";

    // Update names in disk blocks
    for (int i = 0; i < m_diskBlockCount; ++i) {
        if (m_diskBlocks[i] == oldLabel) {
            m_diskBlocks[i] = newLabel;
        } else if (m_diskBlocks[i] == oldIdxLabel) {
            m_diskBlocks[i] = newIdxLabel;
        }
    }

    node->data.name = newName;
    notifyObservers();
    return true;
}

void VirtualFileSystem::searchRecursive(CustomDS::TreeNode<FileEntry>* startNode, const QString& query, std::vector<QString>& results, const QString& currentPath) const {
    if (!startNode) return;
    for (auto* child : startNode->children) {
        QString path = currentPath + (currentPath == "/" ? "" : "/") + child->data.name;
        if (child->data.name.contains(query, Qt::CaseInsensitive)) {
            QString details = child->data.isDirectory ? " [Folder]" : QString(" [File, %1, %2 blocks]").arg(child->data.allocationMethod).arg(child->data.sizeInBlocks);
            results.push_back(path + details);
        }
        if (child->data.isDirectory) {
            searchRecursive(child, query, results, path);
        }
    }
}

std::vector<QString> VirtualFileSystem::searchFiles(const QString& query) const {
    std::vector<QString> results;
    searchRecursive(m_rootDir, query, results, "/");
    return results;
}

bool VirtualFileSystem::changeDirectory(const QString& name) {
    if (name == "..") {
        navigateToParent();
        return true;
    }
    auto* node = findNodeByName(m_currentDir, name);
    if (node && node->data.isDirectory) {
        m_currentDir = node;
        notifyObservers();
        return true;
    }
    return false;
}

void VirtualFileSystem::navigateToParent() {
    if (m_currentDir->parent) {
        m_currentDir = m_currentDir->parent;
        notifyObservers();
    }
}

QString VirtualFileSystem::getCurrentPath() const {
    if (m_currentDir == m_rootDir) return "/";

    std::vector<QString> pathParts;
    auto* curr = m_currentDir;
    while (curr != m_rootDir) {
        pathParts.push_back(curr->data.name);
        curr = curr->parent;
    }

    QString path = "";
    for (auto it = pathParts.rbegin(); it != pathParts.rend(); ++it) {
        path += "/" + *it;
    }
    return path;
}

CustomDS::TreeNode<FileEntry>* VirtualFileSystem::getRootNode() const { return m_rootDir; }
CustomDS::TreeNode<FileEntry>* VirtualFileSystem::getCurrentNode() const { return m_currentDir; }
const std::vector<QString>& VirtualFileSystem::getDiskBlocks() const { return m_diskBlocks; }

int VirtualFileSystem::getFreeBlockCount() const {
    int freeCount = 0;
    for (const auto& block : m_diskBlocks) {
        if (block.isEmpty()) freeCount++;
    }
    return freeCount;
}

int VirtualFileSystem::getUsedBlockCount() const {
    return m_diskBlockCount - getFreeBlockCount();
}

void VirtualFileSystem::reset() {
    delete m_rootDir;
    FileEntry entry = {"/", true, 0, "", -1, -1, -1, {}};
    m_rootDir = new CustomDS::TreeNode<FileEntry>(entry);
    m_currentDir = m_rootDir;
    
    initializeDisk(64);
    notifyObservers();
}
