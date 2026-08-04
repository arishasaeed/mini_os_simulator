#include "AppController.h"

AppController::AppController() {
    // 1. Create Model Instances
    m_procManager = std::make_unique<ProcessManager>();
    m_cpuScheduler = std::make_unique<CpuScheduler>();
    m_memoryManager = std::make_unique<MemoryManager>();
    m_pageReplacer = std::make_unique<PageReplacer>();
    m_deadlockDetector = std::make_unique<DeadlockDetector>();
    m_diskScheduler = std::make_unique<DiskScheduler>();
    m_vfs = std::make_unique<VirtualFileSystem>();

    // 2. Create Main Window Shell
    m_mainWindow = std::make_unique<MainWindow>(
        m_procManager.get(),
        m_cpuScheduler.get(),
        m_memoryManager.get(),
        m_pageReplacer.get(),
        m_deadlockDetector.get(),
        m_diskScheduler.get(),
        m_vfs.get()
    );

    registerObservers();
}

AppController::~AppController() {
    // Unsubscribe observers on teardown to prevent hanging references
    m_procManager->removeObserver(this);
    m_memoryManager->removeObserver(this);
    m_vfs->removeObserver(this);
}

void AppController::registerObservers() {
    // Subscribe to models that modify dashboard header stats in real-time
    m_procManager->addObserver(this);
    m_memoryManager->addObserver(this);
    m_vfs->addObserver(this);
}

void AppController::start() {
    m_mainWindow->show();
}

void AppController::update() {
    // Triggered when Subject notifyObservers is called
    if (m_mainWindow) {
        m_mainWindow->refreshHeaderStats();
    }
}
