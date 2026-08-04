#ifndef MODULE_WIDGETS_H
#define MODULE_WIDGETS_H

#include <QWidget>
#include <QPainter>
#include <QPaintEvent>
#include <vector>
#include <map>
#include <QString>
#include <QPushButton>
#include <QTableWidget>
#include <QLineEdit>
#include <QSpinBox>
#include <QComboBox>
#include <QLabel>
#include <QTabWidget>
#include <QTreeView>
#include <QStandardItemModel>

#include "../common/Observer.h"
#include "../model/ProcessManager.h"
#include "../model/CpuScheduler.h"
#include "../model/MemoryManager.h"
#include "../model/PageReplacer.h"
#include "../model/DeadlockDetector.h"
#include "../model/DiskScheduler.h"
#include "../model/VirtualFileSystem.h"

// ============================================================================
// CUSTOM VISUALIZER WIDGETS (Drawing Canvas Classes)
// ============================================================================

// 1. Process State Transition diagram painter
class ProcessStateVisualizer : public QWidget {
    Q_OBJECT
private:
    ProcessState m_highlightState;
    bool m_hasActiveTransition;
    ProcessState m_fromState;
    ProcessState m_toState;

public:
    ProcessStateVisualizer(QWidget* parent = nullptr);
    void setHighlightState(ProcessState state);
    void setTransition(ProcessState from, ProcessState to);
    void clearTransition();

protected:
    void paintEvent(QPaintEvent* event) override;
};

// 2. CPU Scheduling Gantt Chart painter
class GanttChartVisualizer : public QWidget {
    Q_OBJECT
private:
    std::vector<GanttSegment> m_segments;

public:
    GanttChartVisualizer(QWidget* parent = nullptr);
    void setSegments(const std::vector<GanttSegment>& segments);

protected:
    void paintEvent(QPaintEvent* event) override;
};

// 3. Memory Allocation blocks layout painter
class MemoryBarVisualizer : public QWidget {
    Q_OBJECT
private:
    std::vector<MemoryBlock> m_blocks;
    int m_totalSize;

public:
    MemoryBarVisualizer(QWidget* parent = nullptr);
    void setBlocks(const std::vector<MemoryBlock>& blocks, int totalSize);

protected:
    void paintEvent(QPaintEvent* event) override;
};

// 4. Resource Allocation Graph painter
class RAGVisualizer : public QWidget {
    Q_OBJECT
private:
    int m_processCount;
    int m_resourceCount;
    std::vector<std::vector<int>> m_allocMatrix;
    std::vector<std::vector<int>> m_reqMatrix;
    std::vector<std::string> m_deadlockedNodes;

public:
    RAGVisualizer(QWidget* parent = nullptr);
    void setData(int pCount, int rCount, 
                 const std::vector<std::vector<int>>& alloc, 
                 const std::vector<std::vector<int>>& req,
                 const std::vector<std::string>& deadlocks);

protected:
    void paintEvent(QPaintEvent* event) override;
};

// 5. Disk Head trajectory seek graph painter
class DiskSeekVisualizer : public QWidget {
    Q_OBJECT
private:
    std::vector<int> m_sequence;
    int m_totalCylinders;

public:
    DiskSeekVisualizer(QWidget* parent = nullptr);
    void setSequence(const std::vector<int>& sequence, int cylinders);

protected:
    void paintEvent(QPaintEvent* event) override;
};

// ============================================================================
// SIMULATION MODULE PAGES (Widgets mapping to sidebars)
// ============================================================================

// Module 1: Process Management Widget
class ProcessWidget : public QWidget, public CustomDS::Observer {
    Q_OBJECT
private:
    ProcessManager* m_model;
    
    QTableWidget* m_table;
    ProcessStateVisualizer* m_visualizer;
    
    QLineEdit* m_nameInput;
    QSpinBox* m_arrivalInput;
    QSpinBox* m_burstInput;
    QSpinBox* m_priorityInput;
    QSpinBox* m_memoryInput;
    
    QPushButton* m_createBtn;
    QPushButton* m_deleteBtn;
    QPushButton* m_suspendBtn;
    QPushButton* m_resumeBtn;

    void setupUI();

public:
    ProcessWidget(ProcessManager* model, QWidget* parent = nullptr);
    ~ProcessWidget();

    void update() override; // Observer notification
};

// Module 2: CPU Scheduling Widget
class CpuSchedulerWidget : public QWidget, public CustomDS::Observer {
    Q_OBJECT
private:
    CpuScheduler* m_model;
    ProcessManager* m_procManager; // For loading processes

    QTableWidget* m_inputTable;
    QTableWidget* m_resultTable;
    GanttChartVisualizer* m_ganttVisualizer;
    
    QComboBox* m_algoSelect;
    QSpinBox* m_quantumInput;
    
    QPushButton* m_loadProcessesBtn;
    QPushButton* m_runBtn;
    
    // Stats labels
    QLabel* m_avgWaitLabel;
    QLabel* m_avgTurnLabel;
    QLabel* m_avgRespLabel;
    QLabel* m_cpuUtilLabel;
    QLabel* m_throughputLabel;

    void setupUI();

public:
    CpuSchedulerWidget(CpuScheduler* model, ProcessManager* procManager, QWidget* parent = nullptr);
    ~CpuSchedulerWidget();

    void update() override;
};

// Module 3: Memory Management Widget
class MemoryWidget : public QWidget, public CustomDS::Observer {
    Q_OBJECT
private:
    MemoryManager* m_model;
    
    QComboBox* m_typeSelect;
    QComboBox* m_algoSelect;
    QLineEdit* m_fixedSizesInput;
    QPushButton* m_initBtn;
    
    QSpinBox* m_pidInput;
    QLineEdit* m_pnameInput;
    QSpinBox* m_sizeInput;
    QPushButton* m_allocateBtn;
    QPushButton* m_deallocateBtn;
    
    QTableWidget* m_blocksTable;
    MemoryBarVisualizer* m_barVisualizer;

    QLabel* m_freeLabel;
    QLabel* m_allocatedLabel;
    QLabel* m_internalFragLabel;
    QLabel* m_externalFragLabel;

    void setupUI();

public:
    MemoryWidget(MemoryManager* model, QWidget* parent = nullptr);
    ~MemoryWidget();

    void update() override;
};

// Module 4: Page Replacement Widget
class PageWidget : public QWidget, public CustomDS::Observer {
    Q_OBJECT
private:
    PageReplacer* m_model;
    
    QLineEdit* m_refStringInput;
    QSpinBox* m_framesInput;
    QComboBox* m_algoSelect;
    QPushButton* m_runBtn;
    
    QTableWidget* m_gridTable;
    QLabel* m_faultsLabel;
    QLabel* m_hitsLabel;
    QLabel* m_faultRateLabel;

    void setupUI();

public:
    PageWidget(PageReplacer* model, QWidget* parent = nullptr);
    ~PageWidget();

    void update() override;
};

// Module 5: Deadlock Handling Widget
class DeadlockWidget : public QWidget, public CustomDS::Observer {
    Q_OBJECT
private:
    DeadlockDetector* m_model;

    QSpinBox* m_processCountInput;
    QSpinBox* m_resourceCountInput;
    QLineEdit* m_totalsInput;
    QPushButton* m_initBtn;

    QTableWidget* m_allocTable;
    QTableWidget* m_maxTable;
    QTableWidget* m_requestTable;

    QPushButton* m_runBankerBtn;
    QPushButton* m_detectDeadlockBtn;
    QSpinBox* m_terminatePidInput;
    QPushButton* m_terminateBtn;

    QLabel* m_bankersResultLabel;
    QLabel* m_deadlockStatusLabel;
    RAGVisualizer* m_ragVisualizer;

    void setupUI();
    void syncMatricesToTables();
    void readTablesToMatrices();

public:
    DeadlockWidget(DeadlockDetector* model, QWidget* parent = nullptr);
    ~DeadlockWidget();

    void update() override;
};

// Module 6: Disk Scheduling Widget
class DiskWidget : public QWidget, public CustomDS::Observer {
    Q_OBJECT
private:
    DiskScheduler* m_model;

    QSpinBox* m_startHeadInput;
    QLineEdit* m_requestsInput;
    QSpinBox* m_cylindersInput;
    QComboBox* m_directionSelect;
    QComboBox* m_algoSelect;
    QPushButton* m_runBtn;

    QLabel* m_headMovementLabel;
    QLabel* m_avgSeekLabel;
    DiskSeekVisualizer* m_seekVisualizer;

    void setupUI();

public:
    DiskWidget(DiskScheduler* model, QWidget* parent = nullptr);
    ~DiskWidget();

    void update() override;
};

// Module 7: Virtual File System Widget
class FileSystemWidget : public QWidget, public CustomDS::Observer {
    Q_OBJECT
private:
    VirtualFileSystem* m_model;

    QLabel* m_pathLabel;
    QTreeView* m_treeView;
    QStandardItemModel* m_treeModel;
    
    // Grid of 64 disk blocks
    QWidget* m_blocksContainer;
    std::vector<QLabel*> m_blockLabels;

    // Actions
    QLineEdit* m_itemNameInput;
    QSpinBox* m_fileSizeInput;
    QComboBox* m_allocMethodSelect;
    
    QPushButton* m_createFolderBtn;
    QPushButton* m_createFileBtn;
    QPushButton* m_deleteBtn;
    QLineEdit* m_renameInput;
    QPushButton* m_renameBtn;
    QLineEdit* m_searchInput;
    QPushButton* m_searchBtn;
    
    QLabel* m_freeBlocksLabel;
    QLabel* m_usedBlocksLabel;

    void setupUI();
    void rebuildTree(QStandardItem* parentItem, CustomDS::TreeNode<FileEntry>* modelNode);

public:
    FileSystemWidget(VirtualFileSystem* model, QWidget* parent = nullptr);
    ~FileSystemWidget();

    void update() override;
};

// Module 8: Performance Analytics Widget
class AnalyticsWidget : public QWidget {
    Q_OBJECT
private:
    CpuScheduler* m_cpuModel;
    PageReplacer* m_pageModel;
    DiskScheduler* m_diskModel;

    QTableWidget* m_cpuCompareTable;
    QTableWidget* m_pageCompareTable;
    QTableWidget* m_diskCompareTable;
    
    QPushButton* m_refreshBtn;

    void setupUI();
    void runComparison();

public:
    AnalyticsWidget(CpuScheduler* cpu, PageReplacer* page, DiskScheduler* disk, QWidget* parent = nullptr);
    ~AnalyticsWidget() = default;
};

// About Project Page Widget
class AboutWidget : public QWidget {
    Q_OBJECT
private:
    void setupUI();

public:
    AboutWidget(QWidget* parent = nullptr);
    ~AboutWidget() = default;
};

#endif // MODULE_WIDGETS_H
