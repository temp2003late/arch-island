// Local edge correlation checks registration across different illumination.
// c++ -std=c++20 scripts/check-scene-alignment.cpp $(pkg-config --cflags --libs Qt6Gui) -o /tmp/island-alignment && /tmp/island-alignment
#include <QImage>
#include <QRect>
#include <cmath>
#include <cstdio>

double edge(const QImage &image, int x, int y) {
    const auto a=image.pixelColor(x+1,y), b=image.pixelColor(x-1,y);
    const auto c=image.pixelColor(x,y+1), d=image.pixelColor(x,y-1);
    return std::hypot(a.redF()-b.redF(),c.redF()-d.redF());
}
int main() {
    const QImage day("assets/art/island-day-wide.png");
    const QImage night("assets/art/island-night-wide.png");
    if (day.size()!=QSize(1774,887) || night.size()!=day.size()) return 1;
    // Lighthouse, roof, bridge, cargo/crane, foreground rock, volcano.
    const QRect patches[]={{1205,232,33,103},{786,352,77,42},{984,349,107,46},
                           {1279,555,93,88},{729,571,61,48},{657,143,47,89}};
    for (const auto &rect:patches) {
        double best=-1;
        int bestX=0, bestY=0;
        for (int dy=-4;dy<=4;++dy) for (int dx=-4;dx<=4;++dx) {
            double ab=0, aa=0, bb=0;
            for (int y=rect.top();y<rect.bottom();++y)
                for (int x=rect.left();x<rect.right();++x) {
                    const double a=edge(day,x,y), b=edge(night,x+dx,y+dy);
                    ab+=a*b; aa+=a*a; bb+=b*b;
                }
            const double correlation=ab/std::sqrt(aa*bb);
            if (correlation>best) { best=correlation; bestX=dx; bestY=dy; }
        }
        std::printf("patch (%d,%d) offset=(%d,%d) correlation=%.3f\n",
                    rect.x(),rect.y(),bestX,bestY,best);
    }
}
