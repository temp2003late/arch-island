#pragma once
#include <QQuickPaintedItem>

// Draw the existing raster mark on the parent surface without its white matte.
class BrandMark : public QQuickPaintedItem {
    Q_OBJECT
    Q_PROPERTY(double lightInk READ lightInk WRITE setLightInk NOTIFY lightInkChanged)
public:
    explicit BrandMark(QQuickItem *parent=nullptr);
    double lightInk() const { return m_lightInk; }
    void setLightInk(double value);
    void paint(QPainter *painter) override;
signals:
    void lightInkChanged();
private:
    double m_lightInk=0;
};
