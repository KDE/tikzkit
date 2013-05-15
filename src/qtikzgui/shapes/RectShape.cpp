#include "RectShape.h"

#include <cmath>

class RectShapePrivate
{
    public:
};

RectShape::RectShape(TikzNode * node)
    : AbstractShape(node)
    , d(new RectShapePrivate())
{
}

RectShape::~RectShape()
{
    delete d;
}

tikz::Shape RectShape::type() const
{
    return tikz::ShapeRectangle;
}

QPainterPath RectShape::shape() const
{
    QPainterPath path;
    path.addRect(QRectF(-0.5, -0.5, 1.0, 1.0));
    return path;
}

QVector<tikz::Anchor> RectShape::supportedAnchors() const
{
    // by default, just return NoAnchor
    QVector<tikz::Anchor> anchors;
    anchors << tikz::NoAnchor
            << tikz::Center
            << tikz::North
            << tikz::NorthEast
            << tikz::East
            << tikz::SouthEast
            << tikz::South
            << tikz::SouthWest
            << tikz::West
            << tikz::NorthWest;
    return anchors;
}

QPointF RectShape::anchorPos(tikz::Anchor anchor) const
{
    const qreal radius = 0.5; // TODO: set size correct
    switch (anchor) {
        case tikz::NoAnchor :
        case tikz::Center   : return QPointF(0, 0);
        case tikz::North    : return QPointF(0, radius);
        case tikz::NorthEast: return QPointF(radius, radius);
        case tikz::East     : return QPointF(radius, 0);
        case tikz::SouthEast: return QPointF(radius, -radius);
        case tikz::South    : return QPointF(0, -radius);
        case tikz::SouthWest: return QPointF(-radius, -radius);
        case tikz::West     : return QPointF(-radius, 0);
        case tikz::NorthWest: return QPointF(-radius, radius);
    }

    return QPointF(0, 0);
}

QPointF RectShape::contactPoint(tikz::Anchor anchor, qreal rad) const
{
    if (anchor != tikz::NoAnchor) {
        return anchorPos(anchor);
    }

    // TODO: set to correct size
    const qreal dx = 0.5;
    const qreal dy = 0.5;

    qreal x = dx * std::cos(rad);
    qreal y = dy * std::sin(rad);
    if (!qFuzzyCompare(x, 0.0) && !qFuzzyCompare(y, 0.0)) {
        if (fabs(y) != dy) {
            // normalize to y
            x = (x < 0 ? -1 : 1) * fabs(dy * x / y);
            y = (y < 0 ? -1 : 1) * dy;
        }
        if (fabs(x) > dx) {
            // normalize to x
            y = (y < 0 ? -1 : 1) * fabs(dx * y / x);
            x = (x < 0 ? -1 : 1) * dx;
        }
    }
    return QPointF(x, y);
}

// kate: indent-width 4; replace-tabs on;
