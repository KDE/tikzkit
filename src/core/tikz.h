/* This file is part of the TikZKit project.
 *
 * Copyright (C) 2013-2015 Dominik Haumann <dhaumann@kde.org>
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as published
 * by the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, see
 * <http://www.gnu.org/licenses/>.
 */
#ifndef TIKZ_H
#define TIKZ_H

// TIKZCORE_EXPORT: this like exists just to copy the include files

namespace tikz {

/**
 * Available units.
 */
enum Unit {
    Point,
    Millimeter,
    Centimeter,
    Inch
};

enum TextAlignment {
    NoAlign = 0,
    AlignLeft,
    AlignCenter,
    AlignRight,
    AlignJustify
    // TODO: add all types ?
};

enum Anchor {
    NoAnchor = 0,
    Center,
    North,
    NorthEast,
    East,
    SouthEast,
    South,
    SouthWest,
    West,
    NorthWest
};

enum Shape {
    NoShape = 0,
    ShapeRectangle,
    ShapeCircle,
    ShapeDiamond,
    ShapeEllipse
};

enum PenStyle {
    SolidLine = 0,
    DottedLine,
    DenselyDottedLine,
    LooselyDottedLine,
    DashedLine,
    DenselyDashedLine,
    LooselyDashedLine,
    DashDottedLine,
    DenselyDashDottedLine,
    LooselyDashDottedLine,
    DashDotDottedLine,
    DenselyDashDotDottedLine,
    LooselyDashDotDottedLine
};

enum LineCap {
    CapUnset = 0,
    RoundCap,
    RectCap,
    ButtCap     // TikZ default
};

enum LineJoin {
    JoinUnset = 0,
    RoundJoin,
    BevelJoin,
    MiterJoin    // TikZ default
};

enum Arrow {
    NoArrow = 0,
    /**
     * Default TikZ arrows.
     */
    ToArrow,
    ReversedToArrow,
    StealthArrow,
    ReversedStealthArrow,
    LatexArrow,
    ReversedLatexArrow,
    PipeArrow,
    /**
     * tikzlibrary: arrows
     */
    StealthTickArrow,
    ReversedStealthTickArrow,
    ArrowCount
};

}

#endif // TIKZ_H

// kate: indent-width 4; replace-tabs on;
