#pragma once
#include <QObject>
#include <QElapsedTimer>
#include <QTimer>

// Wall-clock adjustments never affect simulation. Hidden windows resume at the
// saved phase; a stalled GUI drops excess time instead of catching up physics.
class SceneClock : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool running READ running WRITE setRunning NOTIFY runningChanged)
public:
    explicit SceneClock(QObject *parent = nullptr);
    bool running() const { return m_timer.isActive(); }
    void setRunning(bool running);
signals:
    void runningChanged();
    void stepped(double seconds);
private:
    QTimer m_timer;
    QElapsedTimer m_elapsed;
};
