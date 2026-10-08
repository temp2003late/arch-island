#include "worldlife.h"
#include <QPainter>
#include <QPainterPath>
#include <QRadialGradient>
#include <QLineF>
#include <algorithm>
#include <array>
#include <cmath>

namespace {
constexpr double pi = 3.141592653589793;
double cycle(double value) { return value - std::floor(value); }
void glow(QPainter *p, QPointF point, double radius, QColor color) {
    QRadialGradient gradient(point, radius);
    gradient.setColorAt(0, color);
    color.setAlpha(0);
    gradient.setColorAt(1, color);
    p->setPen(Qt::NoPen);
    p->setBrush(gradient);
    p->drawEllipse(point, radius, radius * .65);
}
struct Route { QPointF a, b, c; };
// Foot positions traced on the panorama at its QML size (2000x1000, x=-200).
// Piecewise straight segments preserve bends and stair landings. Never add a
// lateral offset: narrow stairs/bridges have no room for decorative wandering.
const std::array<QPolygonF,6> residentRoutes = {{
    // Western stairway, including the turn on the landing.
    {{502,488},{510,476},{520,460},{525,450},{541,452},{548,439},{550,417},{551,398},{548,386}},
    // Central street, around the church and down to the lower square.
    {{704,475},{713,492},{724,501},{747,505},{770,502},{795,501},{808,507},{832,521},{850,534},{860,545}},
    // Lower promenade and its stone bridge.
    {{867,546},{882,548},{915,550},{949,548},{976,540},{1005,531},{1037,528},{1070,525}},
    // Clear strip of the main dock, inside the bend and away from stacked cargo.
    {{991,672},{1027,668},{1061,660},{1100,650}},
    // Outer dock: follow the actual decking, not a diagonal over the bay.
    {{1190,668},{1223,674},{1252,680},{1270,684}},
    // Upper stone bridge; stop before the lighthouse buildings.
    {{980,386},{1000,385},{1020,382},{1040,379},{1060,377}}
}};
const auto routeLengths = [] {
    std::array<double,6> lengths{};
    for (size_t route=0; route<residentRoutes.size(); ++route)
        for (qsizetype i=1; i<residentRoutes[route].size(); ++i)
            lengths[route] += QLineF(residentRoutes[route][i-1],residentRoutes[route][i]).length();
    return lengths;
}();
double residentProgress(int resident, double time) {
    const double phase = cycle(time/(48+resident*2.7)+resident*.381966);
    return std::clamp((1-std::cos(phase*2*pi))*.57-.07,0.0,1.0);
}
QPointF alongStreet(int route, double progress) {
    const auto &points = residentRoutes[route];
    double remaining = progress*routeLengths[route];
    for (qsizetype i=1; i<points.size(); ++i) {
        const double length = QLineF(points[i-1],points[i]).length();
        if (remaining <= length)
            return points[i-1]+(points[i]-points[i-1])*(remaining/length);
        remaining -= length;
    }
    return points.last();
}
}

QPointF WorldLife::residentPosition(int resident, double sceneTime) {
    if (resident<0 || resident>=ResidentCount || !std::isfinite(sceneTime)) return {};
    return alongStreet(resident%int(residentRoutes.size()),residentProgress(resident,sceneTime));
}

WorldLife::WorldLife(QQuickItem *parent) : QQuickPaintedItem(parent) {
    setAntialiasing(true);
}
void WorldLife::setSceneTime(double value) {
    if (!std::isfinite(value) || value == m_time) return;
    m_time = value; update(); emit changed();
}
void WorldLife::setDarkness(double value) {
    if (!std::isfinite(value)) return;
    value = std::clamp(value, 0.0, 1.0);
    if (value == m_darkness) return;
    m_darkness = value; update(); emit changed();
}
void WorldLife::paint(QPainter *p) {
    if (width() <= 0 || height() <= 0) return;
    p->save();
    p->scale(width()/1600, height()/1000);
    p->setRenderHint(QPainter::Antialiasing);
    const double t = m_time, night = m_darkness;

    // Perspective-compressed ripples in open water; no waves laid over the island.
    const QRectF water[] = {{-180,785,1900,205},{-170,450,270,310},
                            {1390,380,380,390},{260,630,290,110}};
    for (int region=0; region<4; ++region) {
        const auto area = water[region];
        for (int i=0; i<32; ++i) {
            const double phase = cycle(t * (.065 + (i%5)*.007) + i*.618);
            const double x = area.x() + cycle(i*.754877 + region*.21)*area.width();
            const double y = area.y() + cycle(i*.56984)*area.height() + phase*8;
            const double depth = std::clamp((y-260)/740, .08, 1.0);
            QColor foam(170,228,239, int((16+26*(1-night))*std::sin(pi*phase)));
            p->setPen(QPen(foam, .5 + depth*.8, Qt::SolidLine, Qt::RoundCap));
            p->setBrush(Qt::NoBrush);
            QPainterPath ripple;
            const double span = (5+phase*26)*depth;
            ripple.moveTo(x-span,y);
            ripple.cubicTo(x-span*.4,y-2*depth,x+span*.4,y+2*depth,x+span,y);
            p->drawPath(ripple);
        }
    }

    // Breakers spread along the beach, then dissolve instead of pulsing in place.
    const Route shores[] = {
        {{410,574},{480,574},{595,625}},
        {{560,647},{535,680},{545,710}},
        {{719,789},{843,824},{959,846}},
        {{1268,543},{1330,525},{1370,558}}
    };
    for (int shore=0; shore<4; ++shore) for (int i=0; i<3; ++i) {
        const double phase = cycle(t/(4.6+shore*.6)+i/3.0+shore*.23);
        const double strength = std::sin(pi*phase) * (.5+.5*std::sin(pi*phase));
        const QPointF drift(phase*4,phase*8);
        const auto &r = shores[shore];
        QPainterPath breaker;
        breaker.moveTo(r.a+drift);
        breaker.quadTo(r.b+drift,r.c+drift);
        p->setPen(QPen(QColor(158,218,230,int(strength*(night>0.5 ? 25 : 45))),4+phase*3));
        p->drawPath(breaker);
        p->setPen(QPen(QColor(215,246,247,int(strength*(night>0.5 ? 70 : 110))),.8+phase*.7));
        p->drawPath(breaker);
    }

    // Fishing skiffs live on the distant sea; shadows, sails and wakes give volume.
    for (int i=0; i<3; ++i) {
        const double phase = t*(.012+i*.002)+i*2.1;
        const QPointF center(i==0 ? 75+115*std::sin(phase) : 1400+130*std::sin(phase),
                             305+i*68+3*std::sin(t*.7+i));
        p->save(); p->translate(center); p->scale(.45+i*.12,.45+i*.12);
        p->setPen(QPen(QColor(167,218,230,80),1));
        p->drawLine(QPointF(-32,5),QPointF(-12,2));
        p->drawLine(QPointF(-29,8),QPointF(-11,4));
        p->setPen(Qt::NoPen); p->setBrush(QColor(0,20,35,70));
        p->drawEllipse(QPointF(0,5),19,3);
        p->setBrush(QColor("#563b2d"));
        p->drawPolygon(QPolygonF{QPointF(-17,0),QPointF(18,-2),QPointF(11,6),QPointF(-10,6)});
        p->setPen(QPen(QColor("#b08d61"),1.3)); p->drawLine(QPointF(0,1),QPointF(0,-33));
        p->setPen(Qt::NoPen);
        p->setBrush(QColor::fromRgbF(.85-night*.4,.83-night*.4,.69-night*.3));
        p->drawPolygon(QPolygonF{QPointF(-1,-31),QPointF(-1,-3),QPointF(-15,-4+std::sin(t+i))});
        p->setBrush(QColor::fromRgbF(.95-night*.4,.9-night*.4,.76-night*.3));
        p->drawPolygon(QPolygonF{QPointF(2,-29),QPointF(16,-5),QPointF(2,-3)});
        p->restore();
    }

    // Near gulls bank over the bay: a larger silhouette and faster angular motion
    // than the distant flock make the foreground feel separate from the skyline.
    for (int i=0; i<3; ++i) {
        const double phase = t*.055+i*.48;
        const QPointF position(280+155*std::sin(phase),655+48*std::cos(phase*.83));
        p->save(); p->translate(position); p->rotate(12*std::cos(phase));
        const double flap = std::sin(t*3.8+i)*4;
        QPainterPath wings;
        wings.moveTo(-10,-2-flap); wings.quadTo(-5,-3,0,0);
        wings.quadTo(5,-3,10,-2-flap);
        p->setPen(QPen(QColor(13,37,52,int(70*(1-night*.6))),3,Qt::SolidLine,Qt::RoundCap));
        p->drawPath(wings);
        p->setPen(QPen(QColor(214,227,227,int(210-night*130)),1.5,Qt::SolidLine,Qt::RoundCap));
        p->drawPath(wings);
        p->restore();
    }

    // Residents walk, pause, then return instead of teleporting at route ends.
    for (int i=0; i<ResidentCount; ++i) {
        const double distance = residentProgress(i,t);
        const QPointF point = residentPosition(i,t);
        const double size = i%6 == 5 ? .65 : i%6 == 3 ? 1.1 : .85;
        const double stride = distance>0 && distance<1 ? std::sin(t*5+i)*1.1 : 0;
        p->save(); p->translate(point); p->scale(size,size);
        p->setPen(Qt::NoPen); p->setBrush(QColor(5,15,24,90));
        p->drawEllipse(QPointF(2,1),3.6,1.25);
        p->setPen(QPen(QColor("#313745"),1.1,Qt::SolidLine,Qt::RoundCap));
        p->drawLine(QPointF(-.7,-2),QPointF(-1+stride,0));
        p->drawLine(QPointF(.7,-2),QPointF(1-stride,0));
        const QColor clothes[] = {QColor("#e4b078"),QColor("#73b9b0"),QColor("#c78069"),QColor("#cbd6cf")};
        p->setPen(QPen(clothes[i%4].darker(int(100+night*55)),2.6,Qt::SolidLine,Qt::RoundCap));
        p->drawLine(QPointF(0,-5),QPointF(0,-2));
        p->setPen(Qt::NoPen); p->setBrush(QColor("#cda17a")); p->drawEllipse(QPointF(0,-7),1.2,1.3);
        if (i%4==0 && night>.01) {
            glow(p,QPointF(3,-3),9,QColor(255,180,72,int(85*night)));
            p->setBrush(QColor(255,215,135,int(230*night))); p->drawEllipse(QPointF(3,-3),.8,1);
        }
        p->restore();
    }

    // Cooking smoke rises from inhabited houses, with different wind and timing.
    const QPointF chimneys[] = {{509,307},{678,364},{736,414},{986,476},{1121,477}};
    for (int chimney=0; chimney<5; ++chimney) for (int i=0; i<4; ++i) {
        const double phase = cycle(t/(9+chimney*.8)+i*.25+chimney*.17);
        const QPointF point = chimneys[chimney]+QPointF(phase*24+std::sin(t*.3+chimney)*phase*8,-phase*44);
        glow(p,point,3+phase*15,QColor(190,202,204,int(std::sin(phase*pi)*(36-18*night))));
    }

    // Fireflies stay around gardens. Their light is reflected in a soft local halo.
    for (int i=0; i<22; ++i) {
        const double pulse = std::pow(std::max(0.0,std::sin(t*(.7+i*.023)+i*2.4)),3)*night;
        if (pulse<.02) continue;
        const QPointF point(520+cycle(i*.618)*570+7*std::sin(t*.5+i),
                            450+cycle(i*.382)*160+5*std::sin(t*.7+i*3));
        glow(p,point,6,QColor(185,245,113,int(80*pulse)));
        p->setPen(Qt::NoPen); p->setBrush(QColor(226,255,160,int(220*pulse)));
        p->drawEllipse(point,.85,.85);
    }

    // A rare meteor, fading before it reaches the distant ridge.
    const double meteor = cycle((t+19)/67)*67;
    if (meteor<1.3 && night>.01) {
        const QPointF head(1040+meteor*170,42+meteor*75);
        QLinearGradient tail(head-QPointF(65,29),head);
        tail.setColorAt(0,Qt::transparent);
        tail.setColorAt(1,QColor(211,232,255,int(180*night*std::sin(meteor/1.3*pi))));
        p->setPen(QPen(QBrush(tail),1.4)); p->drawLine(head-QPointF(65,29),head);
    }
    p->restore();
}
