#include "brandmark.h"
#include <QImage>
#include <QPainter>
#include <algorithm>
#include <cmath>

namespace {
struct Marks {
    QImage dark, light;
    Marks() {
        const QImage original(":/assets/brand/arthelm.webp");
        dark=QImage(original.size(),QImage::Format_ARGB32_Premultiplied);
        light=QImage(original.size(),QImage::Format_ARGB32_Premultiplied);
        for(int y=0;y<original.height();++y) for(int x=0;x<original.width();++x) {
            const QColor color=original.pixelColor(x,y);
            const double paper=std::min({color.redF(),color.greenF(),color.blueF()});
            const double t=std::clamp((paper-.72)/.20,0.,1.);
            const double alpha=color.alphaF()*(1-t*t*(3-2*t));
            QColor ink=color;
            // Unmatte the softened contour so it cannot leave a pale rectangle
            // or white fringe against navy panels. The original stays intact.
            if(alpha>.001) {
                ink.setRgbF(std::clamp((color.redF()-(1-alpha))/alpha,0.,1.),
                            std::clamp((color.greenF()-(1-alpha))/alpha,0.,1.),
                            std::clamp((color.blueF()-(1-alpha))/alpha,0.,1.),alpha);
            } else ink=Qt::transparent;
            dark.setPixel(x,y,qPremultiply(ink.rgba()));
            if(alpha>.001 && color.blueF()>color.redF() && color.redF()<.65) {
                ink.setRedF(.88); ink.setGreenF(.91); ink.setBlueF(.93);
            }
            light.setPixel(x,y,qPremultiply(ink.rgba()));
        }
    }
};
const Marks &marks() { static const Marks value; return value; }
}
BrandMark::BrandMark(QQuickItem *parent):QQuickPaintedItem(parent) {
    setImplicitSize(52,52);
    setAntialiasing(true);
    setFillColor(Qt::transparent);
}
void BrandMark::setLightInk(double value) {
    if(!std::isfinite(value)) return;
    value=std::clamp(value,0.,1.);
    if(m_lightInk==value) return;
    m_lightInk=value; update(); emit lightInkChanged();
}
void BrandMark::paint(QPainter *painter) {
    const auto &variants=marks();
    if(variants.dark.isNull() || width()<=0 || height()<=0) return;
    const double side=std::min(width(),height());
    const QRectF target((width()-side)/2,(height()-side)/2,side,side);
    painter->setRenderHint(QPainter::SmoothPixmapTransform);
    if(m_lightInk<=0 || m_lightInk>=1) {
        painter->drawImage(target,m_lightInk>=1 ? variants.light : variants.dark);
        return;
    }
    // Add premultiplied colors in a small transparent buffer. Alpha stays
    // constant through the crossfade, including the fine rigging edges.
    const double dpr=painter->device()->devicePixelRatioF();
    QImage frame(QSize(std::max(1,int(std::ceil(width()*dpr))),
                       std::max(1,int(std::ceil(height()*dpr)))),
                 QImage::Format_ARGB32_Premultiplied);
    frame.setDevicePixelRatio(dpr); frame.fill(Qt::transparent);
    {
        QPainter blend(&frame);
        blend.setRenderHint(QPainter::SmoothPixmapTransform);
        blend.setOpacity(1-m_lightInk);
        blend.drawImage(target,variants.dark);
        blend.setCompositionMode(QPainter::CompositionMode_Plus);
        blend.setOpacity(m_lightInk);
        blend.drawImage(target,variants.light);
    }
    painter->drawImage(QPointF(0,0),frame);
}
