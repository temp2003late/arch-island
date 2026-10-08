#include "sceneclock.h"
#include <algorithm>

SceneClock::SceneClock(QObject *parent) : QObject(parent) {
    m_timer.setInterval(33);
    m_timer.setTimerType(Qt::PreciseTimer);
    connect(&m_timer, &QTimer::timeout, this, [this] {
        const double dt = m_elapsed.nsecsElapsed() * 1e-9;
        m_elapsed.start();
        emit stepped(std::clamp(dt, 0.0, 0.1));
    });
}
void SceneClock::setRunning(bool value) {
    if (value == running()) return;
    if (value) { m_elapsed.start(); m_timer.start(); }
    else m_timer.stop();
    emit runningChanged();
}
