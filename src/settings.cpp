#include "settings.h"
Settings::Settings(QObject *p) : QObject(p) {}
Settings::Settings(const QString &file, QObject *p) : QObject(p), m_settings(file, QSettings::IniFormat) {}
QString Settings::mode() const { return m_settings.value("appearance/mode", "auto").toString(); }
QString Settings::networkInterface() const { return m_settings.value("network/interface").toString(); }
bool Settings::reducedMotion() const { return m_settings.value("appearance/reducedMotion", false).toBool(); }
void Settings::setMode(const QString &v) {
    if (v != "auto" && v != "day" && v != "night") return;
    if (mode() == v) return;
    m_settings.setValue("appearance/mode", v); m_settings.sync(); emit changed();
}
void Settings::setNetworkInterface(const QString &v) {
    if (networkInterface() == v) return;
    m_settings.setValue("network/interface", v); m_settings.sync(); emit changed();
}
void Settings::setReducedMotion(bool v) {
    if (reducedMotion() == v) return;
    m_settings.setValue("appearance/reducedMotion", v); m_settings.sync(); emit changed();
}
