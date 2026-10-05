#include "charts.h"
#include "history.h"
#include <QPainter>
#include <QPainterPath>
#include <QQmlEngine>
#include <algorithm>
#include <cmath>

HistoryPlot::HistoryPlot(QQuickItem *p) : QQuickPaintedItem(p) {
    setAntialiasing(true);
    setImplicitSize(180, 42);
}
void HistoryPlot::setValues(const QVariantList &v) { if (m_values == v) return; m_values=v; update(); emit changed(); }
void HistoryPlot::setSecondaryValues(const QVariantList &v) { if (m_secondary == v) return; m_secondary=v; update(); emit changed(); }
void HistoryPlot::setColor(const QColor &v) { if (m_color == v) return; m_color=v; update(); emit changed(); }
void HistoryPlot::setMaximum(double v) { if (m_maximum == v) return; m_maximum=v; update(); emit changed(); }
void HistoryPlot::paint(QPainter *p) {
    if (width() < 4 || height() < 4) return;
    p->setRenderHint(QPainter::Antialiasing);
    double ceiling = m_maximum;
    if (ceiling <= 0) {
        ceiling = 1;
        for (const auto &v : m_values) ceiling = std::max(ceiling, v.toDouble()*1.12);
        for (const auto &v : m_secondary) ceiling = std::max(ceiling, v.toDouble()*1.12);
    }
    const QRectF area(3, 3, width()-6, height()-7);
    p->setPen(QPen(QColor(136,169,202,20), 0.7));
    p->drawLine(QPointF(0,area.bottom()), QPointF(width(),area.bottom()));
    auto drawSeries = [&](const QVariantList &values, QColor color, bool fill) {
        QVector<QPointF> segment;
        auto flush = [&] {
            if (segment.isEmpty()) return;
            QPainterPath line; line.moveTo(segment.first());
            for (qsizetype i=1;i<segment.size();++i) {
                const auto &a=segment[i-1], &b=segment[i];
                const double mid=(a.x()+b.x())/2;
                line.cubicTo(QPointF(mid,a.y()),QPointF(mid,b.y()),b);
            }
            if (fill && segment.size()>1) {
                auto shape=line;
                shape.lineTo(segment.last().x(),area.bottom());
                shape.lineTo(segment.first().x(),area.bottom()); shape.closeSubpath();
                QLinearGradient gradient(0,0,0,height());
                auto top=color; top.setAlphaF(0.22); auto bottom=color; bottom.setAlphaF(0.015);
                gradient.setColorAt(0,top); gradient.setColorAt(1,bottom);
                p->fillPath(shape,gradient);
            }
            auto glow=color; glow.setAlphaF(0.1);
            p->setBrush(Qt::NoBrush);
            p->setPen(QPen(glow,5,Qt::SolidLine,Qt::RoundCap,Qt::RoundJoin)); p->drawPath(line);
            p->setPen(QPen(color,1.4,Qt::SolidLine,Qt::RoundCap,Qt::RoundJoin)); p->drawPath(line);
            segment.clear();
        };
        const int count=std::min(int(values.size()),SampleHistory::Capacity);
        for (int i=0;i<count;++i) {
            const double value=values[values.size()-count+i].toDouble();
            if (!std::isfinite(value) || value<0) { flush(); continue; }
            const double x=area.left()+area.width()*(SampleHistory::Capacity-count+i)/(SampleHistory::Capacity-1);
            const double y=area.bottom()-area.height()*std::clamp(value/ceiling,0.0,1.0);
            segment.append(QPointF(x,y));
        }
        if (!segment.isEmpty()) {
            const auto last=segment.last();
            flush(); p->setPen(Qt::NoPen); p->setBrush(color); p->drawEllipse(last,1.9,1.9);
        }
    };
    drawSeries(m_values,m_color,true);
    if (!m_secondary.isEmpty()) drawSeries(m_secondary,QColor("#93d7df"),false);
}
GaugeArc::GaugeArc(QQuickItem *p) : QQuickPaintedItem(p) { setAntialiasing(true); setImplicitSize(54,54); }
void GaugeArc::setValue(double v) { if (m_value==v) return; m_value=v; update(); emit changed(); }
void GaugeArc::setColor(const QColor &v) { if (m_color==v) return; m_color=v; update(); emit changed(); }
void GaugeArc::paint(QPainter *p) {
    p->setRenderHint(QPainter::Antialiasing);
    const double size=std::min(width(),height());
    const QRectF rect((width()-size)/2+4,(height()-size)/2+4,size-8,size-8);
    auto track=m_color; track.setAlphaF(0.12);
    p->setBrush(Qt::NoBrush); p->setPen(QPen(track,4.5,Qt::SolidLine,Qt::RoundCap)); p->drawEllipse(rect);
    if (m_value>=0 && std::isfinite(m_value)) {
        p->setPen(QPen(m_color,4.5,Qt::SolidLine,Qt::RoundCap));
        p->drawArc(rect,90*16,-qRound(std::clamp(m_value,0.0,1.0)*360*16));
    }
}
void registerVisualTypes() {
    qmlRegisterType<HistoryPlot>("ArchIsland",1,0,"HistoryPlot");
    qmlRegisterType<GaugeArc>("ArchIsland",1,0,"GaugeArc");
}
