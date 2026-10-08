#include "sailboat.h"
#include <QPainter>
#include <QRadialGradient>
#include <QVector3D>
#include <algorithm>
#include <cmath>
#include <vector>

namespace {
using V = QVector3D;
constexpr double pi = 3.14159265358979323846;
struct Face {
    std::vector<V> vertices;
    QColor color;
    float stroke = 0;
    bool luminous = false;
    bool cloth = false;
};
struct Projected {
    QPolygonF polygon;
    QColor color;
    float depth = 0;
    float stroke = 0;
    bool luminous = false;
};
}

Sailboat::Sailboat(QQuickItem *parent) : QQuickPaintedItem(parent) {
    setAntialiasing(true);
    setImplicitSize(190, 190);
}
void Sailboat::setHeading(double v) { if (std::isfinite(v) && m_heading != v) { m_heading=v; update(); emit changed(); } }
void Sailboat::setSceneTime(double v) { if (std::isfinite(v) && m_time != v) { m_time=v; update(); emit changed(); } }
void Sailboat::setDarkness(double v) { if (std::isfinite(v) && m_darkness != v) { m_darkness=std::clamp(v,0.0,1.0); update(); emit changed(); } }

void Sailboat::paint(QPainter *p) {
    if (width() <= 0 || height() <= 0) return;
    std::vector<Face> faces;
    faces.reserve(600);
    auto face = [&](std::initializer_list<V> v, QColor c) { faces.push_back({v,c}); };
    auto line = [&](V a, V b, QColor c, float w = 0.65f) { faces.push_back({{a,b},c,w}); };
    auto box = [&](float x, float y, float z, float length, float tall, float wide, QColor color) {
        V a(x,y,z), b(x+length,y,z), c(x+length,y,z+wide), d(x,y,z+wide);
        V up(0,tall,0);
        face({a,b,b+up,a+up},color); face({b,c,c+up,b+up},color);
        face({c,d,d+up,c+up},color); face({d,a,a+up,d+up},color);
        face({a+up,b+up,c+up,d+up},color.lighter(130));
    };
    auto spar = [&](V a, V b, float radius, QColor color) {
        V axis=(b-a).normalized();
        V u=V::crossProduct(axis,V(0,0,1)).normalized()*radius;
        V v=V::crossProduct(axis,u).normalized()*radius;
        for (int i=0;i<8;++i) {
            double t0=i*pi/4, t1=(i+1)*pi/4;
            V r0=u*std::cos(t0)+v*std::sin(t0), r1=u*std::cos(t1)+v*std::sin(t1);
            face({a+r0,b+r0,b+r1,a+r1},color);
        }
    };
    // Rounded cross sections give the hull real beam and depth, including at 90°.
    constexpr int stations=16, rows=5;
    auto hull = [](int station, int row, float side) {
        float t=float(station)/stations, level=float(row)/rows;
        // Rounding at pi can make sine negative; a fractional power would yield NaN.
        float beam=21*std::pow(std::max(0.f,std::sin(float(pi)*t)),0.62f);
        float rim=4+5*std::pow(2*t-1,4);
        return V(-78+156*t, rim-24*level, side*beam*std::cos(level*float(pi)*0.47f));
    };
    for (float side : {-1.f,1.f}) {
        for (int i=0;i<stations;++i) {
            for (int r=0;r<rows;++r) {
                QColor wood = QColor::fromRgb(112+r*5+(i%3)*3,57+r*2,29+r);
                face({hull(i,r,side),hull(i+1,r,side),hull(i+1,r+1,side),hull(i,r+1,side)},wood);
                line(hull(i,r,side),hull(i+1,r,side),r==0 ? QColor("#d5b177") : QColor("#513420"),r==0 ? 1.3f : .4f);
            }
            const V a=hull(i,0,side), b=hull(i+1,0,side);
            line(a+V(0,8,0),b+V(0,8,0),QColor("#b99258"),.9f);
            if (i%2 == 0) spar(a,a+V(0,8,0),.55,QColor("#9b713c"));
            if (side < 0)
                face({hull(i,0,-1),hull(i+1,0,-1),hull(i+1,0,1),hull(i,0,1)},QColor("#be945d"));
        }
    }
    for (int i=1;i<stations;++i)
        line(hull(i,0,-1)+V(0,.15,0),hull(i,0,1)+V(0,.15,0),QColor("#725132"),.4f);
    box(-45,6,-11,28,12,22,QColor("#80512e"));
    box(-48,18,-13,34,2,26,QColor("#b08148"));
    // Warm windows on both cabin sides remain attached to its geometry.
    for (float side : {-1.f,1.f}) for (int i=0;i<3;++i) {
        float x=-40+i*8, z=side*11.1f;
        faces.push_back({{V(x,10,z),V(x+4,10,z),V(x+4,15,z),V(x,15,z)},QColor("#ffc971"),0,true});
    }
    spar(V(0,4,0),V(0,144,0),1.65,QColor("#a56e34"));
    spar(V(65,6,0),V(90,13,0),.9,QColor("#ac8047"));
    spar(V(0,26,0),V(-69,28,4),1,QColor("#ac8047"));
    auto sail = [&](V a, V b, V c, float phase) {
        constexpr int divisions=10;
        auto vertex = [&](float u,float v) {
            V point=a*(1-u-v)+b*u+c*v;
            float billow=std::sin(pi*u)*std::sin(pi*v)*std::sin(pi*(1-u-v));
            point.setZ(point.z()+billow*(12+1.2*std::sin(m_time*1.1+phase+point.y()*.04)));
            return point;
        };
        for (int i=0;i<divisions;++i) for (int j=0;j<divisions-i;++j) {
            float u=float(i)/divisions, v=float(j)/divisions, step=1.f/divisions;
            faces.push_back({{vertex(u,v),vertex(u+step,v),vertex(u,v+step)},QColor("#eedcb8"),0,false,true});
            if (i+j<divisions-1)
                faces.push_back({{vertex(u+step,v),vertex(u+step,v+step),vertex(u,v+step)},QColor("#eedcb8"),0,false,true});
        }
        for (int i=1;i<5;++i) {
            float u=float(i)/5;
            for (int j=0;j<8;++j) {
                float v=(1-u)*j/8, next=(1-u)*(j+1)/8;
                line(vertex(u,v),vertex(u,next),QColor("#bba785"),.35f);
            }
        }
        line(a,b,QColor("#aa9165")); line(b,c,QColor("#aa9165")); line(c,a,QColor("#aa9165"));
    };
    sail(V(-1,137,0),V(-68,28,4),V(-1,26,0),0);
    sail(V(4,119,0),V(83,13,0),V(7,26,0),1.9);
    for (float side : {-1.f,1.f}) {
        line(V(0,138,0),V(-58,10,side*15),QColor("#806d4f"),.5);
        line(V(0,120,0),V(22,6,side*19),QColor("#806d4f"),.5);
        line(V(0,142,0),V(88,13,0),QColor("#8d7853"),.6);
    }
    for (float side : {-1.f,1.f}) {
        spar(V(-58,9,side*15),V(-58,18,side*15),.5,QColor("#b78e44"));
        faces.push_back({{V(-58,19,side*15)},QColor("#ffc977"),0,true});
    }
    // Orthographic camera at 24 degrees above the water; yaw turns real vertices.
    const double yaw=m_heading*pi/180, elevation=24*pi/180;
    const double roll=.012*std::sin(m_time*.8);
    auto rotate = [&](V v) {
        float y=v.y()*std::cos(roll)-v.z()*std::sin(roll);
        float z=v.y()*std::sin(roll)+v.z()*std::cos(roll);
        return V(v.x()*std::cos(yaw)-z*std::sin(yaw), y,
                 v.x()*std::sin(yaw)+z*std::cos(yaw));
    };
    auto project = [&](V v) { return QPointF(95+v.x()*.88,142+(-v.y()*std::cos(elevation)+v.z()*std::sin(elevation))*.88); };
    p->setRenderHint(QPainter::Antialiasing);
    p->scale(width()/190,height()/190);
    // The wake is on the water plane and follows the stern through the entire turn.
    p->setBrush(Qt::NoBrush);
    p->setPen(QPen(QColor(169,221,228,48),.7,Qt::SolidLine,Qt::RoundCap));
    for (float side : {-1.f,1.f}) {
        QPolygonF wake;
        for(int i=0;i<8;++i) {
            float x=-72-i*2.7f;
            wake << project(rotate(V(x,-8,side*(5+i*1.2f)+.7*std::sin(m_time+i))));
        }
        p->drawPolyline(wake);
    }
    std::vector<Projected> projected;
    projected.reserve(faces.size());
    const V light=V(-.4,.8,.5).normalized();
    for (const Face &f : faces) {
        Projected out;
        out.color=f.color; out.stroke=f.stroke; out.luminous=f.luminous;
        for (V v : f.vertices) {
            v=rotate(v);
            out.polygon << project(v);
            out.depth += v.z()*std::cos(elevation)+v.y()*std::sin(elevation);
        }
        out.depth /= f.vertices.size();
        if (f.vertices.size()>2 && !f.luminous) {
            const V normal=V::crossProduct(rotate(f.vertices[1]-f.vertices[0]),rotate(f.vertices[2]-f.vertices[0])).normalized();
            const float diffuse=std::abs(V::dotProduct(normal,light));
            const float shade=(.64f+.36f*diffuse)*(1-.12f*m_darkness);
            out.color.setRedF(out.color.redF()*shade);
            out.color.setGreenF(out.color.greenF()*shade);
            out.color.setBlueF(out.color.blueF()*shade);
        }
        projected.push_back(std::move(out));
    }
    std::stable_sort(projected.begin(),projected.end(),[](const auto &a,const auto &b){ return a.depth<b.depth; });
    for (const auto &f : projected) {
        if (f.polygon.size()==1) {
            QPointF center=f.polygon.first();
            const float pulse=.85+.15*std::sin(m_time*1.7+f.depth);
            QRadialGradient glow(center,5);
            QColor warm(255,188,83,int((50+80*m_darkness)*pulse));
            glow.setColorAt(0,warm); warm.setAlpha(0); glow.setColorAt(1,warm);
            p->setPen(Qt::NoPen); p->setBrush(glow); p->drawEllipse(center,5,5);
            p->setBrush(QColor("#ffe6a0")); p->drawEllipse(center,1.1,1.6);
        } else if (f.stroke>0) {
            p->setPen(QPen(f.color,f.stroke,Qt::SolidLine,Qt::RoundCap));
            p->setBrush(Qt::NoBrush); p->drawPolyline(f.polygon);
        } else {
            QColor color=f.color;
            if(f.luminous) color.setAlphaF(.78+.12*std::sin(m_time*1.3+f.depth));
            // Matching hairline edges prevent cracks between antialiased sail triangles.
            p->setPen(QPen(color,.25)); p->setBrush(color); p->drawPolygon(f.polygon);
        }
    }
}
