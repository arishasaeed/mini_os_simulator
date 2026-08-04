#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QMainWindow>
#include <QListWidget>
#include <QStackedWidget>
#include <QLabel>
#include <QPushButton>

#include "../model/ProcessManager.h"
#include "../model/CpuScheduler.h"
#include "../model/MemoryManager.h"
#include "../model/PageReplacer.h"
#include "../model/DeadlockDetector.h"
#include "../model/DiskScheduler.h"
#include "../model/VirtualFileSystem.h"

class MainWindow : public QMainWindow {
    Q_OBJECT
    
private:
    QListWidget* m_sidebar;
    QStackedWidget* m_stackedWidget;
    
    // Global Header Stats
    QLabel* m_procStatsLabel;
    QLabel* m_memStatsLabel;
    QLabel* m_diskStatsLabel;
    QPushButton* m_themeBtn;

    // References to Simulation Models (for header stats)
    ProcessManager* m_procManager;
    MemoryManager* m_memoryManager;
    DiskScheduler* m_diskScheduler;
    VirtualFileSystem* m_vfs;

    void setupUI();

public:
    MainWindow(ProcessManager* pm, CpuScheduler* cs, MemoryManager* mm, PageReplacer* pr,
               DeadlockDetector* dd, DiskScheduler* ds, VirtualFileSystem* fs, QWidget* parent = nullptr);
    ~MainWindow() = default;

    // Real-time top bar update triggers
    void refreshHeaderStats();

private slots:
    void handleSidebarSelection(int row);
    void toggleTheme();
};

#endif // MAIN_WINDOW_H
