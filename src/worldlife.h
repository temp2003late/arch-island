#pragma once
#include <QQuickPaintedItem>
#include <QImage>

// Small inhabitants and atmosphere, sharing the island's pausable clock.
class WorldLife : public QQuickPaintedItem {
    Q_OBJECT
    Q_PROPERTY(double sceneTime READ sceneTime WRITE setSceneTime NOTIFY changed)
    Q_PROPERTY(double darkness READ darkness WRITE setDarkness NOTIFY changed)
public:
    explicit WorldLife(QQuickItem *parent = nullptr);
    double sceneTime() const { return m_time; }
    double darkness() const { return m_darkness; }
    void setSceneTime(double value);
    void setDarkness(double value);
    static constexpr int ResidentCount = 18;
    static QPointF residentPosition(int resident, double sceneTime);
    void paint(QPainter *painter) override;
signals:
    void changed();
private:
    double m_time = 0;
    double m_lastRepaint = -1000;
    double m_darkness = 0;
    bool waterAt(QPointF point, double margin = 0) const;
    QImage m_waterMask;
};
