#pragma once
#include <QQuickPaintedItem>
#include <memory>

// Small orthographic 3D mesh rendered into a transparent Qt Quick item.
// QPainter also supports the software scene graph used on machines without a GPU.
class Sailboat : public QQuickPaintedItem {
    Q_OBJECT
    Q_PROPERTY(double heading READ heading WRITE setHeading NOTIFY changed)
    Q_PROPERTY(double sceneTime READ sceneTime WRITE setSceneTime NOTIFY changed)
    Q_PROPERTY(double darkness READ darkness WRITE setDarkness NOTIFY changed)
public:
    explicit Sailboat(QQuickItem *parent = nullptr);
    ~Sailboat() override;
    double heading() const { return m_heading; }
    double sceneTime() const { return m_time; }
    double darkness() const { return m_darkness; }
    void setHeading(double value);
    void setSceneTime(double value);
    void setDarkness(double value);
    void paint(QPainter *painter) override;
signals:
    void changed();
private:
    struct Mesh;
    std::unique_ptr<Mesh> m_mesh;
    double m_heading = 0;
    double m_time = 0;
    double m_lastRepaint = -1000;
    double m_darkness = 0;
};
