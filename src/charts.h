#pragma once
#include <QQuickPaintedItem>
#include <QColor>
#include <QVariantList>

// Small retained chart textures; repaint only when a sample/property changes.
class HistoryPlot : public QQuickPaintedItem {
    Q_OBJECT
    Q_PROPERTY(QVariantList values READ values WRITE setValues NOTIFY changed)
    Q_PROPERTY(QVariantList secondaryValues READ secondaryValues WRITE setSecondaryValues NOTIFY changed)
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY changed)
    Q_PROPERTY(double maximum READ maximum WRITE setMaximum NOTIFY changed)
public:
    explicit HistoryPlot(QQuickItem *parent = nullptr);
    QVariantList values() const { return m_values; }
    QVariantList secondaryValues() const { return m_secondary; }
    QColor color() const { return m_color; }
    double maximum() const { return m_maximum; }
    void setValues(const QVariantList &values);
    void setSecondaryValues(const QVariantList &values);
    void setColor(const QColor &value);
    void setMaximum(double value);
    void paint(QPainter *painter) override;
signals:
    void changed();
private:
    QVariantList m_values, m_secondary;
    QColor m_color = QColor("#3ea9ff");
    double m_maximum = 100;
};

class GaugeArc : public QQuickPaintedItem {
    Q_OBJECT
    Q_PROPERTY(double value READ value WRITE setValue NOTIFY changed)
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY changed)
public:
    explicit GaugeArc(QQuickItem *parent = nullptr);
    double value() const { return m_value; }
    QColor color() const { return m_color; }
    void setValue(double value);
    void setColor(const QColor &color);
    void paint(QPainter *painter) override;
signals:
    void changed();
private:
    double m_value = -1;
    QColor m_color = QColor("#3ea9ff");
};
void registerVisualTypes();
