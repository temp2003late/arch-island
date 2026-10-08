#include "settings.h"
#include <utility>
#ifdef ISLAND_HAS_DBUS
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>
#endif
Settings::Settings(QObject *p) : QObject(p) { observeSystemMotion(); }
Settings::Settings(const QString &file, QObject *p) : QObject(p), m_settings(file, QSettings::IniFormat) { observeSystemMotion(); }
void Settings::observeSystemMotion() {
#ifdef ISLAND_HAS_DBUS
    auto bus = QDBusConnection::sessionBus();
    bus.connect("org.freedesktop.portal.Desktop", "/org/freedesktop/portal/desktop",
                "org.freedesktop.portal.Settings", "SettingChanged", this,
                SLOT(portalSettingChanged(QString,QString,QDBusVariant)));
    const std::pair<QString,QString> keys[] = {
        {"org.gnome.desktop.interface","enable-animations"},
        {"org.kde.kdeglobals.KDE","AnimationDurationFactor"}};
    for (const auto &[group,key] : keys) {
        auto call=QDBusMessage::createMethodCall("org.freedesktop.portal.Desktop",
            "/org/freedesktop/portal/desktop","org.freedesktop.portal.Settings","Read");
        call << group << key;
        auto *watcher=new QDBusPendingCallWatcher(bus.asyncCall(call),this);
        connect(watcher,&QDBusPendingCallWatcher::finished,this,[this,group,key](QDBusPendingCallWatcher *done) {
            const QDBusPendingReply<QDBusVariant> reply=*done;
            if (!reply.isError()) portalSettingChanged(group,key,reply.value());
            done->deleteLater();
        });
    }
#endif
}
#ifdef ISLAND_HAS_DBUS
void Settings::portalSettingChanged(const QString &group, const QString &key, const QDBusVariant &wrapped) {
    QVariant value=wrapped.variant();
    if (value.metaType()==QMetaType::fromType<QDBusVariant>()) value=value.value<QDBusVariant>().variant();
    const bool before=systemReducedMotion();
    if (group=="org.gnome.desktop.interface" && key=="enable-animations" && value.metaType()==QMetaType::fromType<bool>())
        m_gnomeReduced=!value.toBool();
    if (group=="org.kde.kdeglobals.KDE" && key=="AnimationDurationFactor") {
        bool valid=false;
        const double factor=value.toDouble(&valid);
        if (valid) m_kdeReduced=factor<=0;
    }
    if (before!=systemReducedMotion()) emit changed();
}
#endif
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
