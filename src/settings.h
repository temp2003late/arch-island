#pragma once
#include <QObject>
#include <QSettings>
#ifdef ISLAND_HAS_DBUS
#include <QDBusVariant>
#endif
class Settings final : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString mode READ mode WRITE setMode NOTIFY changed)
    Q_PROPERTY(QString networkInterface READ networkInterface WRITE setNetworkInterface NOTIFY changed)
    Q_PROPERTY(bool reducedMotion READ reducedMotion WRITE setReducedMotion NOTIFY changed)
    Q_PROPERTY(bool systemReducedMotion READ systemReducedMotion NOTIFY changed)
public:
    explicit Settings(QObject *parent = nullptr);
    explicit Settings(const QString &file, QObject *parent = nullptr);
    QString mode() const;
    QString networkInterface() const;
    bool reducedMotion() const;
    bool systemReducedMotion() const { return m_gnomeReduced || m_kdeReduced; }
    void setMode(const QString &value);
    void setNetworkInterface(const QString &value);
    void setReducedMotion(bool value);
signals:
    void changed();
private:
    void observeSystemMotion();
    bool m_gnomeReduced = false;
    bool m_kdeReduced = false;
    QSettings m_settings;
#ifdef ISLAND_HAS_DBUS
private slots:
    void portalSettingChanged(const QString &group, const QString &key, const QDBusVariant &value);
#endif
};
