#ifndef VIRTUAL_FILE_SYSTEM_H
#define VIRTUAL_FILE_SYSTEM_H

#include <vector>
#include <QString>
#include <memory>
#include "../common/Subject.h"
#include "../common/DataStructures.h"

struct FileEntry {
    QString name;
    bool isDirectory;
    int sizeInBlocks;                 // 0 for directories
    QString allocationMethod;         // "Sequential", "Linked", "Indexed"
    int startBlock;                   // Sequential and Linked
    int endBlock;                     // Linked
    int indexBlock;                   // Indexed
    std::vector<int> allocatedBlocks; // List of data blocks
};

class VirtualFileSystem : public CustomDS::Subject {
private:
    // Tree Node storing FileEntry
    CustomDS::TreeNode<FileEntry>* m_rootDir;
    CustomDS::TreeNode<FileEntry>* m_currentDir;
    
    // Virtual Disk State (Total 64 blocks)
    int m_diskBlockCount;
    std::vector<QString> m_diskBlocks; // Holds file name occupying block, "" if free

    // Helper block allocators
    bool allocateSequential(int size, std::vector<int>& allocated, int& start);
    bool allocateLinked(int size, std::vector<int>& allocated, int& start, int& end);
    bool allocateIndexed(int size, std::vector<int>& allocated, int& indexBlock);

    void freeBlocks(const std::vector<int>& blocks);
    CustomDS::TreeNode<FileEntry>* findNodeByName(CustomDS::TreeNode<FileEntry>* parent, const QString& name) const;
    void searchRecursive(CustomDS::TreeNode<FileEntry>* startNode, const QString& query, std::vector<QString>& results, const QString& currentPath) const;

public:
    VirtualFileSystem();
    ~VirtualFileSystem();

    void initializeDisk(int blockCount = 64);
    
    // Virtual Filesystem Operations
    bool createDirectory(const QString& name);
    bool createFile(const QString& name, int sizeInBlocks, const QString& allocationMethod);
    bool deleteItem(const QString& name);
    bool renameItem(const QString& oldName, const QString& newName);
    std::vector<QString> searchFiles(const QString& query) const;
    
    // Navigation
    bool changeDirectory(const QString& name);
    void navigateToParent();
    QString getCurrentPath() const;

    // Getters for Stats and UI
    CustomDS::TreeNode<FileEntry>* getRootNode() const;
    CustomDS::TreeNode<FileEntry>* getCurrentNode() const;
    const std::vector<QString>& getDiskBlocks() const;
    int getFreeBlockCount() const;
    int getUsedBlockCount() const;

    void reset();
};

#endif // VIRTUAL_FILE_SYSTEM_H
