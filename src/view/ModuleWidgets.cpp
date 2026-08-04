#include "ModuleWidgets.h"
#include "../common/SystemConfig.h"
#include <QHeaderView>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QMessageBox>
#include <QFileDialog>
#include <QScrollArea>
#include <cmath>

// ============================================================================
// CUSTOM VISUALIZER PAINT EVENT IMPLEMENTATIONS
// ============================================================================

// 1. ProcessStateVisualizer
ProcessStateVisualizer::ProcessStateVisualizer(QWidget* parent)
    : QWidget(parent), m_highlightState(ProcessState::NEW), m_hasActiveTransition(false) {
    setMinimumHeight(240);
}

void ProcessStateVisualizer::setHighlightState(ProcessState state) {
    m_highlightState = state;
    update();
}

void ProcessStateVisualizer::setTransition(ProcessState from, ProcessState to) {
    m_hasActiveTransition = true;
    m_fromState = from;
    m_toState = to;
    update();
}

void ProcessStateVisualizer::clearTransition() {
    m_hasActiveTransition = false;
    update();
}

void ProcessStateVisualizer::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    bool isDark = SystemConfig::getInstance().isDarkTheme();
    QColor bgColor = isDark ? QColor("#1e293b") : QColor("#ffffff");
    QColor textColor = isDark ? QColor("#f8fafc") : QColor("#0f172a");
    QColor nodeColor = isDark ? QColor("#334155") : QColor("#e2e8f0");
    QColor nodeBorderColor = isDark ? QColor("#475569") : QColor("#cbd5e1");
    QColor highlightColor = QColor("#3b82f6"); // Blue
    QColor waitingColor = QColor("#eab308");   // Yellow

    // Fill background
    painter.fillRect(rect(), bgColor);
    painter.setPen(nodeBorderColor);
    painter.drawRect(0, 0, width() - 1, height() - 1);

    // Define Node Centers
    std::map<ProcessState, QPoint> centers;
    centers[ProcessState::NEW] = QPoint(60, 100);
    centers[ProcessState::READY] = QPoint(180, 100);
    centers[ProcessState::RUNNING] = QPoint(300, 100);
    centers[ProcessState::WAITING] = QPoint(240, 190);
    centers[ProcessState::TERMINATED] = QPoint(420, 100);

    // Node Names
    std::map<ProcessState, QString> names;
    names[ProcessState::NEW] = "NEW";
    names[ProcessState::READY] = "READY";
    names[ProcessState::RUNNING] = "RUNNING";
    names[ProcessState::WAITING] = "WAITING";
    names[ProcessState::TERMINATED] = "TERM";

    int radius = 26;

    // Helper: Draw Arrow
    auto drawArrow = [&](QPoint p1, QPoint p2, bool active) {
        double angle = std::atan2(p2.y() - p1.y(), p2.x() - p1.x());
        // Shorten to touch node boundaries
        QPoint start = p1 + QPoint(radius * std::cos(angle), radius * std::sin(angle));
        QPoint end = p2 - QPoint((radius + 6) * std::cos(angle), (radius + 6) * std::sin(angle));

        QPen pen(active ? highlightColor : nodeBorderColor, active ? 3 : 1.5);
        painter.setPen(pen);
        painter.drawLine(start, end);

        // Arrow head
        double arrowSize = 7;
        QPointF arrowP1 = end - QPointF(arrowSize * std::cos(angle - M_PI/6), arrowSize * std::sin(angle - M_PI/6));
        QPointF arrowP2 = end - QPointF(arrowSize * std::cos(angle + M_PI/6), arrowSize * std::sin(angle + M_PI/6));

        painter.setBrush(active ? highlightColor : nodeBorderColor);
        QPolygonF head;
        head << end << arrowP1 << arrowP2;
        painter.drawPolygon(head);
    };

    // Draw Transitions
    drawArrow(centers[ProcessState::NEW], centers[ProcessState::READY], 
              m_hasActiveTransition && m_fromState == ProcessState::NEW && m_toState == ProcessState::READY);
              
    drawArrow(centers[ProcessState::READY], centers[ProcessState::RUNNING], 
              (m_hasActiveTransition && m_fromState == ProcessState::READY && m_toState == ProcessState::RUNNING) || 
              (m_highlightState == ProcessState::RUNNING));

    // Running -> Ready (Preemption) curve or straight offset
    QPoint runOffset = centers[ProcessState::RUNNING] + QPoint(-10, -15);
    QPoint readyOffset = centers[ProcessState::READY] + QPoint(10, -15);
    drawArrow(runOffset, readyOffset, 
              m_hasActiveTransition && m_fromState == ProcessState::RUNNING && m_toState == ProcessState::READY);

    drawArrow(centers[ProcessState::RUNNING], centers[ProcessState::WAITING], 
              m_hasActiveTransition && m_fromState == ProcessState::RUNNING && m_toState == ProcessState::WAITING);

    drawArrow(centers[ProcessState::WAITING], centers[ProcessState::READY], 
              m_hasActiveTransition && m_fromState == ProcessState::WAITING && m_toState == ProcessState::READY);

    drawArrow(centers[ProcessState::RUNNING], centers[ProcessState::TERMINATED], 
              m_hasActiveTransition && m_fromState == ProcessState::RUNNING && m_toState == ProcessState::TERMINATED);

    // Draw Nodes
    for (auto const& [state, center] : centers) {
        bool isCurrent = (m_highlightState == state);
        
        QBrush brush(nodeColor);
        QPen pen(nodeBorderColor, 1.5);

        if (isCurrent) {
            if (state == ProcessState::WAITING) {
                brush = QBrush(waitingColor);
                pen = QPen(waitingColor.darker(120), 2);
            } else {
                brush = QBrush(highlightColor);
                pen = QPen(highlightColor.darker(120), 2);
            }
        }

        painter.setBrush(brush);
        painter.setPen(pen);
        painter.drawEllipse(center, radius, radius);

        // Draw Text
        painter.setPen(isCurrent ? QColor("#ffffff") : textColor);
        painter.setBrush(Qt::NoBrush);
        
        QFont font = painter.font();
        font.setBold(isCurrent);
        painter.setFont(font);
        
        QRect textRect(center.x() - radius, center.y() - 10, radius * 2, 20);
        painter.drawText(textRect, Qt::AlignCenter, names[state]);
    }
}

// 2. GanttChartVisualizer
GanttChartVisualizer::GanttChartVisualizer(QWidget* parent) : QWidget(parent) {
    setMinimumHeight(120);
}

void GanttChartVisualizer::setSegments(const std::vector<GanttSegment>& segments) {
    m_segments = segments;
    update();
}

void GanttChartVisualizer::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    bool isDark = SystemConfig::getInstance().isDarkTheme();
    QColor bgColor = isDark ? QColor("#1e293b") : QColor("#ffffff");
    QColor textColor = isDark ? QColor("#cbd5e1") : QColor("#0f172a");
    
    painter.fillRect(rect(), bgColor);
    painter.setPen(isDark ? QColor("#334155") : QColor("#cbd5e1"));
    painter.drawRect(0, 0, width() - 1, height() - 1);

    if (m_segments.empty()) {
        painter.setPen(textColor);
        painter.drawText(rect(), Qt::AlignCenter, "No Gantt data available. Run the simulation.");
        return;
    }

    int totalTime = m_segments.back().endTime;
    if (totalTime <= 0) return;

    int leftMargin = 30;
    int rightMargin = 30;
    int chartWidth = width() - leftMargin - rightMargin;
    int chartHeight = 50;
    int yPos = 30;

    // Drawing Timeline Tick Marks and Segments
    for (const auto& seg : m_segments) {
        int xStart = leftMargin + (double)seg.startTime / totalTime * chartWidth;
        int xEnd = leftMargin + (double)seg.endTime / totalTime * chartWidth;
        int blockWidth = xEnd - xStart;

        // Colors based on PID to distinguish
        int colorSeed = seg.pid * 45;
        QColor blockColor = QColor::fromHsl(colorSeed % 360, 180, isDark ? 100 : 200);

        painter.setBrush(blockColor);
        painter.setPen(QPen(isDark ? QColor("#0f172a") : QColor("#ffffff"), 1.5));
        painter.drawRect(xStart, yPos, blockWidth, chartHeight);

        // Process Name Label
        painter.setPen(QColor("#000000")); // Always black on pastel shapes
        QFont font = painter.font();
        font.setBold(true);
        painter.setFont(font);
        painter.drawText(QRect(xStart, yPos, blockWidth, chartHeight), Qt::AlignCenter, seg.name);

        // Start time ticks
        painter.setPen(textColor);
        font.setBold(false);
        painter.setFont(font);
        painter.drawText(xStart - 10, yPos + chartHeight + 16, 20, 15, Qt::AlignCenter, QString::number(seg.startTime));
    }

    // Last end time tick
    int lastX = leftMargin + chartWidth;
    painter.drawText(lastX - 10, yPos + chartHeight + 16, 20, 15, Qt::AlignCenter, QString::number(totalTime));
}

// 3. MemoryBarVisualizer
MemoryBarVisualizer::MemoryBarVisualizer(QWidget* parent) : QWidget(parent), m_totalSize(1000) {
    setMinimumHeight(100);
}

void MemoryBarVisualizer::setBlocks(const std::vector<MemoryBlock>& blocks, int totalSize) {
    m_blocks = blocks;
    m_totalSize = totalSize;
    update();
}

void MemoryBarVisualizer::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    bool isDark = SystemConfig::getInstance().isDarkTheme();
    QColor bgColor = isDark ? QColor("#1e293b") : QColor("#ffffff");
    QColor textColor = isDark ? QColor("#cbd5e1") : QColor("#0f172a");

    painter.fillRect(rect(), bgColor);
    painter.setPen(isDark ? QColor("#334155") : QColor("#cbd5e1"));
    painter.drawRect(0, 0, width() - 1, height() - 1);

    if (m_blocks.empty() || m_totalSize <= 0) {
        painter.setPen(textColor);
        painter.drawText(rect(), Qt::AlignCenter, "No memory data loaded.");
        return;
    }

    int leftMargin = 20;
    int rightMargin = 20;
    int widthDraw = width() - leftMargin - rightMargin;
    int heightDraw = 40;
    int yPos = 25;

    for (const auto& block : m_blocks) {
        double relativeStart = (double)block.startAddress / m_totalSize;
        double relativeWidth = (double)block.size / m_totalSize;

        int xStart = leftMargin + relativeStart * widthDraw;
        int xBlockWidth = relativeWidth * widthDraw;

        QColor color;
        QString text = "";
        
        if (block.isAllocated) {
            color = QColor("#2563eb"); // Occupied Blue
            text = QString("%1 (%2MB)").arg(block.allocatedPName).arg(block.size - block.internalFragmentation);
        } else {
            color = isDark ? QColor("#334155") : QColor("#e2e8f0"); // Free Slate
            text = QString("FREE (%2MB)").arg(block.size);
        }

        // Draw Block
        painter.setBrush(color);
        painter.setPen(QPen(isDark ? QColor("#0f172a") : QColor("#ffffff"), 1.5));
        painter.drawRect(xStart, yPos, xBlockWidth, heightDraw);

        // Text inside block
        painter.setPen(block.isAllocated ? QColor("#ffffff") : textColor);
        painter.drawText(QRect(xStart + 2, yPos, xBlockWidth - 4, heightDraw), Qt::AlignCenter, text);

        // Address markers below
        painter.setPen(textColor);
        painter.drawText(xStart - 15, yPos + heightDraw + 15, 30, 15, Qt::AlignCenter, QString::number(block.startAddress));

        // Draw internal fragmentation if any
        if (block.isAllocated && block.internalFragmentation > 0) {
            double fragRatio = (double)block.internalFragmentation / block.size;
            int fragWidth = fragRatio * xBlockWidth;
            int fragStart = xStart + xBlockWidth - fragWidth;

            // Red striped block for internal fragmentation
            QColor fragColor(220, 38, 38, 180);
            painter.setBrush(fragColor);
            painter.setPen(Qt::NoPen);
            painter.drawRect(fragStart, yPos, fragWidth, heightDraw);
            
            // Draw small label if big enough
            if (fragWidth > 25) {
                painter.setPen(QColor("#ffffff"));
                painter.drawText(QRect(fragStart, yPos, fragWidth, heightDraw), Qt::AlignCenter, "IF");
            }
        }
    }

    // Last Address label at the very end
    int endX = leftMargin + widthDraw;
    painter.setPen(textColor);
    painter.drawText(endX - 15, yPos + heightDraw + 15, 30, 15, Qt::AlignCenter, QString::number(m_totalSize));
}

// 4. RAGVisualizer
RAGVisualizer::RAGVisualizer(QWidget* parent) 
    : QWidget(parent), m_processCount(0), m_resourceCount(0) {
    setMinimumHeight(280);
}

void RAGVisualizer::setData(int pCount, int rCount, 
                             const std::vector<std::vector<int>>& alloc, 
                             const std::vector<std::vector<int>>& req,
                             const std::vector<std::string>& deadlocks) {
    m_processCount = pCount;
    m_resourceCount = rCount;
    m_allocMatrix = alloc;
    m_reqMatrix = req;
    m_deadlockedNodes = deadlocks;
    update();
}

void RAGVisualizer::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    bool isDark = SystemConfig::getInstance().isDarkTheme();
    QColor bgColor = isDark ? QColor("#1e293b") : QColor("#ffffff");
    QColor textColor = isDark ? QColor("#f8fafc") : QColor("#0f172a");

    painter.fillRect(rect(), bgColor);
    painter.setPen(isDark ? QColor("#334155") : QColor("#cbd5e1"));
    painter.drawRect(0, 0, width() - 1, height() - 1);

    if (m_processCount == 0 || m_resourceCount == 0) {
        painter.setPen(textColor);
        painter.drawText(rect(), Qt::AlignCenter, "Initialize Banker's to see graph.");
        return;
    }

    // Coordinate positions
    std::vector<QPoint> procPos(m_processCount);
    std::vector<QPoint> resPos(m_resourceCount);

    int canvasHeight = height();
    int canvasWidth = width();

    // Process nodes on the left, resources on the right
    int leftX = 80;
    int rightX = canvasWidth - 100;
    
    // Distribute processes
    for (int i = 0; i < m_processCount; ++i) {
        int y = 35 + i * (canvasHeight - 60) / (m_processCount > 1 ? m_processCount - 1 : 1);
        procPos[i] = QPoint(leftX, y);
    }

    // Distribute resources
    for (int j = 0; j < m_resourceCount; ++j) {
        int y = 45 + j * (canvasHeight - 80) / (m_resourceCount > 1 ? m_resourceCount - 1 : 1);
        resPos[j] = QPoint(rightX, y);
    }

    int pRadius = 20;
    int rSize = 34;

    // Helper: cycle search check
    auto isDeadlocked = [&](const std::string& name) {
        return std::find(m_deadlockedNodes.begin(), m_deadlockedNodes.end(), name) != m_deadlockedNodes.end();
    };

    // Draw Edges
    for (int i = 0; i < m_processCount; ++i) {
        for (int j = 0; j < m_resourceCount; ++j) {
            // Resource to Process (Allocation)
            if (m_allocMatrix[i][j] > 0) {
                QPoint start = resPos[j];
                QPoint end = procPos[i];
                double angle = std::atan2(end.y() - start.y(), end.x() - start.x());
                QPoint edgeStart = start + QPoint(rSize/2 * std::cos(angle), rSize/2 * std::sin(angle));
                QPoint edgeEnd = end - QPoint((pRadius + 5) * std::cos(angle), (pRadius + 5) * std::sin(angle));

                bool active = isDeadlocked("P" + std::to_string(i)) && isDeadlocked("R" + std::to_string(j));
                painter.setPen(QPen(active ? QColor("#ef4444") : QColor("#10b981"), active ? 2.5 : 1.2));
                painter.drawLine(edgeStart, edgeEnd);

                // Draw arrowhead
                double headSize = 6;
                QPointF p1 = edgeEnd - QPointF(headSize * std::cos(angle - M_PI/6), headSize * std::sin(angle - M_PI/6));
                QPointF p2 = edgeEnd - QPointF(headSize * std::cos(angle + M_PI/6), headSize * std::sin(angle + M_PI/6));
                painter.setBrush(active ? QColor("#ef4444") : QColor("#10b981"));
                QPolygonF arrow;
                arrow << edgeEnd << p1 << p2;
                painter.drawPolygon(arrow);
            }

            // Process to Resource (Request)
            if (m_reqMatrix[i][j] > 0) {
                QPoint start = procPos[i];
                QPoint end = resPos[j];
                double angle = std::atan2(end.y() - start.y(), end.x() - start.x());
                QPoint edgeStart = start + QPoint(pRadius * std::cos(angle), pRadius * std::sin(angle));
                QPoint edgeEnd = end - QPoint((rSize/2 + 5) * std::cos(angle), (rSize/2 + 5) * std::sin(angle));

                bool active = isDeadlocked("P" + std::to_string(i)) && isDeadlocked("R" + std::to_string(j));
                painter.setPen(QPen(active ? QColor("#ef4444") : QColor("#2563eb"), active ? 2.5 : 1.2));
                painter.drawLine(edgeStart, edgeEnd);

                // Draw arrowhead
                double headSize = 6;
                QPointF p1 = edgeEnd - QPointF(headSize * std::cos(angle - M_PI/6), headSize * std::sin(angle - M_PI/6));
                QPointF p2 = edgeEnd - QPointF(headSize * std::cos(angle + M_PI/6), headSize * std::sin(angle + M_PI/6));
                painter.setBrush(active ? QColor("#ef4444") : QColor("#2563eb"));
                QPolygonF arrow;
                arrow << edgeEnd << p1 << p2;
                painter.drawPolygon(arrow);
            }
        }
    }

    // Draw Processes (Circles)
    for (int i = 0; i < m_processCount; ++i) {
        bool dl = isDeadlocked("P" + std::to_string(i));
        painter.setBrush(dl ? QColor("#fecaca") : QColor("#e0f2fe"));
        painter.setPen(QPen(dl ? QColor("#ef4444") : QColor("#3b82f6"), 2));
        painter.drawEllipse(procPos[i], pRadius, pRadius);

        painter.setPen(QColor("#0f172a"));
        QFont font = painter.font();
        font.setBold(true);
        painter.setFont(font);
        painter.drawText(QRect(procPos[i].x() - pRadius, procPos[i].y() - pRadius, pRadius*2, pRadius*2), 
                         Qt::AlignCenter, QString("P%1").arg(i));
    }

    // Draw Resources (Squares)
    for (int j = 0; j < m_resourceCount; ++j) {
        bool dl = isDeadlocked("R" + std::to_string(j));
        painter.setBrush(dl ? QColor("#fecaca") : QColor("#fef3c7"));
        painter.setPen(QPen(dl ? QColor("#ef4444") : QColor("#f59e0b"), 2));
        painter.drawRect(resPos[j].x() - rSize/2, resPos[j].y() - rSize/2, rSize, rSize);

        painter.setPen(QColor("#0f172a"));
        QFont font = painter.font();
        font.setBold(true);
        painter.setFont(font);
        painter.drawText(QRect(resPos[j].x() - rSize/2, resPos[j].y() - rSize/2, rSize, rSize), 
                         Qt::AlignCenter, QString("R%1").arg(j));
    }
}

// 5. DiskSeekVisualizer
DiskSeekVisualizer::DiskSeekVisualizer(QWidget* parent) : QWidget(parent), m_totalCylinders(200) {
    setMinimumHeight(220);
}

void DiskSeekVisualizer::setSequence(const std::vector<int>& sequence, int cylinders) {
    m_sequence = sequence;
    m_totalCylinders = cylinders;
    update();
}

void DiskSeekVisualizer::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    bool isDark = SystemConfig::getInstance().isDarkTheme();
    QColor bgColor = isDark ? QColor("#1e293b") : QColor("#ffffff");
    QColor textColor = isDark ? QColor("#cbd5e1") : QColor("#0f172a");

    painter.fillRect(rect(), bgColor);
    painter.setPen(isDark ? QColor("#334155") : QColor("#cbd5e1"));
    painter.drawRect(0, 0, width() - 1, height() - 1);

    if (m_sequence.empty() || m_totalCylinders <= 0) {
        painter.setPen(textColor);
        painter.drawText(rect(), Qt::AlignCenter, "Execute disk simulation to plot path.");
        return;
    }

    int leftMargin = 30;
    int rightMargin = 30;
    int topMargin = 20;
    int bottomMargin = 20;

    int graphWidth = width() - leftMargin - rightMargin;
    int graphHeight = height() - topMargin - bottomMargin;

    // Draw coordinate cylinder grid lines (0, 50, 100, 150, 200, etc.)
    painter.setPen(QPen(isDark ? QColor("#334155") : QColor("#cbd5e1"), 1, Qt::DashLine));
    int intervals = 4;
    for (int k = 0; k <= intervals; ++k) {
        int cylVal = k * m_totalCylinders / intervals;
        int x = leftMargin + (double)cylVal / m_totalCylinders * graphWidth;
        painter.drawLine(x, topMargin, x, topMargin + graphHeight);
        
        painter.setPen(textColor);
        painter.drawText(x - 15, topMargin + graphHeight + 3, 30, 15, Qt::AlignCenter, QString::number(cylVal));
        painter.setPen(QPen(isDark ? QColor("#334155") : QColor("#cbd5e1"), 1, Qt::DashLine));
    }

    // Plot Points and Draw Lines
    int pointCount = m_sequence.size();
    std::vector<QPoint> points(pointCount);

    painter.setPen(QPen(QColor("#3b82f6"), 2));
    for (int i = 0; i < pointCount; ++i) {
        int x = leftMargin + (double)m_sequence[i] / m_totalCylinders * graphWidth;
        int y = topMargin + (double)i / (pointCount > 1 ? pointCount - 1 : 1) * graphHeight;
        points[i] = QPoint(x, y);

        if (i > 0) {
            // Check if jump in CSCAN or CLOOK (represented by dashed line)
            bool isJump = std::abs(m_sequence[i] - m_sequence[i-1]) > m_totalCylinders * 0.7;
            if (isJump) {
                painter.setPen(QPen(QColor("#a8a29e"), 1.5, Qt::DotLine));
            } else {
                painter.setPen(QPen(QColor("#3b82f6"), 2));
            }
            painter.drawLine(points[i-1], points[i]);
        }
    }

    // Draw dots & step labels
    for (int i = 0; i < pointCount; ++i) {
        painter.setBrush(i == 0 ? QColor("#eab308") : QColor("#2563eb")); // Start head is yellow
        painter.setPen(Qt::NoPen);
        painter.drawEllipse(points[i], 5, 5);

        // Step index labels on Y axis
        painter.setPen(textColor);
        painter.drawText(leftMargin - 25, points[i].y() - 6, 20, 12, Qt::AlignRight | Qt::AlignVCenter, QString::number(i));
    }
}

// ============================================================================
// SIMULATION MODULE PAGES IMPLEMENTATIONS
// ============================================================================

// ----------------------------------------------------------------------------
// MODULE 1: PROCESS MANAGEMENT VIEW
// ----------------------------------------------------------------------------
ProcessWidget::ProcessWidget(ProcessManager* model, QWidget* parent) 
    : QWidget(parent), m_model(model) {
    m_model->addObserver(this);
    setupUI();
    update();
}

ProcessWidget::~ProcessWidget() {
    m_model->removeObserver(this);
}

void ProcessWidget::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(15, 15, 15, 15);
    mainLayout->setSpacing(15);

    // Header
    QLabel* headerLabel = new QLabel("Process Management Simulator", this);
    headerLabel->setObjectName("TitleLabel");
    mainLayout->addWidget(headerLabel);

    // Dynamic split pane
    QHBoxLayout* contentLayout = new QHBoxLayout();
    contentLayout->setSpacing(15);

    // Left Panel: Operations & Inputs
    QVBoxLayout* leftPanel = new QVBoxLayout();
    leftPanel->setSpacing(10);

    QGroupBox* createGroup = new QGroupBox("Create New Process", this);
    QGridLayout* grid = new QGridLayout(createGroup);
    grid->setSpacing(8);

    grid->addWidget(new QLabel("Name:", this), 0, 0);
    m_nameInput = new QLineEdit(this);
    m_nameInput->setPlaceholderText("Auto-generated if empty");
    grid->addWidget(m_nameInput, 0, 1);

    grid->addWidget(new QLabel("Arrival Time:", this), 1, 0);
    m_arrivalInput = new QSpinBox(this);
    m_arrivalInput->setRange(0, 100);
    grid->addWidget(m_arrivalInput, 1, 1);

    grid->addWidget(new QLabel("Burst Time (CPU):", this), 2, 0);
    m_burstInput = new QSpinBox(this);
    m_burstInput->setRange(1, 100);
    m_burstInput->setValue(5);
    grid->addWidget(m_burstInput, 2, 1);

    grid->addWidget(new QLabel("Priority (Lower = Higher):", this), 3, 0);
    m_priorityInput = new QSpinBox(this);
    m_priorityInput->setRange(1, 10);
    m_priorityInput->setValue(1);
    grid->addWidget(m_priorityInput, 3, 1);

    grid->addWidget(new QLabel("Memory Required (MB):", this), 4, 0);
    m_memoryInput = new QSpinBox(this);
    m_memoryInput->setRange(8, 512);
    m_memoryInput->setValue(32);
    grid->addWidget(m_memoryInput, 4, 1);

    m_createBtn = new QPushButton("Create Process", this);
    grid->addWidget(m_createBtn, 5, 0, 1, 2);

    leftPanel->addWidget(createGroup);

    // Control Group
    QGroupBox* controlGroup = new QGroupBox("Process Actions", this);
    QVBoxLayout* ctrlLayout = new QVBoxLayout(controlGroup);
    ctrlLayout->setSpacing(8);

    m_suspendBtn = new QPushButton("Suspend Selection", this);
    m_suspendBtn->setObjectName("SecondaryButton");
    ctrlLayout->addWidget(m_suspendBtn);

    m_resumeBtn = new QPushButton("Resume Selection", this);
    m_resumeBtn->setObjectName("SecondaryButton");
    ctrlLayout->addWidget(m_resumeBtn);

    m_deleteBtn = new QPushButton("Delete Selection", this);
    m_deleteBtn->setObjectName("DestructiveButton");
    ctrlLayout->addWidget(m_deleteBtn);

    leftPanel->addWidget(controlGroup);
    leftPanel->addStretch();
    
    contentLayout->addLayout(leftPanel, 1);

    // Right Panel: Tables and State Drawings
    QVBoxLayout* rightPanel = new QVBoxLayout();
    rightPanel->setSpacing(12);

    // Table
    m_table = new QTableWidget(this);
    m_table->setColumnCount(6);
    m_table->setHorizontalHeaderLabels({"PID", "Name", "State", "Arrival", "Burst", "Priority"});
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_table->setMinimumHeight(180);
    rightPanel->addWidget(m_table);

    // Visualizer Diagram
    QLabel* diagHeader = new QLabel("Active State Transition Monitor:", this);
    diagHeader->setStyleSheet("font-weight: bold;");
    rightPanel->addWidget(diagHeader);

    m_visualizer = new ProcessStateVisualizer(this);
    rightPanel->addWidget(m_visualizer);

    contentLayout->addLayout(rightPanel, 2);
    mainLayout->addLayout(contentLayout);

    // Connections
    connect(m_createBtn, &QPushButton::clicked, this, [this]() {
        m_model->createProcess(m_nameInput->text(), m_arrivalInput->value(), 
                               m_burstInput->value(), m_priorityInput->value(), m_memoryInput->value());
        m_nameInput->clear();
    });

    connect(m_deleteBtn, &QPushButton::clicked, this, [this]() {
        int row = m_table->currentRow();
        if (row >= 0) {
            int pid = m_table->item(row, 0)->text().toInt();
            m_model->deleteProcess(pid);
        }
    });

    connect(m_suspendBtn, &QPushButton::clicked, this, [this]() {
        int row = m_table->currentRow();
        if (row >= 0) {
            int pid = m_table->item(row, 0)->text().toInt();
            m_model->suspendProcess(pid);
        }
    });

    connect(m_resumeBtn, &QPushButton::clicked, this, [this]() {
        int row = m_table->currentRow();
        if (row >= 0) {
            int pid = m_table->item(row, 0)->text().toInt();
            m_model->resumeProcess(pid);
        }
    });

    connect(m_table, &QTableWidget::itemSelectionChanged, this, [this]() {
        int row = m_table->currentRow();
        if (row >= 0) {
            QString stateStr = m_table->item(row, 2)->text();
            ProcessState state = ProcessState::NEW;
            if (stateStr == "READY") state = ProcessState::READY;
            else if (stateStr == "RUNNING") state = ProcessState::RUNNING;
            else if (stateStr == "WAITING") state = ProcessState::WAITING;
            else if (stateStr == "TERMINATED") state = ProcessState::TERMINATED;
            m_visualizer->setHighlightState(state);
        }
    });
}

void ProcessWidget::update() {
    m_table->setRowCount(0);
    const auto& procs = m_model->getProcesses();

    for (const auto& p : procs) {
        int row = m_table->rowCount();
        m_table->insertRow(row);

        m_table->setItem(row, 0, new QTableWidgetItem(QString::number(p.pid)));
        m_table->setItem(row, 1, new QTableWidgetItem(p.name));
        m_table->setItem(row, 2, new QTableWidgetItem(p.getStateString()));
        m_table->setItem(row, 3, new QTableWidgetItem(QString::number(p.arrivalTime)));
        m_table->setItem(row, 4, new QTableWidgetItem(QString::number(p.burstTime)));
        m_table->setItem(row, 5, new QTableWidgetItem(QString::number(p.priority)));
    }
}

// ----------------------------------------------------------------------------
// MODULE 2: CPU SCHEDULING VIEW
// ----------------------------------------------------------------------------
CpuSchedulerWidget::CpuSchedulerWidget(CpuScheduler* model, ProcessManager* procManager, QWidget* parent)
    : QWidget(parent), m_model(model), m_procManager(procManager) {
    m_model->addObserver(this);
    setupUI();
    update();
}

CpuSchedulerWidget::~CpuSchedulerWidget() {
    m_model->removeObserver(this);
}

void CpuSchedulerWidget::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(15, 15, 15, 15);
    mainLayout->setSpacing(12);

    QLabel* headerLabel = new QLabel("CPU Scheduling Algorithms", this);
    headerLabel->setObjectName("TitleLabel");
    mainLayout->addWidget(headerLabel);

    // Top Controls
    QHBoxLayout* topControls = new QHBoxLayout();
    topControls->addWidget(new QLabel("Algorithm:", this));
    
    m_algoSelect = new QComboBox(this);
    m_algoSelect->addItem("FCFS (First Come First Served)", (int)SchedulerFactory::AlgorithmType::FCFS);
    m_algoSelect->addItem("SJF (Non-Preemptive Shortest Job First)", (int)SchedulerFactory::AlgorithmType::SJF_NON_PREEMPTIVE);
    m_algoSelect->addItem("SRTF (Preemptive SJF)", (int)SchedulerFactory::AlgorithmType::SJF_PREEMPTIVE);
    m_algoSelect->addItem("Priority (Non-Preemptive)", (int)SchedulerFactory::AlgorithmType::PRIORITY_NON_PREEMPTIVE);
    m_algoSelect->addItem("Priority (Preemptive)", (int)SchedulerFactory::AlgorithmType::PRIORITY_PREEMPTIVE);
    m_algoSelect->addItem("Round Robin", (int)SchedulerFactory::AlgorithmType::ROUND_ROBIN);
    topControls->addWidget(m_algoSelect);

    topControls->addWidget(new QLabel("Time Quantum (RR Only):", this));
    m_quantumInput = new QSpinBox(this);
    m_quantumInput->setRange(1, 20);
    m_quantumInput->setValue(2);
    topControls->addWidget(m_quantumInput);

    m_loadProcessesBtn = new QPushButton("Load Active Processes", this);
    m_loadProcessesBtn->setObjectName("SecondaryButton");
    topControls->addWidget(m_loadProcessesBtn);

    m_runBtn = new QPushButton("Run Simulation", this);
    topControls->addWidget(m_runBtn);
    mainLayout->addLayout(topControls);

    // Split grids
    QHBoxLayout* gridSplit = new QHBoxLayout();
    
    QGroupBox* inputGroup = new QGroupBox("Input Process Queue", this);
    QVBoxLayout* inputL = new QVBoxLayout(inputGroup);
    m_inputTable = new QTableWidget(this);
    m_inputTable->setColumnCount(4);
    m_inputTable->setHorizontalHeaderLabels({"PID", "Name", "Arrival", "Burst"});
    m_inputTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    inputL->addWidget(m_inputTable);
    gridSplit->addWidget(inputGroup, 1);

    QGroupBox* resultGroup = new QGroupBox("Execution & Metrics Result Table", this);
    QVBoxLayout* resultL = new QVBoxLayout(resultGroup);
    m_resultTable = new QTableWidget(this);
    m_resultTable->setColumnCount(6);
    m_resultTable->setHorizontalHeaderLabels({"PID", "Name", "Finish", "Turnaround", "Waiting", "Response"});
    m_resultTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    resultL->addWidget(m_resultTable);
    gridSplit->addWidget(resultGroup, 2);

    mainLayout->addLayout(gridSplit, 2);

    // Gantt chart area
    QGroupBox* ganttGroup = new QGroupBox("Visual Gantt Chart Timeline", this);
    QVBoxLayout* ganttL = new QVBoxLayout(ganttGroup);
    m_ganttVisualizer = new GanttChartVisualizer(this);
    ganttL->addWidget(m_ganttVisualizer);
    mainLayout->addWidget(ganttGroup, 1);

    // Stats Bar
    QHBoxLayout* statsBar = new QHBoxLayout();
    m_avgWaitLabel = new QLabel("Avg Waiting Time: 0.00", this);
    m_avgTurnLabel = new QLabel("Avg Turnaround Time: 0.00", this);
    m_avgRespLabel = new QLabel("Avg Response Time: 0.00", this);
    m_cpuUtilLabel = new QLabel("CPU Utilization: 0.0%", this);
    m_throughputLabel = new QLabel("Throughput: 0.00 jobs/sec", this);

    statsBar->addWidget(m_avgWaitLabel);
    statsBar->addWidget(m_avgTurnLabel);
    statsBar->addWidget(m_avgRespLabel);
    statsBar->addWidget(m_cpuUtilLabel);
    statsBar->addWidget(m_throughputLabel);
    mainLayout->addLayout(statsBar);

    // Wires
    connect(m_loadProcessesBtn, &QPushButton::clicked, this, [this]() {
        // Load processes from active process manager
        const auto& activeProcs = m_procManager->getProcesses();
        m_model->setProcesses(activeProcs);
        update();
    });

    connect(m_runBtn, &QPushButton::clicked, this, [this]() {
        // Run Scheduler
        SchedulerFactory::AlgorithmType type = (SchedulerFactory::AlgorithmType)m_algoSelect->currentData().toInt();
        m_model->setAlgorithm(type);
        m_model->setTimeQuantum(m_quantumInput->value());
        m_model->runSimulation();
    });
}

void CpuSchedulerWidget::update() {
    // Fill input table from m_procManager
    m_inputTable->setRowCount(0);
    const auto& activeProcs = m_procManager->getProcesses();
    for (const auto& p : activeProcs) {
        int row = m_inputTable->rowCount();
        m_inputTable->insertRow(row);
        m_inputTable->setItem(row, 0, new QTableWidgetItem(QString::number(p.pid)));
        m_inputTable->setItem(row, 1, new QTableWidgetItem(p.name));
        m_inputTable->setItem(row, 2, new QTableWidgetItem(QString::number(p.arrivalTime)));
        m_inputTable->setItem(row, 3, new QTableWidgetItem(QString::number(p.burstTime)));
    }

    // Fill results table from CpuScheduler output
    m_resultTable->setRowCount(0);
    const auto& scheduled = m_model->getScheduledProcesses();
    for (const auto& p : scheduled) {
        int row = m_resultTable->rowCount();
        m_resultTable->insertRow(row);
        m_resultTable->setItem(row, 0, new QTableWidgetItem(QString::number(p.pid)));
        m_resultTable->setItem(row, 1, new QTableWidgetItem(p.name));
        m_resultTable->setItem(row, 2, new QTableWidgetItem(QString::number(p.finishTime)));
        m_resultTable->setItem(row, 3, new QTableWidgetItem(QString::number(p.turnaroundTime)));
        m_resultTable->setItem(row, 4, new QTableWidgetItem(QString::number(p.waitingTime)));
        m_resultTable->setItem(row, 5, new QTableWidgetItem(QString::number(p.responseTime)));
    }

    // Update Gantt Visualizer
    m_ganttVisualizer->setSegments(m_model->getGanttChart());

    // Update Stats Label
    m_avgWaitLabel->setText(QString("Avg Waiting Time: %1").arg(m_model->getAvgWaitingTime(), 0, 'f', 2));
    m_avgTurnLabel->setText(QString("Avg Turnaround Time: %1").arg(m_model->getAvgTurnaroundTime(), 0, 'f', 2));
    m_avgRespLabel->setText(QString("Avg Response Time: %1").arg(m_model->getAvgResponseTime(), 0, 'f', 2));
    m_cpuUtilLabel->setText(QString("CPU Utilization: %1%").arg(m_model->getCpuUtilization(), 0, 'f', 1));
    m_throughputLabel->setText(QString("Throughput: %1 jobs/sec").arg(m_model->getThroughput(), 0, 'f', 3));
}

// ----------------------------------------------------------------------------
// MODULE 3: MEMORY MANAGEMENT VIEW
// ----------------------------------------------------------------------------
MemoryWidget::MemoryWidget(MemoryManager* model, QWidget* parent) 
    : QWidget(parent), m_model(model) {
    m_model->addObserver(this);
    setupUI();
    update();
}

MemoryWidget::~MemoryWidget() {
    m_model->removeObserver(this);
}

void MemoryWidget::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(15, 15, 15, 15);
    mainLayout->setSpacing(12);

    QLabel* headerLabel = new QLabel("Memory Management Simulator", this);
    headerLabel->setObjectName("TitleLabel");
    mainLayout->addWidget(headerLabel);

    // Initialization Bar
    QHBoxLayout* initBar = new QHBoxLayout();
    initBar->addWidget(new QLabel("Partition Scheme:", this));
    
    m_typeSelect = new QComboBox(this);
    m_typeSelect->addItem("Fixed Partitioning", (int)MemoryManager::PartitionType::FIXED);
    m_typeSelect->addItem("Dynamic Partitioning", (int)MemoryManager::PartitionType::DYNAMIC);
    initBar->addWidget(m_typeSelect);

    initBar->addWidget(new QLabel("Sizes (Fixed Only, comma separated):", this));
    m_fixedSizesInput = new QLineEdit(this);
    m_fixedSizesInput->setText("100, 150, 200, 250, 300");
    initBar->addWidget(m_fixedSizesInput);

    m_initBtn = new QPushButton("Reinitialize Memory", this);
    m_initBtn->setObjectName("SecondaryButton");
    initBar->addWidget(m_initBtn);
    mainLayout->addLayout(initBar);

    // Operations and Visualization Area
    QHBoxLayout* bodyLayout = new QHBoxLayout();
    bodyLayout->setSpacing(15);

    // Left Actions Panel
    QVBoxLayout* actionsPanel = new QVBoxLayout();
    
    QGroupBox* allocGroup = new QGroupBox("Allocate Process", this);
    QGridLayout* allocGrid = new QGridLayout(allocGroup);
    allocGrid->setSpacing(8);

    allocGrid->addWidget(new QLabel("PID:", this), 0, 0);
    m_pidInput = new QSpinBox(this);
    m_pidInput->setRange(1, 100);
    allocGrid->addWidget(m_pidInput, 0, 1);

    allocGrid->addWidget(new QLabel("Process Name:", this), 1, 0);
    m_pnameInput = new QLineEdit(this);
    m_pnameInput->setText("P1");
    allocGrid->addWidget(m_pnameInput, 1, 1);

    allocGrid->addWidget(new QLabel("Size (MB):", this), 2, 0);
    m_sizeInput = new QSpinBox(this);
    m_sizeInput->setRange(10, 500);
    m_sizeInput->setValue(50);
    allocGrid->addWidget(m_sizeInput, 2, 1);

    allocGrid->addWidget(new QLabel("Algorithm Policy:", this), 3, 0);
    m_algoSelect = new QComboBox(this);
    m_algoSelect->addItem("First Fit", (int)MemoryManager::AllocationAlgorithm::FIRST_FIT);
    m_algoSelect->addItem("Best Fit", (int)MemoryManager::AllocationAlgorithm::BEST_FIT);
    m_algoSelect->addItem("Worst Fit", (int)MemoryManager::AllocationAlgorithm::WORST_FIT);
    m_algoSelect->addItem("Next Fit", (int)MemoryManager::AllocationAlgorithm::NEXT_FIT);
    allocGrid->addWidget(m_algoSelect, 3, 1);

    m_allocateBtn = new QPushButton("Request Allocation", this);
    allocGrid->addWidget(m_allocateBtn, 4, 0, 1, 2);

    actionsPanel->addWidget(allocGroup);

    QGroupBox* deallocGroup = new QGroupBox("Deallocate Process", this);
    QVBoxLayout* deallocLayout = new QVBoxLayout(deallocGroup);
    deallocLayout->setSpacing(8);
    m_deallocateBtn = new QPushButton("Release Chosen Partition", this);
    m_deallocateBtn->setObjectName("DestructiveButton");
    deallocLayout->addWidget(m_deallocateBtn);

    actionsPanel->addWidget(deallocGroup);
    actionsPanel->addStretch();
    bodyLayout->addLayout(actionsPanel, 1);

    // Right Partition table and layout bar
    QVBoxLayout* visualPanel = new QVBoxLayout();
    
    m_blocksTable = new QTableWidget(this);
    m_blocksTable->setColumnCount(6);
    m_blocksTable->setHorizontalHeaderLabels({"Block ID", "Start Address", "Size (MB)", "Status", "Allocated PID", "Internal Frag"});
    m_blocksTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_blocksTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    visualPanel->addWidget(m_blocksTable, 2);

    visualPanel->addWidget(new QLabel("Physical Memory Layout Map:", this));
    m_barVisualizer = new MemoryBarVisualizer(this);
    visualPanel->addWidget(m_barVisualizer, 1);

    bodyLayout->addLayout(visualPanel, 2);
    mainLayout->addLayout(bodyLayout, 2);

    // Fragmentation Metrics display
    QHBoxLayout* metricsLayout = new QHBoxLayout();
    m_freeLabel = new QLabel("Free Memory: 0MB", this);
    m_allocatedLabel = new QLabel("Allocated Memory: 0MB", this);
    m_internalFragLabel = new QLabel("Internal Fragmentation: 0MB", this);
    m_externalFragLabel = new QLabel("External Fragmentation: 0MB", this);
    
    metricsLayout->addWidget(m_freeLabel);
    metricsLayout->addWidget(m_allocatedLabel);
    metricsLayout->addWidget(m_internalFragLabel);
    metricsLayout->addWidget(m_externalFragLabel);
    mainLayout->addLayout(metricsLayout);

    // Event Slots
    connect(m_initBtn, &QPushButton::clicked, this, [this]() {
        MemoryManager::PartitionType type = (MemoryManager::PartitionType)m_typeSelect->currentData().toInt();
        std::vector<int> sizes;
        if (type == MemoryManager::PartitionType::FIXED) {
            QStringList parts = m_fixedSizesInput->text().split(",");
            for (QString s : parts) {
                bool ok;
                int val = s.trimmed().toInt(&ok);
                if (ok && val > 0) sizes.push_back(val);
            }
        }
        m_model->initializeMemory(type, sizes);
    });

    connect(m_allocateBtn, &QPushButton::clicked, this, [this]() {
        MemoryManager::AllocationAlgorithm algo = (MemoryManager::AllocationAlgorithm)m_algoSelect->currentData().toInt();
        bool ok = m_model->allocateProcess(m_pidInput->value(), m_pnameInput->text(), m_sizeInput->value(), algo);
        if (!ok) {
            int extFrag = m_model->getExternalFragmentation(m_sizeInput->value());
            if (extFrag > 0) {
                QMessageBox::warning(this, "Allocation Failure", 
                    QString("Insufficient contiguous memory block! External fragmentation detected: %1MB.\nRun compaction or free files.").arg(extFrag));
            } else {
                QMessageBox::warning(this, "Allocation Failure", "No suitable memory partition block found.");
            }
        } else {
            // increment pid
            m_pidInput->setValue(m_pidInput->value() + 1);
            m_pnameInput->setText(QString("P%1").arg(m_pidInput->value()));
        }
    });

    connect(m_deallocateBtn, &QPushButton::clicked, this, [this]() {
        int row = m_blocksTable->currentRow();
        if (row >= 0) {
            int allocPid = m_blocksTable->item(row, 4)->text().toInt();
            if (allocPid > 0) {
                m_model->deallocateProcess(allocPid);
            } else {
                QMessageBox::information(this, "Deallocate", "Selected partition block is already free.");
            }
        }
    });
}

void MemoryWidget::update() {
    m_blocksTable->setRowCount(0);
    const auto& blocks = m_model->getBlocks();

    for (const auto& b : blocks) {
        int row = m_blocksTable->rowCount();
        m_blocksTable->insertRow(row);

        m_blocksTable->setItem(row, 0, new QTableWidgetItem(QString::number(b.id)));
        m_blocksTable->setItem(row, 1, new QTableWidgetItem(QString::number(b.startAddress)));
        m_blocksTable->setItem(row, 2, new QTableWidgetItem(QString::number(b.size)));
        m_blocksTable->setItem(row, 3, new QTableWidgetItem(b.isAllocated ? "ALLOCATED" : "FREE"));
        m_blocksTable->setItem(row, 4, new QTableWidgetItem(b.isAllocated ? QString::number(b.allocatedPid) : "-"));
        m_blocksTable->setItem(row, 5, new QTableWidgetItem(QString::number(b.internalFragmentation)));
    }

    m_barVisualizer->setBlocks(blocks, m_model->getTotalMemorySize());

    m_freeLabel->setText(QString("Free Memory: %1MB").arg(m_model->getFreeMemory()));
    m_allocatedLabel->setText(QString("Allocated Memory: %1MB").arg(m_model->getAllocatedMemory()));
    m_internalFragLabel->setText(QString("Internal Fragmentation: %1MB").arg(m_model->getInternalFragmentation()));
    
    // Check external fragmentation for current request size
    int requested = m_sizeInput->value();
    m_externalFragLabel->setText(QString("External Fragmentation: %1MB").arg(m_model->getExternalFragmentation(requested)));
}

// ----------------------------------------------------------------------------
// MODULE 4: PAGE REPLACEMENT VIEW
// ----------------------------------------------------------------------------
PageWidget::PageWidget(PageReplacer* model, QWidget* parent) 
    : QWidget(parent), m_model(model) {
    m_model->addObserver(this);
    setupUI();
    update();
}

PageWidget::~PageWidget() {
    m_model->removeObserver(this);
}

void PageWidget::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(15, 15, 15, 15);
    mainLayout->setSpacing(12);

    QLabel* headerLabel = new QLabel("Page Replacement Algorithms", this);
    headerLabel->setObjectName("TitleLabel");
    mainLayout->addWidget(headerLabel);

    // Setup controls
    QHBoxLayout* controls = new QHBoxLayout();
    controls->addWidget(new QLabel("Page Reference String:", this));
    
    m_refStringInput = new QLineEdit(this);
    m_refStringInput->setText("7, 0, 1, 2, 0, 3, 0, 4, 2, 3, 0, 3, 2, 1, 2, 0, 1, 7, 0, 1");
    controls->addWidget(m_refStringInput, 3);

    controls->addWidget(new QLabel("Frame Slots:", this));
    m_framesInput = new QSpinBox(this);
    m_framesInput->setRange(1, 10);
    m_framesInput->setValue(3);
    controls->addWidget(m_framesInput, 1);

    controls->addWidget(new QLabel("Algorithm:", this));
    m_algoSelect = new QComboBox(this);
    m_algoSelect->addItem("FIFO (First In First Out)", (int)PageReplacer::Algorithm::FIFO);
    m_algoSelect->addItem("LRU (Least Recently Used)", (int)PageReplacer::Algorithm::LRU);
    m_algoSelect->addItem("Optimal Page Replacement", (int)PageReplacer::Algorithm::OPTIMAL);
    controls->addWidget(m_algoSelect, 2);

    m_runBtn = new QPushButton("Run Simulation", this);
    controls->addWidget(m_runBtn, 1);
    mainLayout->addLayout(controls);

    // Grid output
    QGroupBox* outputGroup = new QGroupBox("Step-by-Step Allocation Frame Grid", this);
    QVBoxLayout* gridL = new QVBoxLayout(outputGroup);
    m_gridTable = new QTableWidget(this);
    m_gridTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_gridTable->setSelectionMode(QAbstractItemView::NoSelection);
    gridL->addWidget(m_gridTable);
    mainLayout->addWidget(outputGroup, 2);

    // Metrics
    QHBoxLayout* stats = new QHBoxLayout();
    m_faultsLabel = new QLabel("Page Faults: 0", this);
    m_hitsLabel = new QLabel("Page Hits: 0", this);
    m_faultRateLabel = new QLabel("Page Fault Rate: 0.0%", this);
    
    stats->addWidget(m_faultsLabel);
    stats->addWidget(m_hitsLabel);
    stats->addWidget(m_faultRateLabel);
    mainLayout->addLayout(stats);

    connect(m_runBtn, &QPushButton::clicked, this, [this]() {
        QStringList parts = m_refStringInput->text().split(",");
        std::vector<int> refStr;
        for (QString s : parts) {
            bool ok;
            int page = s.trimmed().toInt(&ok);
            if (ok && page >= 0) refStr.push_back(page);
        }

        m_model->setReferenceString(refStr);
        m_model->setFrameCount(m_framesInput->value());
        m_model->setAlgorithm((PageReplacer::Algorithm)m_algoSelect->currentData().toInt());
        m_model->runSimulation();
    });
}

void PageWidget::update() {
    m_gridTable->setRowCount(0);
    m_gridTable->setColumnCount(0);

    const auto& steps = m_model->getSteps();
    if (steps.empty()) return;

    int rowsCount = m_framesInput->value() + 2; // Rows: Ref Page, Frame 1..N, Hit/Fault Marker
    m_gridTable->setRowCount(rowsCount);
    m_gridTable->setColumnCount(steps.size());

    // Row headers
    QStringList vertLabels;
    vertLabels << "Ref String";
    for (int k = 0; k < m_framesInput->value(); ++k) {
        vertLabels << QString("Frame %1").arg(k + 1);
    }
    vertLabels << "Result";
    m_gridTable->setVerticalHeaderLabels(vertLabels);

    // Fill the grid columns
    for (size_t col = 0; col < steps.size(); ++col) {
        const auto& step = steps[col];

        // Reference Page row
        QTableWidgetItem* refItem = new QTableWidgetItem(QString::number(step.page));
        refItem->setTextAlignment(Qt::AlignCenter);
        QFont f = refItem->font(); f.setBold(true); refItem->setFont(f);
        m_gridTable->setItem(0, refItem);

        // Frame rows
        for (int frameIdx = 0; frameIdx < (int)step.frames.size(); ++frameIdx) {
            int pageVal = step.frames[frameIdx];
            QTableWidgetItem* frameItem = new QTableWidgetItem(pageVal == -1 ? "-" : QString::number(pageVal));
            frameItem->setTextAlignment(Qt::AlignCenter);
            m_gridTable->setItem(frameIdx + 1, frameItem);
        }

        // Hit or Fault Marker row
        QTableWidgetItem* markerItem = new QTableWidgetItem(step.isHit ? "HIT" : "FAULT");
        markerItem->setTextAlignment(Qt::AlignCenter);
        
        if (step.isHit) {
            markerItem->setBackground(QBrush(QColor(16, 185, 129, 180))); // Transparent Green
            markerItem->setForeground(QBrush(QColor("#ffffff")));
        } else {
            markerItem->setBackground(QBrush(QColor(239, 68, 68, 180))); // Transparent Red
            markerItem->setForeground(QBrush(QColor("#ffffff")));
        }
        
        m_gridTable->setItem(rowsCount - 1, markerItem);
    }

    m_gridTable->resizeColumnsToContents();

    m_faultsLabel->setText(QString("Total Page Faults: %1").arg(m_model->getPageFaults()));
    m_hitsLabel->setText(QString("Total Page Hits: %1").arg(m_model->getPageHits()));
    m_faultRateLabel->setText(QString("Page Fault Rate: %1%").arg(m_model->getFaultRate() * 100, 0, 'f', 1));
}

// ----------------------------------------------------------------------------
// MODULE 5: DEADLOCK HANDLING VIEW
// ----------------------------------------------------------------------------
DeadlockWidget::DeadlockWidget(DeadlockDetector* model, QWidget* parent)
    : QWidget(parent), m_model(model) {
    m_model->addObserver(this);
    setupUI();
    update();
}

DeadlockWidget::~DeadlockWidget() {
    m_model->removeObserver(this);
}

void DeadlockWidget::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(15, 15, 15, 15);
    mainLayout->setSpacing(12);

    QLabel* headerLabel = new QLabel("Deadlock Handling (Banker's & Graph Detection)", this);
    headerLabel->setObjectName("TitleLabel");
    mainLayout->addWidget(headerLabel);

    // Resource Initializer
    QHBoxLayout* topRow = new QHBoxLayout();
    topRow->addWidget(new QLabel("Processes Count:", this));
    m_processCountInput = new QSpinBox(this);
    m_processCountInput->setRange(1, 10);
    m_processCountInput->setValue(5);
    topRow->addWidget(m_processCountInput);

    topRow->addWidget(new QLabel("Resources Count:", this));
    m_resourceCountInput = new QSpinBox(this);
    m_resourceCountInput->setRange(1, 5);
    m_resourceCountInput->setValue(3);
    topRow->addWidget(m_resourceCountInput);

    topRow->addWidget(new QLabel("Resource Vector (A,B,C...):", this));
    m_totalsInput = new QLineEdit(this);
    m_totalsInput->setText("10, 5, 7");
    topRow->addWidget(m_totalsInput);

    m_initBtn = new QPushButton("Initialize", this);
    m_initBtn->setObjectName("SecondaryButton");
    topRow->addWidget(m_initBtn);
    mainLayout->addLayout(topRow);

    // Body
    QHBoxLayout* splitGrid = new QHBoxLayout();

    // Left Matrix Editors
    QVBoxLayout* matricesLayout = new QVBoxLayout();
    
    QGroupBox* allocG = new QGroupBox("Allocation Matrix (Current Holdings)", this);
    QVBoxLayout* allocL = new QVBoxLayout(allocG);
    m_allocTable = new QTableWidget(this);
    allocL->addWidget(m_allocTable);
    matricesLayout->addWidget(allocG);

    QGroupBox* maxG = new QGroupBox("Maximum Claim Matrix (Max Request)", this);
    QVBoxLayout* maxL = new QVBoxLayout(maxG);
    m_maxTable = new QTableWidget(this);
    maxL->addWidget(m_maxTable);
    matricesLayout->addWidget(maxG);

    QGroupBox* reqG = new QGroupBox("Request Matrix (Active Deadlock Detection Requests)", this);
    QVBoxLayout* reqL = new QVBoxLayout(reqG);
    m_requestTable = new QTableWidget(this);
    reqL->addWidget(m_requestTable);
    matricesLayout->addWidget(reqG);

    splitGrid->addLayout(matricesLayout, 3);

    // Right Graphics & Results
    QVBoxLayout* graphsLayout = new QVBoxLayout();
    
    QGroupBox* configActionG = new QGroupBox("Deadlock Operations", this);
    QVBoxLayout* actionL = new QVBoxLayout(configActionG);
    actionL->setSpacing(8);

    m_runBankerBtn = new QPushButton("Calculate Banker's Safety Sequence", this);
    actionL->addWidget(m_runBankerBtn);

    m_bankersResultLabel = new QLabel("Safety State: Run analysis...", this);
    m_bankersResultLabel->setStyleSheet("font-weight: bold; padding: 4px;");
    actionL->addWidget(m_bankersResultLabel);

    m_detectDeadlockBtn = new QPushButton("Detect Graph Deadlock Cycle", this);
    m_detectDeadlockBtn->setObjectName("SecondaryButton");
    actionL->addWidget(m_detectDeadlockBtn);

    m_deadlockStatusLabel = new QLabel("Cycle Status: Clear", this);
    m_deadlockStatusLabel->setStyleSheet("font-weight: bold; padding: 4px;");
    actionL->addWidget(m_deadlockStatusLabel);

    QHBoxLayout* termLayout = new QHBoxLayout();
    termLayout->addWidget(new QLabel("Term ID to recover:", this));
    m_terminatePidInput = new QSpinBox(this);
    m_terminatePidInput->setRange(0, 9);
    termLayout->addWidget(m_terminatePidInput);
    m_terminateBtn = new QPushButton("Kill Process", this);
    m_terminateBtn->setObjectName("DestructiveButton");
    termLayout->addWidget(m_terminateBtn);
    actionL->addLayout(termLayout);

    graphsLayout->addWidget(configActionG, 1);

    graphsLayout->addWidget(new QLabel("Resource Allocation Graph Viewer:", this));
    m_ragVisualizer = new RAGVisualizer(this);
    graphsLayout->addWidget(m_ragVisualizer, 2);

    splitGrid->addLayout(graphsLayout, 2);
    mainLayout->addLayout(splitGrid);

    // Slots
    connect(m_initBtn, &QPushButton::clicked, this, [this]() {
        QStringList parts = m_totalsInput->text().split(",");
        std::vector<int> totals;
        for (QString s : parts) {
            bool ok;
            int t = s.trimmed().toInt(&ok);
            if (ok && t >= 0) totals.push_back(t);
        }
        m_model->initialize(m_processCountInput->value(), m_resourceCountInput->value(), totals);
    });

    connect(m_runBankerBtn, &QPushButton::clicked, this, [this]() {
        readTablesToMatrices();
        std::vector<int> safeSeq;
        QString message;
        bool safe = m_model->runBankers(safeSeq, message);
        
        m_bankersResultLabel->setText(message);
        if (safe) {
            m_bankersResultLabel->setStyleSheet("color: #10b981; font-weight: bold;");
        } else {
            m_bankersResultLabel->setStyleSheet("color: #ef4444; font-weight: bold;");
        }
    });

    connect(m_detectDeadlockBtn, &QPushButton::clicked, this, [this]() {
        readTablesToMatrices();
        auto cycle = m_model->detectDeadlockCycle();
        if (!cycle.empty()) {
            QString cycleStr = "";
            for (size_t k = 0; k < cycle.size(); ++k) {
                cycleStr += QString::fromStdString(cycle[k]);
                if (k < cycle.size() - 1) cycleStr += " -> ";
            }
            m_deadlockStatusLabel->setText("DEADLOCK DETECTED! Cycle: " + cycleStr);
            m_deadlockStatusLabel->setStyleSheet("color: #ef4444; font-weight: bold;");
            
            // Sync highlight to graph
            m_ragVisualizer->setData(m_model->getProcessCount(), m_model->getResourceCount(), 
                                     m_model->getAllocation(), m_model->getRequest(), cycle);
        } else {
            m_deadlockStatusLabel->setText("No deadlock cycle detected. Graph is clean.");
            m_deadlockStatusLabel->setStyleSheet("color: #10b981; font-weight: bold;");
            m_ragVisualizer->setData(m_model->getProcessCount(), m_model->getResourceCount(), 
                                     m_model->getAllocation(), m_model->getRequest(), {});
        }
    });

    connect(m_terminateBtn, &QPushButton::clicked, this, [this]() {
        bool ok = m_model->recoverDeadlock(m_terminatePidInput->value());
        if (ok) {
            QMessageBox::information(this, "Deadlock Recovered", 
                QString("Process P%1 has been terminated. Allocated resources released back to system.").arg(m_terminatePidInput->value()));
        }
    });
}

void DeadlockWidget::syncMatricesToTables() {
    int p = m_model->getProcessCount();
    int r = m_model->getResourceCount();

    m_allocTable->setRowCount(p);
    m_allocTable->setColumnCount(r);
    m_maxTable->setRowCount(p);
    m_maxTable->setColumnCount(r);
    m_requestTable->setRowCount(p);
    m_requestTable->setColumnCount(r);

    QStringList headers;
    for (int j = 0; j < r; ++j) {
        headers << QString("Res %1").arg((char)('A' + j));
    }
    
    m_allocTable->setHorizontalHeaderLabels(headers);
    m_maxTable->setHorizontalHeaderLabels(headers);
    m_requestTable->setHorizontalHeaderLabels(headers);

    QStringList vertHeaders;
    for (int i = 0; i < p; ++i) vertHeaders << QString("P%1").arg(i);
    
    m_allocTable->setVerticalHeaderLabels(vertHeaders);
    m_maxTable->setVerticalHeaderLabels(vertHeaders);
    m_requestTable->setVerticalHeaderLabels(vertHeaders);

    const auto& alloc = m_model->getAllocation();
    const auto& max = m_model->getMax();
    const auto& req = m_model->getRequest();

    for (int i = 0; i < p; ++i) {
        for (int j = 0; j < r; ++j) {
            m_allocTable->setItem(i, j, new QTableWidgetItem(QString::number(alloc[i][j])));
            m_maxTable->setItem(i, j, new QTableWidgetItem(QString::number(max[i][j])));
            m_requestTable->setItem(i, j, new QTableWidgetItem(QString::number(req[i][j])));
        }
    }
}

void DeadlockWidget::readTablesToMatrices() {
    int p = m_model->getProcessCount();
    int r = m_model->getResourceCount();

    for (int i = 0; i < p; ++i) {
        std::vector<int> allocRow(r);
        std::vector<int> maxRow(r);
        std::vector<int> reqRow(r);

        for (int j = 0; j < r; ++j) {
            allocRow[j] = m_allocTable->item(i, j)->text().toInt();
            maxRow[j] = m_maxTable->item(i, j)->text().toInt();
            reqRow[j] = m_requestTable->item(i, j)->text().toInt();
        }
        m_model->setAllocationRow(i, allocRow);
        m_model->setMaxRow(i, maxRow);
        m_model->setRequestRow(i, reqRow);
    }
}

void DeadlockWidget::update() {
    syncMatricesToTables();
    m_ragVisualizer->setData(m_model->getProcessCount(), m_model->getResourceCount(), 
                             m_model->getAllocation(), m_model->getRequest(), {});
}

// ----------------------------------------------------------------------------
// MODULE 6: DISK SCHEDULING VIEW
// ----------------------------------------------------------------------------
DiskWidget::DiskWidget(DiskScheduler* model, QWidget* parent) 
    : QWidget(parent), m_model(model) {
    m_model->addObserver(this);
    setupUI();
    update();
}

DiskWidget::~DiskWidget() {
    m_model->removeObserver(this);
}

void DiskWidget::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(15, 15, 15, 15);
    mainLayout->setSpacing(12);

    QLabel* headerLabel = new QLabel("Disk Head Scheduling Simulator", this);
    headerLabel->setObjectName("TitleLabel");
    mainLayout->addWidget(headerLabel);

    // Inputs Bar
    QHBoxLayout* controls = new QHBoxLayout();
    controls->addWidget(new QLabel("Head Start:", this));
    m_startHeadInput = new QSpinBox(this);
    m_startHeadInput->setRange(0, 1000);
    m_startHeadInput->setValue(53);
    controls->addWidget(m_startHeadInput);

    controls->addWidget(new QLabel("Requests Cylinder List:", this));
    m_requestsInput = new QLineEdit(this);
    m_requestsInput->setText("98, 183, 37, 122, 14, 124, 65, 67");
    controls->addWidget(m_requestsInput, 2);

    controls->addWidget(new QLabel("Cylinders Count:", this));
    m_cylindersInput = new QSpinBox(this);
    m_cylindersInput->setRange(50, 10000);
    m_cylindersInput->setValue(200);
    controls->addWidget(m_cylindersInput);

    controls->addWidget(new QLabel("Initial Dir:", this));
    m_directionSelect = new QComboBox(this);
    m_directionSelect->addItem("Right (Ascending)", true);
    m_directionSelect->addItem("Left (Descending)", false);
    controls->addWidget(m_directionSelect);

    mainLayout->addLayout(controls);

    // Algorithm & Run row
    QHBoxLayout* runRow = new QHBoxLayout();
    runRow->addWidget(new QLabel("Scheduling Policy:", this));
    m_algoSelect = new QComboBox(this);
    m_algoSelect->addItem("FCFS (First Come First Served)", (int)DiskSchedulerFactory::AlgorithmType::FCFS);
    m_algoSelect->addItem("SSTF (Shortest Seek Time First)", (int)DiskSchedulerFactory::AlgorithmType::SSTF);
    m_algoSelect->addItem("SCAN (Elevator)", (int)DiskSchedulerFactory::AlgorithmType::SCAN);
    m_algoSelect->addItem("C-SCAN (Circular SCAN)", (int)DiskSchedulerFactory::AlgorithmType::CSCAN);
    m_algoSelect->addItem("LOOK (Shortcut Elevator)", (int)DiskSchedulerFactory::AlgorithmType::LOOK);
    m_algoSelect->addItem("C-LOOK (Circular LOOK)", (int)DiskSchedulerFactory::AlgorithmType::CLOOK);
    runRow->addWidget(m_algoSelect, 2);

    m_runBtn = new QPushButton("Run Simulation", this);
    runRow->addWidget(m_runBtn, 1);
    mainLayout->addLayout(runRow);

    // Visualization area
    QGroupBox* visGroup = new QGroupBox("Disk Head Movement Seek Trajectory Graph", this);
    QVBoxLayout* visL = new QVBoxLayout(visGroup);
    m_seekVisualizer = new DiskSeekVisualizer(this);
    visL->addWidget(m_seekVisualizer);
    mainLayout->addWidget(visGroup, 2);

    // Metrics Displays
    QHBoxLayout* statsRow = new QHBoxLayout();
    m_headMovementLabel = new QLabel("Total Head Seek Movement: 0 cylinders", this);
    m_avgSeekLabel = new QLabel("Average Seek Time: 0.00 ms", this);
    
    statsRow->addWidget(m_headMovementLabel);
    statsRow->addWidget(m_avgSeekLabel);
    mainLayout->addLayout(statsRow);

    // Wires
    connect(m_runBtn, &QPushButton::clicked, this, [this]() {
        QStringList parts = m_requestsInput->text().split(",");
        std::vector<int> reqs;
        for (QString s : parts) {
            bool ok;
            int r = s.trimmed().toInt(&ok);
            if (ok && r >= 0) reqs.push_back(r);
        }

        m_model->setStartHead(m_startHeadInput->value());
        m_model->setRequests(reqs);
        m_model->setTotalCylinders(m_cylindersInput->value());
        m_model->setInitialDirection(m_directionSelect->currentData().toBool());
        m_model->setAlgorithm((DiskSchedulerFactory::AlgorithmType)m_algoSelect->currentData().toInt());
        m_model->runSimulation();
    });
}

void DiskWidget::update() {
    const auto& seq = m_model->getSeekSequence();
    m_seekVisualizer->setSequence(seq, m_model->getTotalHeadMovement() == 0 ? m_cylindersInput->value() : m_cylindersInput->value());
    
    m_headMovementLabel->setText(QString("Total Head Seek Movement: %1 cylinders").arg(m_model->getTotalHeadMovement()));
    m_avgSeekLabel->setText(QString("Average Seek Time: %1 ms").arg(m_model->getAvgSeekTime(), 0, 'f', 2));
}

// ----------------------------------------------------------------------------
// MODULE 7: FILE SYSTEM VIEW
// ----------------------------------------------------------------------------
FileSystemWidget::FileSystemWidget(VirtualFileSystem* model, QWidget* parent) 
    : QWidget(parent), m_model(model) {
    m_model->addObserver(this);
    setupUI();
    update();
}

FileSystemWidget::~FileSystemWidget() {
    m_model->removeObserver(this);
}

void FileSystemWidget::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(15, 15, 15, 15);
    mainLayout->setSpacing(12);

    QLabel* headerLabel = new QLabel("Virtual Disk File Allocation Simulator", this);
    headerLabel->setObjectName("TitleLabel");
    mainLayout->addWidget(headerLabel);

    // Path indicator
    m_pathLabel = new QLabel("Current Directory Path: /", this);
    m_pathLabel->setStyleSheet("font-weight: bold; font-size: 14px; color: #2563eb;");
    mainLayout->addWidget(m_pathLabel);

    QHBoxLayout* bodySplit = new QHBoxLayout();
    
    // Left: Tree List navigation and operations
    QVBoxLayout* leftPanel = new QVBoxLayout();
    leftPanel->setSpacing(8);

    m_treeView = new QTreeView(this);
    m_treeModel = new QStandardItemModel(this);
    m_treeView->setModel(m_treeModel);
    m_treeView->setHeaderHidden(true);
    m_treeView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    leftPanel->addWidget(m_treeView, 2);

    // Directory Controls
    QHBoxLayout* dirCtrl = new QHBoxLayout();
    QPushButton* parentBtn = new QPushButton("Go to Parent (..)", this);
    parentBtn->setObjectName("SecondaryButton");
    dirCtrl->addWidget(parentBtn);
    leftPanel->addLayout(dirCtrl);

    // Inputs Panel
    QGroupBox* inputsG = new QGroupBox("Create File/Folder", this);
    QGridLayout* inputGrid = new QGridLayout(inputsG);
    inputGrid->setSpacing(6);

    inputGrid->addWidget(new QLabel("Name:", this), 0, 0);
    m_itemNameInput = new QLineEdit(this);
    inputGrid->addWidget(m_itemNameInput, 0, 1);

    inputGrid->addWidget(new QLabel("Size (blocks):", this), 1, 0);
    m_fileSizeInput = new QSpinBox(this);
    m_fileSizeInput->setRange(1, 32);
    m_fileSizeInput->setValue(3);
    inputGrid->addWidget(m_fileSizeInput, 1, 1);

    inputGrid->addWidget(new QLabel("Allocation Method:", this), 2, 0);
    m_allocMethodSelect = new QComboBox(this);
    m_allocMethodSelect->addItem("Sequential Allocation");
    m_allocMethodSelect->addItem("Linked Allocation");
    m_allocMethodSelect->addItem("Indexed Allocation");
    inputGrid->addWidget(m_allocMethodSelect, 2, 1);

    m_createFolderBtn = new QPushButton("Make Folder", this);
    m_createFolderBtn->setObjectName("SecondaryButton");
    inputGrid->addWidget(m_createFolderBtn, 3, 0);

    m_createFileBtn = new QPushButton("Create File", this);
    inputGrid->addWidget(m_createFileBtn, 3, 1);

    leftPanel->addWidget(inputsG);

    // Edit Controls
    QGroupBox* editG = new QGroupBox("Modify / Delete selection", this);
    QVBoxLayout* editLayout = new QVBoxLayout(editG);
    editLayout->setSpacing(6);

    QHBoxLayout* renameRow = new QHBoxLayout();
    m_renameInput = new QLineEdit(this);
    m_renameInput->setPlaceholderText("New Name");
    renameRow->addWidget(m_renameInput);
    m_renameBtn = new QPushButton("Rename", this);
    m_renameBtn->setObjectName("SecondaryButton");
    renameRow->addWidget(m_renameBtn);
    editLayout->addLayout(renameRow);

    m_deleteBtn = new QPushButton("Delete Selected Item", this);
    m_deleteBtn->setObjectName("DestructiveButton");
    editLayout->addWidget(m_deleteBtn);

    leftPanel->addWidget(editG);

    // Search bar
    QHBoxLayout* searchRow = new QHBoxLayout();
    m_searchInput = new QLineEdit(this);
    m_searchInput->setPlaceholderText("Search item name...");
    searchRow->addWidget(m_searchInput);
    m_searchBtn = new QPushButton("Search", this);
    m_searchBtn->setObjectName("SecondaryButton");
    searchRow->addWidget(m_searchBtn);
    leftPanel->addLayout(searchRow);

    bodySplit->addLayout(leftPanel, 1);

    // Right: Disk block visual grid map
    QVBoxLayout* rightPanel = new QVBoxLayout();
    rightPanel->addWidget(new QLabel("Physical Storage Allocation Blocks (Total 64 Blocks):", this));

    m_blocksContainer = new QWidget(this);
    QGridLayout* blockGrid = new QGridLayout(m_blocksContainer);
    blockGrid->setSpacing(4);

    // Draw 8x8 block grid
    m_blockLabels.resize(64);
    for (int i = 0; i < 64; ++i) {
        QLabel* label = new QLabel(QString::number(i), m_blocksContainer);
        label->setFixedSize(56, 32);
        label->setAlignment(Qt::AlignCenter);
        label->setStyleSheet("background-color: #334155; border: 1px solid #475569; border-radius: 4px; font-weight: bold; color: #cbd5e1;");
        blockGrid->addWidget(label, i / 8, i % 8);
        m_blockLabels[i] = label;
    }
    
    rightPanel->addWidget(m_blocksContainer);
    rightPanel->addStretch();

    QHBoxLayout* blockStats = new QHBoxLayout();
    m_freeBlocksLabel = new QLabel("Free Blocks: 64", this);
    m_usedBlocksLabel = new QLabel("Used Blocks: 0", this);
    blockStats->addWidget(m_freeBlocksLabel);
    blockStats->addWidget(m_usedBlocksLabel);
    rightPanel->addLayout(blockStats);

    bodySplit->addLayout(rightPanel, 1);
    mainLayout->addLayout(bodySplit);

    // Slots
    connect(parentBtn, &QPushButton::clicked, this, [this]() {
        m_model->navigateToParent();
    });

    connect(m_treeView, &QTreeView::doubleClicked, this, [this](const QModelIndex& index) {
        QString name = index.data().toString();
        // If folders, navigate inside on double click
        m_model->changeDirectory(name);
    });

    connect(m_createFolderBtn, &QPushButton::clicked, this, [this]() {
        bool ok = m_model->createDirectory(m_itemNameInput->text());
        if (ok) m_itemNameInput->clear();
        else QMessageBox::warning(this, "FS", "Failed to create directory. Ensure unique name.");
    });

    connect(m_createFileBtn, &QPushButton::clicked, this, [this]() {
        QString method = m_allocMethodSelect->currentText().split(" ")[0]; // Get "Sequential", "Linked", "Indexed"
        bool ok = m_model->createFile(m_itemNameInput->text(), m_fileSizeInput->value(), method);
        if (ok) m_itemNameInput->clear();
        else QMessageBox::warning(this, "FS", "Failed to create file. Ensure enough contiguous blocks for Sequential, or index block for Indexed.");
    });

    connect(m_deleteBtn, &QPushButton::clicked, this, [this]() {
        QModelIndex idx = m_treeView->currentIndex();
        if (idx.isValid()) {
            QString name = idx.data().toString();
            m_model->deleteItem(name);
        }
    });

    connect(m_renameBtn, &QPushButton::clicked, this, [this]() {
        QModelIndex idx = m_treeView->currentIndex();
        if (idx.isValid()) {
            QString name = idx.data().toString();
            m_model->renameItem(name, m_renameInput->text());
            m_renameInput->clear();
        }
    });

    connect(m_searchBtn, &QPushButton::clicked, this, [this]() {
        QString query = m_searchInput->text();
        if (query.isEmpty()) return;
        auto results = m_model->searchFiles(query);
        QString msg = results.empty() ? "No files found." : QString("%1 matches found:\n").arg(results.size());
        for (const auto& r : results) {
            msg += r + "\n";
        }
        QMessageBox::information(this, "File Search Result", msg);
        m_searchInput->clear();
    });
}

void FileSystemWidget::rebuildTree(QStandardItem* parentItem, CustomDS::TreeNode<FileEntry>* modelNode) {
    if (!modelNode) return;
    for (auto* child : modelNode->children) {
        QStandardItem* item = new QStandardItem(child->data.name);
        if (child->data.isDirectory) {
            item->setIcon(style()->standardIcon(QStyle::SP_DirIcon));
            rebuildTree(item, child);
        } else {
            item->setIcon(style()->standardIcon(QStyle::SP_FileIcon));
        }
        parentItem->appendRow(item);
    }
}

void FileSystemWidget::update() {
    m_pathLabel->setText(QString("Current Directory Path: %1").arg(m_model->getCurrentPath()));

    // Rebuild tree display model
    m_treeModel->clear();
    QStandardItem* rootItem = new QStandardItem(m_model->getCurrentNode()->data.name);
    rootItem->setIcon(style()->standardIcon(QStyle::SP_DirIcon));
    m_treeModel->appendRow(rootItem);
    m_treeView->expandAll();
    
    // Read recursively
    rebuildTree(rootItem, m_model->getCurrentNode());

    // Update physical blocks representation grid
    const auto& blocks = m_model->getDiskBlocks();
    bool isDark = SystemConfig::getInstance().isDarkTheme();

    for (int i = 0; i < 64; ++i) {
        if (blocks[i].isEmpty()) {
            m_blockLabels[i]->setText(QString::number(i));
            m_blockLabels[i]->setStyleSheet(isDark 
                ? "background-color: #334155; border: 1px solid #475569; border-radius: 4px; font-weight: bold; color: #64748b;"
                : "background-color: #f1f5f9; border: 1px solid #cbd5e1; border-radius: 4px; font-weight: bold; color: #94a3b8;");
        } else {
            m_blockLabels[i]->setText(blocks[i]);
            
            // Assign color based on the first character of the file label
            int charSeed = blocks[i].isEmpty() ? 0 : blocks[i][0].unicode() * 20;
            QColor blockColor = QColor::fromHsl(charSeed % 360, 200, isDark ? 60 : 160);
            
            m_blockLabels[i]->setStyleSheet(
                QString("background-color: %1; border: 1px solid %2; border-radius: 4px; font-weight: bold; color: #ffffff; font-size: 10px;")
                .arg(blockColor.name())
                .arg(blockColor.darker(130).name())
            );
        }
    }

    m_freeBlocksLabel->setText(QString("Free Blocks: %1").arg(m_model->getFreeBlockCount()));
    m_usedBlocksLabel->setText(QString("Used Blocks: %1").arg(m_model->getUsedBlockCount()));
}

// ----------------------------------------------------------------------------
// MODULE 8: PERFORMANCE ANALYTICS VIEW
// ----------------------------------------------------------------------------
AnalyticsWidget::AnalyticsWidget(CpuScheduler* cpu, PageReplacer* page, DiskScheduler* disk, QWidget* parent)
    : QWidget(parent), m_cpuModel(cpu), m_pageModel(page), m_diskModel(disk) {
    setupUI();
}

void AnalyticsWidget::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(15, 15, 15, 15);
    mainLayout->setSpacing(12);

    QLabel* headerLabel = new QLabel("System Performance Comparative Analytics", this);
    headerLabel->setObjectName("TitleLabel");
    mainLayout->addWidget(headerLabel);

    QHBoxLayout* refreshRow = new QHBoxLayout();
    m_refreshBtn = new QPushButton("Refresh Comparison Metrics", this);
    refreshRow->addWidget(m_refreshBtn);
    refreshRow->addStretch();
    mainLayout->addLayout(refreshRow);

    QScrollArea* scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    
    QWidget* content = new QWidget(scroll);
    QVBoxLayout* contentL = new QVBoxLayout(content);
    contentL->setSpacing(15);

    // 1. CPU Comparison Card
    QGroupBox* cpuCompareG = new QGroupBox("CPU Scheduling Algorithm Comparisons", content);
    QVBoxLayout* cpuL = new QVBoxLayout(cpuCompareG);
    m_cpuCompareTable = new QTableWidget(content);
    m_cpuCompareTable->setColumnCount(6);
    m_cpuCompareTable->setHorizontalHeaderLabels({"Algorithm", "Avg Wait Time", "Avg Turnaround", "Avg Response", "CPU Util %", "Throughput"});
    m_cpuCompareTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_cpuCompareTable->setMinimumHeight(150);
    cpuL->addWidget(m_cpuCompareTable);
    contentL->addWidget(cpuCompareG);

    // 2. Page Replacement Card
    QGroupBox* pageCompareG = new QGroupBox("Page Replacement Page Fault Comparisons", content);
    QVBoxLayout* pageL = new QVBoxLayout(pageCompareG);
    m_pageCompareTable = new QTableWidget(content);
    m_pageCompareTable->setColumnCount(4);
    m_pageCompareTable->setHorizontalHeaderLabels({"Algorithm", "Total Page Faults", "Total Page Hits", "Fault Rate"});
    m_pageCompareTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_pageCompareTable->setMinimumHeight(120);
    pageL->addWidget(m_pageCompareTable);
    contentL->addWidget(pageCompareG);

    // 3. Disk Seek Card
    QGroupBox* diskCompareG = new QGroupBox("Disk Head Seek Movement Comparisons", content);
    QVBoxLayout* diskL = new QVBoxLayout(diskCompareG);
    m_diskCompareTable = new QTableWidget(content);
    m_diskCompareTable->setColumnCount(3);
    m_diskCompareTable->setHorizontalHeaderLabels({"Algorithm", "Total Cylinder Seek distance", "Average Seek distance"});
    m_diskCompareTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_diskCompareTable->setMinimumHeight(150);
    diskL->addWidget(m_diskCompareTable);
    contentL->addWidget(diskCompareG);

    content->setLayout(contentL);
    scroll->setWidget(content);
    mainLayout->addWidget(scroll);

    connect(m_refreshBtn, &QPushButton::clicked, this, &AnalyticsWidget::runComparison);
}

void AnalyticsWidget::runComparison() {
    // 1. CPU Scheduling Comparison Run
    m_cpuCompareTable->setRowCount(0);
    // Gather all alg values
    std::vector<SchedulerFactory::AlgorithmType> algs = {
        SchedulerFactory::AlgorithmType::FCFS,
        SchedulerFactory::AlgorithmType::SJF_NON_PREEMPTIVE,
        SchedulerFactory::AlgorithmType::SJF_PREEMPTIVE,
        SchedulerFactory::AlgorithmType::PRIORITY_NON_PREEMPTIVE,
        SchedulerFactory::AlgorithmType::PRIORITY_PREEMPTIVE,
        SchedulerFactory::AlgorithmType::ROUND_ROBIN
    };

    for (auto alg : algs) {
        // Run simulation temporarily for this algorithm using cached processes
        m_cpuModel->setAlgorithm(alg);
        m_cpuModel->runSimulation();

        int row = m_cpuCompareTable->rowCount();
        m_cpuCompareTable->insertRow(row);
        m_cpuCompareTable->setItem(row, 0, new QTableWidgetItem(m_cpuModel->getAlgorithmName()));
        m_cpuCompareTable->setItem(row, 1, new QTableWidgetItem(QString::number(m_cpuModel->getAvgWaitingTime(), 'f', 2)));
        m_cpuCompareTable->setItem(row, 2, new QTableWidgetItem(QString::number(m_cpuModel->getAvgTurnaroundTime(), 'f', 2)));
        m_cpuCompareTable->setItem(row, 3, new QTableWidgetItem(QString::number(m_cpuModel->getAvgResponseTime(), 'f', 2)));
        m_cpuCompareTable->setItem(row, 4, new QTableWidgetItem(QString("%1%").arg(m_cpuModel->getCpuUtilization(), 0, 'f', 1)));
        m_cpuCompareTable->setItem(row, 5, new QTableWidgetItem(QString::number(m_cpuModel->getThroughput(), 'f', 3)));
    }

    // 2. Page Replacement Comparison Run
    m_pageCompareTable->setRowCount(0);
    std::vector<PageReplacer::Algorithm> pageAlgs = {
        PageReplacer::Algorithm::FIFO,
        PageReplacer::Algorithm::LRU,
        PageReplacer::Algorithm::OPTIMAL
    };

    for (auto alg : pageAlgs) {
        m_pageModel->setAlgorithm(alg);
        m_pageModel->runSimulation();

        int row = m_pageCompareTable->rowCount();
        m_pageCompareTable->insertRow(row);
        m_pageCompareTable->setItem(row, 0, new QTableWidgetItem(m_pageModel->getAlgorithmName()));
        m_pageCompareTable->setItem(row, 1, new QTableWidgetItem(QString::number(m_pageModel->getPageFaults())));
        m_pageCompareTable->setItem(row, 2, new QTableWidgetItem(QString::number(m_pageModel->getPageHits())));
        m_pageCompareTable->setItem(row, 3, new QTableWidgetItem(QString("%1%").arg(m_pageModel->getFaultRate() * 100, 0, 'f', 1)));
    }

    // 3. Disk Seek Comparison Run
    m_diskCompareTable->setRowCount(0);
    std::vector<DiskSchedulerFactory::AlgorithmType> diskAlgs = {
        DiskSchedulerFactory::AlgorithmType::FCFS,
        DiskSchedulerFactory::AlgorithmType::SSTF,
        DiskSchedulerFactory::AlgorithmType::SCAN,
        DiskSchedulerFactory::AlgorithmType::CSCAN,
        DiskSchedulerFactory::AlgorithmType::LOOK,
        DiskSchedulerFactory::AlgorithmType::CLOOK
    };

    for (auto alg : diskAlgs) {
        m_diskModel->setAlgorithm(alg);
        m_diskModel->runSimulation();

        int row = m_diskCompareTable->rowCount();
        m_diskCompareTable->insertRow(row);
        m_diskCompareTable->setItem(row, 0, new QTableWidgetItem(m_diskModel->getAlgorithmName()));
        m_diskCompareTable->setItem(row, 1, new QTableWidgetItem(QString::number(m_diskModel->getTotalHeadMovement())));
        m_diskCompareTable->setItem(row, 2, new QTableWidgetItem(QString::number(m_diskModel->getAvgSeekTime(), 'f', 2)));
    }
}

// ----------------------------------------------------------------------------
// MODULE 9: ABOUT VIEW
// ----------------------------------------------------------------------------
AboutWidget::AboutWidget(QWidget* parent) : QWidget(parent) {
    setupUI();
}

void AboutWidget::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(15);

    QLabel* title = new QLabel("Mini Operating System Simulator", this);
    title->setObjectName("TitleLabel");
    title->setStyleSheet("font-size: 22px; font-weight: bold; color: #2563eb;");
    mainLayout->addWidget(title);

    QLabel* subtitle = new QLabel("Final Year Operating Systems Presentation & Demonstration", this);
    subtitle->setStyleSheet("font-size: 14px; color: #64748b; font-style: italic;");
    mainLayout->addWidget(subtitle);

    QFrame* divider = new QFrame(this);
    divider->setFrameShape(QFrame::HLine);
    divider->setFrameShadow(QFrame::Sunken);
    divider->setStyleSheet("background-color: #334155;");
    mainLayout->addWidget(divider);

    QScrollArea* scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    
    QWidget* content = new QWidget(scroll);
    QVBoxLayout* contentL = new QVBoxLayout(content);
    contentL->setSpacing(12);

    QLabel* intro = new QLabel(
        "This project is a highly interactive, desktop-based GUI application "
        "designed to simulate and visually demonstrate core Operating System "
        "concepts in real-time. Built entirely in C++ utilizing the Qt 6 Framework.",
        content
    );
    intro->setWordWrap(true);
    contentL->addWidget(intro);

    // Architectural features
    QGroupBox* archG = new QGroupBox("Software Architecture & Standards", content);
    QVBoxLayout* archL = new QVBoxLayout(archG);
    archL->addWidget(new QLabel("• <b>MVC Design Pattern:</b> Full separation between simulation data structures, algorithms, and rendering widgets.", content));
    archL->addWidget(new QLabel("• <b>Strategy Pattern:</b> Swappable algorithms for CPU schedulers, memory, pages, and disk seeking.", content));
    archL->addWidget(new QLabel("• <b>Observer Pattern:</b> Subject-Observer links updating vector views reactively on simulations.", content));
    archL->addWidget(new QLabel("• <b>Singleton Pattern:</b> Configuration and QSS theme managers switching styling values dynamically.", content));
    archL->addWidget(new QLabel("• <b>Custom Data Structures:</b> Circular Queues, LIFO Stacks, Doubly Linked Lists, N-ary Folder Trees, and Directed Graphs.", content));
    archG->setLayout(archL);
    contentL->addWidget(archG);

    // Developers Card
    QGroupBox* devG = new QGroupBox("Academic Course Project Details", content);
    QGridLayout* devGrid = new QGridLayout(devG);
    devGrid->addWidget(new QLabel("<b>Course:</b>", content), 0, 0);
    devGrid->addWidget(new QLabel("CS-302: Operating Systems (Lab & Theory)", content), 0, 1);
    devGrid->addWidget(new QLabel("<b>Framework:</b>", content), 1, 0);
    devGrid->addWidget(new QLabel("C++ / Qt 6.x (Desktop Core widgets)", content), 1, 1);
    devGrid->addWidget(new QLabel("<b>Database:</b>", content), 2, 0);
    devGrid->addWidget(new QLabel("Database-Free memory structure", content), 2, 1);
    devG->setLayout(devGrid);
    contentL->addWidget(devG);

    content->setLayout(contentL);
    scroll->setWidget(content);
    mainLayout->addWidget(scroll);
}
