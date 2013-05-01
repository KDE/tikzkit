#include "PaintHelper.h"
#include "Style.h"
#include "tikz.h"

class PaintHelperPrivate
{
    public:
        QPainter* painter;
        tikz::Style* style;

    //
    // private helper functions
    //
    public:
        qreal lineWidth() const
        {
//             qreal oneMilliMeter = painter->device()->physicalDpiX() / 25.4;
            //     qDebug() << painter->device()->physicalDpiX() << oneMilliMeter;
//             p.setWidthF(p.widthF() * oneMilliMeter / painter->device()->physicalDpiX() * 2.54);
            return 0.1;
        }
};

PaintHelper::PaintHelper(QPainter & painter, tikz::Style & style)
    : d(new PaintHelperPrivate())
{
    d->painter = &painter;
    d->style = &style;
}

PaintHelper::~PaintHelper()
{
    delete d;
}

qreal PaintHelper::lineWidth() const
{
    // 1pt =  0.3527 mm, see http://en.wikipedia.org/wiki/Point_(typography)
    const qreal mm = 0.3527;
    qreal pt = 0.0;
    switch (d->style->lineWidth()) {
        case tikz::WidthUnset: pt = 0.0; break;
        case tikz::UltraThin : pt = 0.1; break; // 0.03527 mm
        case tikz::VeryThin  : pt = 0.2; break; // 0.07054 mm
        case tikz::Thin      : pt = 0.4; break; // 0.14108 mm
        case tikz::SemiThick : pt = 0.6; break; // 0.21162 mm
        case tikz::Thick     : pt = 0.8; break; // 0.28216 mm
        case tikz::VeryThick : pt = 1.2; break; // 0.42324 mm
        case tikz::UltraThick: pt = 1.6; break; // 0.56432 mm
        default: break;
    }
    return pt * mm * d->lineWidth();
}

Qt::PenStyle PaintHelper::penStyle() const
{
    switch (d->style->lineWidth()) {
        case tikz::PenUnset: return Qt::NoPen;
        case tikz::NoPen: return Qt::NoPen;
        case tikz::SolidLine: return Qt::SolidLine;
        case tikz::DottedLine: return Qt::DotLine;
        case tikz::DenselyDottedLine: return Qt::DotLine; // no Qt style
        case tikz::LooselyDottedLine: return Qt::DotLine; // no Qt style
        case tikz::DashedLine: return Qt::DashLine;
        case tikz::DenselyDashedLine: return Qt::DashLine; // no Qt style
        case tikz::LooselyDashedLine: return Qt::DashLine; // no Qt style
        case tikz::DashDottedLine: return Qt::DashDotLine;
        case tikz::DenselyDashDottedLine: return Qt::DashDotLine; // no Qt style
        case tikz::LooselyDashDottedLine: return Qt::DashDotLine; // no Qt style
        case tikz::DashDotDottedLine: return Qt::DashDotDotLine;
        case tikz::DenselyDashDotDottedLine: return Qt::DashDotDotLine; // no Qt style
        case tikz::LooselyDashDotDottedLine: return Qt::DashDotDotLine; // no Qt style
        default: break;
    }
    return Qt::NoPen;
}

QPen PaintHelper::pen() const
{
    // invalid color -> NoPen
    QColor c = d->style->drawColor();
    if (!c.isValid()) {
        return Qt::NoPen;
    }

    // NoPen requested?
    Qt::PenStyle ps = penStyle();
    if (ps == Qt::NoPen) {
        return Qt::NoPen;
    }

    // construct valid pen
    QPen pen(c);
    pen.setWidthF(lineWidth());

    return pen;
}

//     rgb
//     red,1,0,0/0,1,1/0,1,1,0/.3;%
//     green,0,1,0/.33333,1,1/1,0,1,0/.59;%
//     blue,0,0,1/.66667,1,1/1,1,0,0/.11;%
//     brown,.75,.5,.25/.083333,.66667,.75/0,.25,.5,.25/.5475;%
//     lime,.75,1,0/.20833,1,1/.25,0,1,0/.815;%
//     orange,1,.5,0/.083333,1,1/0,.5,1,0/.595;%
//     pink,1,.75,.75/0,.25,1/0,.25,.25,0/.825;%
//     purple,.75,0,.25/.94444,1,.75/0,.75,.5,.25/.2525;%
//     teal,0,.5,.5/.5,1,.5/.5,0,0,.5/.35;%
//     violet,.5,0,.5/.83333,1,.5/0,.5,0,.5/.205}%
//     \definecolorset{cmyk/rgb/hsb/gray}{}{}%
//     cyan,1,0,0,0/0,1,1/.5,1,1/.7;%
//     magenta,0,1,0,0/1,0,1/.83333,1,1/.41;%
//     yellow,0,0,1,0/1,1,0/.16667,1,1/.89;%
//     olive,0,0,1,.5/.5,.5,0/.16667,1,.5/.39}
//     \definecolorset{gray/rgb/hsb/cmyk}{}{}%
//     black,0/0,0,0/0,0,0/0,0,0,1;%
//     darkgray,.25/.25,.25,.25/0,0,.25/0,0,0,.75;%
//     gray,.5/.5,.5,.5/0,0,.5/0,0,0,.5;%
//     lightgray,.75/.75,.75,.75/0,0,.75/0,0,0,.25;%
//     white,1/1,1,1/0,0,1/0,0,0,0


// kate: indent-width 4; replace-tabs on;