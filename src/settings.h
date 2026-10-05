#pragma once
#include <QObject>
#include <QSettings>
class Settings final : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString mode READ mode WRITE setMode NOTIFY changed)
    Q_PROPERTY(QString networkInterface READ networkInterface WRITE setNetworkInterface NOTIFY changed)
    Q_PROPERTY(bool reducedMotion READ reducedMotion WRITE setReducedMotion NOTIFY changed)
public:
    explicit Settings(QObject *parent = nullptr);
    explicit Settings(const QString &file, QObject *parent = nullptr);
    QString mode() const;
    QString networkInterface() const;
    bool reducedMotion() const;
    void setMode(const QString &value);
    void setNetworkInterface(const QString &value);
    void setReducedMotion(bool value);
signals:
    void changed();
private:
    QSettings m_settings;
};
