#ifndef SYSTEM_CONFIG_H
#define SYSTEM_CONFIG_H

#include <QObject>
#include <QString>

class SystemConfig : public QObject {
    Q_OBJECT
    
private:
    // Private constructor for Singleton
    SystemConfig();
    ~SystemConfig() = default;

    bool m_darkTheme;
    int m_simulationSpeedMs;

public:
    // Delete copy constructors
    SystemConfig(const SystemConfig&) = delete;
    SystemConfig& operator=(const SystemConfig&) = delete;

    // Static retrieval instance
    static SystemConfig& getInstance();

    // Theme Accessors
    bool isDarkTheme() const;
    void setDarkTheme(bool dark);

    // Simulation Speed Accessors (delay in ms)
    int getSimulationSpeedMs() const;
    void setSimulationSpeedMs(int speedMs);

    // Get current QSS stylesheet contents
    QString getStylesheet() const;

signals:
    void themeChanged(bool isDark);
};

#endif // SYSTEM_CONFIG_H
