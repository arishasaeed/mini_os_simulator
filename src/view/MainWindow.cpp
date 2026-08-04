#include "MainWindow.h"
#include "ModuleWidgets.h"
#include "../common/SystemConfig.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFrame>

MainWindow::MainWindow(ProcessManager* pm, CpuScheduler* cs, MemoryManager* mm, PageReplacer* pr,
                       DeadlockDetector* dd, DiskScheduler* ds, VirtualFileSystem* fs, QWidget* parent)
    : QMainWindow(parent),
      m_procManager(pm),
      m_memoryManager(mm),
      m_diskScheduler(ds),
      m_vfs(fs) {
    
    setupUI();
    
    // Connect theme changes
    connect(&SystemConfig::getInstance(), &SystemConfig::themeChanged, this, [this](bool dark) {
        setStyleSheet(SystemConfig::getInstance().getStylesheet());
        m_themeBtn->setText(dark ? "☀️ Light Mode" : "🌙 Dark Mode");
    });
    
    // Initial stats fill
    refreshHeaderStats();
    
    // Apply initial theme stylesheet
    setStyleSheet(SystemConfig::getInstance().getStylesheet());
}

void MainWindow::setupUI() {
    setWindowTitle("Mini Operating System Simulator - Lab Dashboard");
    resize(1200, 800);
    setMinimumSize(1000, 700);

    QWidget* centralWidget = new QWidget(this);
    QVBoxLayout* centralLayout = new QVBoxLayout(centralWidget);
    centralLayout->setContentsMargins(0, 0, 0, 0);
    centralLayout->setSpacing(0);

    // ==========================================
    // 1. TOP HEADER BAR
    // ==========================================
    QFrame* headerBar = new QFrame(this);
    headerBar->setObjectName("HeaderBar");
    headerBar->setFrameShape(QFrame::StyledPanel);
    headerBar->setStyleSheet(
        "QFrame#HeaderBar {"
        "    background-color: #0f172a;"
        "    border-bottom: 2px solid #2563eb;"
        "    min-height: 60px;"
        "    max-height: 60px;"
        "}"
    );

    QHBoxLayout* headerLayout = new QHBoxLayout(headerBar);
    headerLayout->setContentsMargins(15, 0, 15, 0);

    QLabel* logoLabel = new QLabel("⚙️ Mini OS Simulator", this);
    logoLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: #ffffff;");
    headerLayout->addWidget(logoLabel);
    headerLayout->addSpacing(30);

    // Stats indicator layout
    QHBoxLayout* statsLayout = new QHBoxLayout();
    statsLayout->setSpacing(20);

    m_procStatsLabel = new QLabel("Processes: 0", this);
    m_procStatsLabel->setStyleSheet("color: #cbd5e1; font-weight: 500; font-size: 13px;");
    statsLayout->addWidget(m_procStatsLabel);

    m_memStatsLabel = new QLabel("Free Memory: 1000 MB", this);
    m_memStatsLabel->setStyleSheet("color: #cbd5e1; font-weight: 500; font-size: 13px;");
    statsLayout->addWidget(m_memStatsLabel);

    m_diskStatsLabel = new QLabel("VFS Free: 64 Blocks", this);
    m_diskStatsLabel->setStyleSheet("color: #cbd5e1; font-weight: 500; font-size: 13px;");
    statsLayout->addWidget(m_diskStatsLabel);

    headerLayout->addLayout(statsLayout);
    headerLayout->addStretch();

    // Theme toggle button
    m_themeBtn = new QPushButton(SystemConfig::getInstance().isDarkTheme() ? "☀️ Light Mode" : "🌙 Dark Mode", this);
    m_themeBtn->setStyleSheet(
        "QPushButton {"
        "    background-color: #1e293b;"
        "    border: 1px solid #475569;"
        "    border-radius: 4px;"
        "    color: #cbd5e1;"
        "    padding: 6px 12px;"
        "}"
        "QPushButton:hover {"
        "    background-color: #334155;"
        "    color: #f8fafc;"
        "}"
    );
    headerLayout->addWidget(m_themeBtn);
    centralLayout->addWidget(headerBar);

    // ==========================================
    // 2. MAIN BODY (SIDEBAR + STACKED WORKSPACE)
    // ==========================================
    QHBoxLayout* bodyLayout = new QHBoxLayout();
    bodyLayout->setContentsMargins(0, 0, 0, 0);
    bodyLayout->setSpacing(0);

    // Navigation Sidebar
    m_sidebar = new QListWidget(this);
    m_sidebar->setObjectName("Sidebar");
    m_sidebar->setStyleSheet(
        "QListWidget#Sidebar {"
        "    background-color: #0b0f19;"
        "    border: none;"
        "    max-width: 230px;"
        "    min-width: 230px;"
        "    padding: 10px;"
        "}"
        "QListWidget#Sidebar::item {"
        "    padding: 12px 16px;"
        "    border-radius: 6px;"
        "    font-weight: 600;"
        "    color: #94a3b8;"
        "    margin-bottom: 4px;"
        "}"
        "QListWidget#Sidebar::item:hover {"
        "    background-color: #1e293b;"
        "    color: #f8fafc;"
        "}"
        "QListWidget#Sidebar::item:selected {"
        "    background-color: #2563eb;"
        "    color: #ffffff;"
        "}"
    );

    // Add Sidebar Navigation Items
    m_sidebar->addItem("📋 Process Management");
    m_sidebar->addItem("⚡ CPU Scheduling");
    m_sidebar->addItem("💾 Memory Management");
    m_sidebar->addItem("📑 Page Replacement");
    m_sidebar->addItem("🔒 Deadlock Handling");
    m_sidebar->addItem("💿 Disk Scheduling");
    m_sidebar->addItem("📁 Virtual File System");
    m_sidebar->addItem("📊 Performance Analytics");
    m_sidebar->addItem("ℹ️ About Project");

    bodyLayout->addWidget(m_sidebar);

    // Stacked Widget Workspace
    m_stackedWidget = new QStackedWidget(this);
    
    // Add pages
    m_stackedWidget->addWidget(new ProcessWidget(m_procManager, this));
    m_stackedWidget->addWidget(new CpuSchedulerWidget(m_cpuScheduler, m_procManager, this));
    m_stackedWidget->addWidget(new MemoryWidget(m_memoryManager, this));
    m_stackedWidget->addWidget(new PageWidget(m_pageReplacer, this));
    m_stackedWidget->addWidget(new DeadlockWidget(m_deadlockDetector, this));
    m_stackedWidget->addWidget(new DiskWidget(m_diskScheduler, this));
    m_stackedWidget->addWidget(new FileSystemWidget(m_vfs, this));
    m_stackedWidget->addWidget(new AnalyticsWidget(m_cpuScheduler, m_pageReplacer, m_diskScheduler, this));
    m_stackedWidget->addWidget(new AboutWidget(this));

    bodyLayout->addWidget(m_stackedWidget);
    centralLayout->addLayout(bodyLayout);

    setCentralWidget(centralWidget);

    // Event routing
    connect(m_sidebar, &QListWidget::currentRowChanged, this, &MainWindow::handleSidebarSelection);
    connect(m_themeBtn, &QPushButton::clicked, this, &MainWindow::toggleTheme);

    m_sidebar->setCurrentRow(0); // Select first page on start
}

void MainWindow::handleSidebarSelection(int row) {
    if (row >= 0 && row < m_stackedWidget->count()) {
        m_stackedWidget->setCurrentIndex(row);
        refreshHeaderStats();
    }
}

void MainWindow::toggleTheme() {
    bool current = SystemConfig::getInstance().isDarkTheme();
    SystemConfig::getInstance().setDarkTheme(!current);
}

void MainWindow::refreshHeaderStats() {
    int activeProcsCount = m_procManager->getProcesses().size();
    m_procStatsLabel->setText(QString("Processes: %1").arg(activeProcsCount));

    int freeMem = m_memoryManager->getFreeMemory();
    m_memStatsLabel->setText(QString("Free Memory: %1 MB").arg(freeMem));

    int freeBlocks = m_vfs->getFreeBlockCount();
    m_diskStatsLabel->setText(QString("VFS Free: %1 / 64 Blocks").arg(freeBlocks));
}
