// c++ -O2 -fPIC -std=c++20 scripts/create-scene-masks.cpp $(pkg-config --cflags --libs Qt6Gui) -o /tmp/island-masks
// Run /tmp/island-masks from the project root. Artwork is never modified.
#include <QImage>
#include <QPainter>
#include <QPolygonF>
#include <algorithm>
#include <cmath>
#include <cstdio>

int main() {
    const QImage day("assets/art/island.png"), night("assets/art/island_night.png");
    if (day.size()!=QSize(3840,2160) || night.size()!=day.size()) {
        std::fprintf(stderr,"Expected matching 3840x2160 island panoramas\n");
        return 1;
    }
    // Half-resolution mask; traced in the shared 1600x900 logical world.
    // Conservative outlines include both lighting versions and their tiny shifts.
    const QSize size(1920,1080);
    QImage water(size,QImage::Format_RGB32); water.fill(Qt::black);
    QPainter p(&water); p.scale(1.2,1.2);
    p.setPen(Qt::NoPen); p.setBrush(Qt::white);
    p.drawRect(QRectF(0,165,1600,735));
    p.setBrush(Qt::black);
    p.setPen(QPen(Qt::black,10,Qt::SolidLine,Qt::RoundCap,Qt::RoundJoin));
    p.drawPolygon(QPolygonF{
        {65,345},{100,315},{140,280},{145,220},{190,210},{220,225},
        {250,175},{290,145},{315,120},{345,55},{590,55},{640,140},
        {700,185},{750,215},{790,260},{835,265},{845,200},{890,200},
        {920,300},{985,330},{1080,330},{1140,270},{1180,205},{1250,205},
        {1290,250},{1345,245},{1355,135},{1415,135},{1430,255},
        {1490,270},{1525,350},{1535,390},{1575,455},{1540,480},
        {1500,470},{1470,485},{1415,490},{1370,480},{1320,495},
        {1280,490},{1245,510},{1180,505},{1120,495},{1080,520},
        {1020,525},{990,520},{945,520},{915,515},{885,510},
        {855,505},{825,510},{800,495},{770,490},{740,505},
        {710,510},{680,510},{655,515},{620,515},{590,525},
        {555,540},{510,545},{485,555},{440,565},{410,560},
        {385,585},{340,590},{300,570},{280,555},{250,550},
        {220,555},{180,545},{155,530},{110,530},{105,500},
        {75,485},{105,455},{85,420},{70,395}});
    // Working harbor, both decks and all perspective-projected piles.
    p.drawPolygon(QPolygonF{{990,470},{1080,460},{1225,465},{1335,485},
        {1345,520},{1290,540},{1290,575},{1245,590},{1200,578},
        {1110,555},{1040,535},{1010,520}});
    // Left waterfront pier and piles.
    p.drawPolygon(QPolygonF{{95,495},{235,500},{260,525},{245,550},
        {185,560},{100,545}});
    // Foreground rock shelves are separate from the island.
    p.drawPolygon(QPolygonF{{0,615},{65,615},{85,625},{135,610},
        {165,625},{190,635},{235,640},{265,670},{290,685},
        {330,690},{350,730},{390,735},{420,760},{440,785},
        {485,785},{510,815},{535,840},{580,840},{605,875},
        {655,895},{655,900},{0,900}});
    p.drawPolygon(QPolygonF{{1300,670},{1340,650},{1380,660},
        {1400,665},{1430,680},{1460,690},{1480,725},{1510,740},
        {1530,775},{1480,785},{1430,775},{1380,755},{1350,755},
        {1310,730},{1285,705}});
    // Small isolated rocks to the west and far-right coastline.
    for (const QRectF &r : {QRectF(5,470,75,30),QRectF(28,383,28,22),
                            QRectF(1550,420,48,35)}) p.drawEllipse(r);
    p.end();
    // Palm crowns only; no cloud motion, architecture, trunks or lava channel.
    const QRectF crowns[]={
        {495,48,75,65},{550,80,58,50},{620,135,65,60},
        {245,226,76,65},{320,275,82,65},{380,295,76,65},
        {427,320,82,65},{470,340,80,65},{505,372,70,62},
        {570,390,83,65},{610,419,75,60},{700,345,72,60},
        {730,235,65,55},{1100,273,80,65},{1160,208,85,70},
        {1210,215,70,65},{1270,248,65,55}};
    const QImage colors=day.scaled(size).convertToFormat(QImage::Format_RGB32);
    QImage mask(size,QImage::Format_RGB32);
    for(int y=0;y<size.height();++y) for(int x=0;x<size.width();++x) {
        int sea=qRed(water.pixel(x,y));
        // Feather inward with a solid coastal guard: displaced samples cannot
        // fetch rock, buildings, deck planks or their supports.
        if(sea) for(int dy=-4;dy<=4;++dy) for(int dx=-4;dx<=4;++dx) {
            const int sx=std::clamp(x+dx,0,size.width()-1);
            const int sy=std::clamp(y+dy,0,size.height()-1);
            if(!qRed(water.pixel(sx,sy)))
                sea=std::min(sea,int(std::max(0.,std::hypot(dx,dy)-1.)*65));
        }
        const QRgb color=colors.pixel(x,y);
        double leaf=0;
        // Warm sunlight makes some foliage yellow; reject blue water and pale
        // stone before testing a traced crown's free-tip attachment gradient.
        if(qGreen(color)>qBlue(color)*1.18 && qGreen(color)>qRed(color)*.80)
            for(const auto &r:crowns) {
                const QPointF point(x/1.2,y/1.2);
                const double a=(point.x()-r.center().x())/(r.width()*.5);
                const double b=(point.y()-r.center().y())/(r.height()*.5);
                if(a*a+b*b<1) leaf=std::max(leaf,
                    (1-std::clamp((point.y()-r.top())/r.height(),0.,1.))
                    *std::min(1.,(1-a*a-b*b)*6));
            }
        mask.setPixel(x,y,qRgb(sea,int(leaf*255),0));
    }
    if(!mask.save("assets/art/scene-mask.png")) return 2;
    std::printf("Shared sea / attached palm masks: %dx%d; blue channel unused\n",
                mask.width(),mask.height());
}
