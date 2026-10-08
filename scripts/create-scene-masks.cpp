// Rebuild with: c++ -std=c++20 scripts/create-scene-masks.cpp $(pkg-config --cflags --libs Qt6Gui) -o /tmp/island-masks && /tmp/island-masks
// Coordinates traced on the existing 1774x887 panorama. Artwork is never edited.
#include <QImage>
#include <QPainter>
#include <QPolygonF>
#include <cmath>
#include <algorithm>
#include <cstdio>
int main() {
    const QImage day("assets/art/island-day-wide.png");
    const QImage night("assets/art/island-night-wide.png");
    if(day.size()!=QSize(1774,887) || night.size()!=day.size()) return 1;
    QImage water(day.size(), QImage::Format_RGB32); water.fill(Qt::black);
    QPainter p(&water); p.setPen(Qt::NoPen); p.setBrush(Qt::white);
    p.drawRect(0,246,1774,641);
    p.setBrush(Qt::black); p.setPen(QPen(Qt::black,10,Qt::SolidLine,Qt::RoundCap,Qt::RoundJoin));
    p.drawPolygon(QPolygonF{{402,240},{404,268},{384,292},{369,318},{341,333},{315,358},
      {330,382},{394,397},{451,417},{480,434},{466,452},{515,481},{557,499},
      {614,517},{646,531},{676,522},{700,539},{739,548},{765,536},{809,550},
      {838,570},{851,596},{874,611},{912,612},{943,616},{974,643},{996,666},
      {1033,664},{1031,646},{1013,625},{1077,617},{1121,596},{1131,583},
      {1157,600},{1202,617},{1236,631},{1267,635},{1300,648},{1343,651},
      {1465,676},{1478,648},{1468,621},{1417,608},{1397,584},{1365,575},
      {1345,546},{1329,511},{1341,496},{1320,473},{1325,452},{1307,421},
      {1297,382},{1260,357},{1210,329},{1156,320},{1111,319},{1060,320},
      {1005,296},{975,277},{942,261},{903,244},{862,226},{819,210},{804,185},
      {777,166},{736,148},{706,137},{666,144},{637,163},{604,203},{559,205},
      {535,185},{508,187},{485,202},{451,213},{429,236}});
    // Western beach hut, access bridge and piles; separate foreground rocks.
    p.drawPolygon(QPolygonF{{354,446},{412,448},{444,466},{508,455},{515,479},{435,496},{424,509},{372,509},{357,488}});
    const QPolygonF rocks[] = {
      {{0,296},{16,293},{38,324},{43,348},{25,354},{0,346}},
      {{48,340},{66,326},{90,339},{98,325},{119,328},{131,351},{158,329},{178,337},{181,351},{156,358},{68,357}},
      {{288,318},{306,294},{316,296},{333,318},{319,331}},
      {{336,302},{352,288},{368,299},{358,308}},
      {{265,372},{276,358},{291,377},{285,383}},
      {{418,416},{433,398},{448,416},{446,424}},
      {{278,499},{288,487},{303,498},{318,496},{335,510},{308,518},{279,514}},
      {{589,613},{602,601},{613,615},{633,598},{646,594},{663,615},{651,632},{618,636}},
      {{644,653},{660,649},{673,673},{698,660},{711,677},{702,687},{673,687},{655,676}},
      {{704,566},{731,537},{750,547},{771,565},{791,548},{816,577},{835,608},{858,633},{843,650},{818,650},{796,669},{769,657},{746,665},{718,650},{690,650},{672,635},{682,609}},
      {{809,666},{825,657},{840,674},{872,629},{888,630},{914,665},{945,668},{967,706},{949,719},{921,716},{909,704},{883,719},{861,705},{839,705}},
      {{978,708},{996,698},{1019,723},{1011,735},{981,730}},
      {{1328,391},{1345,377},{1360,399},{1351,411}},
      {{1372,376},{1390,349},{1408,377},{1417,411},{1400,421},{1376,408}},
      {{1426,409},{1448,378},{1461,386},{1481,413},{1487,427},{1463,433},{1434,426}},
      {{1330,486},{1337,472},{1363,485},{1368,499},{1345,501}},
      {{1370,470},{1381,445},{1393,456},{1407,484},{1394,494},{1370,489}},
      {{1411,484},{1428,464},{1444,489},{1434,505},{1408,499}},
      {{1462,497},{1483,477},{1494,484},{1510,503},{1497,510},{1470,510}}
    };
    for(const auto &r:rocks) p.drawPolygon(r);
    // Include the full harbor decking AND its vertical piles. The top-view
    // shoreline alone does not cover the lower, perspective-projected supports.
    p.drawPolygon(QPolygonF{{930,542},{1104,539},{1300,561},{1453,599},
        {1490,633},{1480,694},{1356,677},{1218,649},{1141,655},
        {1062,672},{1002,653},{947,633}});
    p.drawPolygon(QPolygonF{{1208,341},{1207,259},{1211,234},{1223,224},{1233,237},{1237,261},{1236,342}});
    // These crowns project beyond the upper eastern shoreline.
    for (const QRectF &r : {QRectF(1059,290,47,45),QRectF(1094,296,50,48),
                           QRectF(1111,268,66,60),QRectF(1167,272,39,45),
                           QRectF(1217,290,53,47),QRectF(1251,285,64,64)})
        p.drawEllipse(r);
    p.end();
    // Crown bounds exclude trunks and buildings. G stores distance from the
    // leaf attachment, so displacement grows toward the free tips.
    const QRectF crowns[] = {{501,167,41,38},{553,162,47,40},{565,154,34,30},
      {758,157,47,40},{812,198,37,34},{415,211,37,32},{451,202,31,32},
      {425,236,59,39},{492,231,56,46},{521,236,70,48},{650,243,54,39},
      {718,293,47,46},{765,267,42,43},{544,321,57,48},{491,326,40,35},
      {569,385,49,39},{510,393,59,48},{676,336,46,38},{677,446,48,42},
      {741,427,61,62},{945,391,41,43},{910,404,49,36},{999,353,45,41},
      {1061,292,43,38},{1115,272,58,52},{1097,300,44,41},{1170,276,33,36},
      {1257,290,51,51},{1221,296,46,37},{1300,352,36,41},
      {1137,454,51,45},{926,500,38,38},{954,497,48,42},{828,490,50,49},
      {1182,467,54,44},{1270,432,35,36}};
    QImage mask(day.size(),QImage::Format_RGB32);
    for(int y=0;y<day.height();++y) for(int x=0;x<day.width();++x) {
        const QColor d=day.pixelColor(x,y), n=night.pixelColor(x,y);
        int sea=qRed(water.pixel(x,y));
        // Feather only inward; a four-pixel guard protects rock silhouettes.
        if(sea) for(int dy=-3;dy<=3;++dy) for(int dx=-3;dx<=3;++dx)
            if(x+dx>=0 && x+dx<day.width() && y+dy>=0 && y+dy<day.height() && !qRed(water.pixel(x+dx,y+dy)))
                sea=std::min(sea,int(std::hypot(dx,dy)*50));
        double leaf=0;
        if(d.green()>d.red()*1.06 && d.green()>d.blue()*1.15)
            for(const auto &r:crowns) {
                const double a=(x-r.center().x())/(r.width()*.5), b=(y-r.center().y())/(r.height()*.5);
                if(a*a+b*b<1) leaf=std::max(leaf,(1-std::clamp((y-r.top())/r.height(),0.,1.))*std::min(1.,(1-a*a-b*b)*6));
            }
        // Stop above the upper village: its warm roofs/windows resemble lava
        // chromatically, but must never participate in downhill heat flow.
        const bool volcanic=x>607 && x<798 && y>138 && y<261;
        const double hot=volcanic ? std::clamp((n.redF()-n.greenF()-.13)*3.,0.,1.)
            *std::clamp((n.redF()-.45)*4.,0.,1.)*std::clamp((261-y)/15.,0.,1.) : 0;
        // G is reused for upper-sky clouds. A neighborhood minimum rejects
        // isolated star cores; a generous moon guard keeps its silhouette fixed.
        if(y<140 && std::hypot(x-1366.,y-109.)>55) {
            double cloud=1;
            for(int dy=-4;dy<=4;dy+=4) for(int dx=-4;dx<=4;dx+=4)
                cloud=std::min<double>(cloud,night.pixelColor(std::clamp(x+dx,0,1773),std::clamp(y+dy,0,886)).blueF());
            leaf=std::clamp((cloud-.16)*3.,0.,1.)*std::clamp((140-y)/25.,0.,1.);
        }
        mask.setPixel(x,y,qRgb(sea,int(leaf*255),int(hot*255)));
    }
    if(!mask.save("assets/art/scene-mask.png")) return 2;
    std::printf("Shared sea / crown attachment / existing lava masks: %dx%d\n",mask.width(),mask.height());
}
