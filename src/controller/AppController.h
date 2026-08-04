#ifndef APP_CONTROLLER_H
#define APP_CONTROLLER_H

#include <QObject>
#include <memory>

#include "../common/Observer.h"
#include "../model/ProcessManager.h"
#include "../model/CpuScheduler.h"
#include "../model/MemoryManager.h"
#include "../model/PageReplacer.h"
#include "../model/DeadlockDetector.h"
#include "../model/DiskScheduler.h"
#include "../model/VirtualFileSystem.h"
#include "../view/MainWindow.h"

// Controller in the MVC architecture
class AppController : public QObject, public CustomDS::Observer {
    Q_OBJECT
    
private:
    // Model Ownership
    std::unique_ptr<ProcessManager> m_procManager;
    std::unique_ptr<CpuScheduler> m_cpuScheduler;
    std::unique_ptr<MemoryManager> m_memoryManager;
    std::unique_ptr<PageReplacer> m_pageReplacer;
    std::unique_ptr<DeadlockDetector> m_deadlockDetector;
    std::unique_ptr<DiskScheduler> m_diskScheduler;
    std::unique_ptr<VirtualFileSystem> m_vfs;

    // View Ownership
    std::unique_ptr<MainWindow> m_mainWindow;

    void registerObservers();

public:
    AppController();
    ~AppController();

    // Start UI dashboard loop
    void start();

    // Catch subject notifications
    void update() override;
};

#endif // APP_CONTROLLER_H
