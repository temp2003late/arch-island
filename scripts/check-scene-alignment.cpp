// Native-pixel edge correlation across the supplied day/night pair.
// c++ -O2 -fPIC -std=c++20 scripts/check-scene-alignment.cpp $(pkg-config --cflags --libs Qt6Gui) -o /tmp/island-alignment
#include <QImage>
#include <QRect>
#include <cmath>
#include <cstdio>
#include <vector>

std::vector<float> edges(const QImage &input) {
    const auto image=input.convertToFormat(QImage::Format_RGB32);
    const int width=image.width(), height=image.height();
    std::vector<float> result(width*height);
    for(int y=1;y<height-1;++y) {
        const auto *row=reinterpret_cast<const QRgb *>(image.constScanLine(y));
        const auto *above=reinterpret_cast<const QRgb *>(image.constScanLine(y-1));
        const auto *below=reinterpret_cast<const QRgb *>(image.constScanLine(y+1));
        for(int x=1;x<width-1;++x)
            result[y*width+x]=std::hypot(qGray(row[x+1])-qGray(row[x-1]),
                                        qGray(below[x])-qGray(above[x]));
    }
    return result;
}
int main() {
    const QImage day("assets/art/island.png"), night("assets/art/island_night.png");
    if(day.size()!=QSize(3840,2160) || night.size()!=day.size()) {
        std::fprintf(stderr,"Expected matching 3840x2160 panoramas\n");
        return 1;
    }
    const auto a=edges(day), b=edges(night);
    const int width=day.width();
    struct Patch { const char *name; QRect rect; };
    const Patch patches[]={
        {"residence",{950,180,370,180}}, {"lighthouse",{3290,350,100,270}},
        {"church",{1900,500,240,320}}, {"bridge",{1350,1000,330,160}},
        {"crane",{2670,850,310,320}}, {"foreground rock",{3190,1600,300,170}}
    };
    for(const auto &patch:patches) {
        double best=-1, zero=0;
        int bestX=0, bestY=0;
        for(int dy=-12;dy<=12;++dy) for(int dx=-12;dx<=12;++dx) {
            double ab=0, aa=0, bb=0;
            for(int y=patch.rect.top();y<patch.rect.bottom();y+=2)
                for(int x=patch.rect.left();x<patch.rect.right();x+=2) {
                    const double av=a[y*width+x], bv=b[(y+dy)*width+x+dx];
                    ab+=av*bv; aa+=av*av; bb+=bv*bv;
                }
            const double correlation=aa*bb>0 ? ab/std::sqrt(aa*bb) : 0;
            if(dx==0 && dy==0) zero=correlation;
            if(correlation>best) { best=correlation; bestX=dx; bestY=dy; }
        }
        std::printf("%s: offset=(%d,%d) best=%.3f zero=%.3f\n",
                    patch.name,bestX,bestY,best,zero);
    }
}
