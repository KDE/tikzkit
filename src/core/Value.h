/* This file is part of the TikZKit project.
 *
 * Copyright (C) 2013-2014 Dominik Haumann <dhaumann@kde.org>
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
#ifndef TIKZ_VALUE_H
#define TIKZ_VALUE_H

#include "tikz_export.h"
#include "tikz.h"

#include <cmath>
#include <QDebug>

namespace tikz
{

namespace internal {
    // see: http://tex.stackexchange.com/questions/8260
    static constexpr qreal units[4][4] = {
        // pt          , mm      , cm     , in

        // conversion from pt
        {1.0           , 2540.0/7227.0, 254.0/7227.0, 100.0/7227.0},
        // conversion from mm
        {7227.0/2540.0 , 1.0          , 0.1         , 5.0/127.0},
        // conversion from cm
        {7227.0/254.0  , 10.0         , 1.0         , 50.0/127.0},
        // conversion from in
        {7227.0/100.0  , 127.0/5.0    , 127.0/50.0  , 1.0}
    };
}

/**
 * Units.
 */
enum Unit {
    Point,      // =  1.00pt = 0.3515mm
    Millimeter, // =  2.84pt = 0.99826mm
    Centimeter, // = 28.40pt = 9.9826mm
    Inch        // = 72.27pt = 25.402905mm
};

class TIKZCORE_EXPORT Value
{
    public:
        /**
         * Constructor with value and type.
         */
        constexpr Value(qreal value = 0.0, Unit unit = Unit::Point)
            : m_value(value)
            , m_unit(unit)
        {
        }

        /**
         * Check whether this value is a finite value, i.e. other than NaN or infinity.
         */
        inline constexpr bool isValid() const noexcept
        {
            return std::isfinite(m_value);
        }

        /**
         * Get the Unit.
         */
        inline constexpr Unit unit() const noexcept
        {
            return m_unit;
        }

        /**
         * Explicit conversion to value.
         */
        inline constexpr qreal value() const noexcept
        {
            return m_value;
        }

        /**
         * Explicit conversion to Point.
         */
        inline constexpr qreal toPoint() const noexcept
        {
            return convertTo(tikz::Unit::Point).m_value;
        }

        /**
         *
         */
        inline constexpr Value convertTo(Unit unit) const noexcept
        {
            return Value(m_value * internal::units[m_unit][unit], unit);
        }

        /**
         * Convert this number to a string.
         */
        QString toString() const;

        /**
         * Convert @p str to a Value.
         */
        static Value fromString(const QString & str);

    //
    // operators for qreal values
    //
    public:
//         /**
//          * Implicit cast to value.
//          * @warning the returned value \e always is of type Unit::Point!
//          */
//         inline constexpr operator qreal () const noexcept
//         {
//             return convertTo(Unit::Point).value();
//         }

        /**
         * Adds @p value to this Value and returns a reference to this Value.
         * @warning This operation is independent of the unit.
         */
        inline Value & operator+=(qreal value) noexcept
        {
            Q_ASSERT(isValid());
            m_value += value;
            return *this;
        }

        /**
         * Subtracts @p value from this Value and returns a reference to this Value.
         * @warning This operation is independent of the unit.
         */
        inline Value & operator-=(qreal value) noexcept
        {
            Q_ASSERT(isValid());
            m_value -= value;
            return *this;
        }

        /**
         * Multiplies this Value with @p factor and returns a reference to this Value.
         * @warning This operation is independent of the unit.
         */
        inline Value & operator*=(qreal factor) noexcept
        {
            Q_ASSERT(isValid());
            m_value *= factor;
            return *this;
        }

        /**
         * Divides this Value by @p divisor and returns a reference to this Value.
         * @warning This operation is independent of the unit.
         */
        inline Value & operator/=(qreal divisor) noexcept
        {
            Q_ASSERT(isValid());
            m_value /= divisor;
            return *this;
        }

    //
    // operators for two Value
    //
    public:
        /**
         * Plus operator for two Value%s.
         * Returns a new Value object with the same unit increased by @p value.
         * If @p value's unit is different, @p value is first converted to a
         * compatible Unit before adding the value.
         */
        inline Value operator+(const Value & value) const noexcept
        {
            Q_ASSERT(isValid());
            Q_ASSERT(value.isValid());
            return Value(m_value + value.convertTo(m_unit).value(), m_unit);
        }

        /**
         * Minus operator for two Value%s.
         * Returns a new Value object with the same unit decreased by @p value.
         * If @p value's unit is different, @p value is first converted to a
         * compatible Unit before adding the value.
         */
        inline Value operator-(const Value & value) const noexcept
        {
            Q_ASSERT(isValid());
            Q_ASSERT(value.isValid());
            return Value(m_value - value.convertTo(m_unit).value(), m_unit);
        }

        /**
         * += operator for two Value%s.
         * Returns a reference to this object increased by @p value.
         * If @p value's unit is different, @p value is first converted to a
         * compatible Unit before adding the value.
         */
        inline Value & operator+=(const Value & value)
        {
            Q_ASSERT(isValid());
            Q_ASSERT(value.isValid());
            return operator+=(value.convertTo(m_unit).value());
        }

        /**
         * -= operator for two Value%s.
         * Returns a reference to this object decreased by @p value.
         * If @p value's unit is different, @p value is first converted to a
         * compatible Unit before adding the value.
         */
        inline Value & operator-=(const Value & value)
        {
            Q_ASSERT(isValid());
            Q_ASSERT(value.isValid());
            return operator-=(value.convertTo(m_unit).value());
        }

        /**
         * QDebug support.
         */
        inline friend QDebug operator<<(QDebug s, const tikz::Value & value)
        {
            if (&value != nullptr) {
                s.nospace() << value.toString();
            } else {
                s.nospace() << "(null Value)";
            }
            return s.space();
        }

    private:
        /**
         * The value.
         */
        qreal m_value;

        /**
         * The unit of this value.
         */
        Unit m_unit;
};

/**
 * Equality operator.
 * Return @e true, if the Value @p lhs equals the Value @p rhs.
 * @note The comparison is unit-aware.
 */
inline constexpr bool operator==(const Value & lhs, const Value & rhs) noexcept
{
    return qFuzzyCompare(lhs.value(), rhs.convertTo(lhs.unit()).value());
}

/**
 * Inequality operator.
 * Return @e true, if the Value @p lhs does not equal the Value @p rhs.
 * @note The comparison is unit-aware.
 */
inline constexpr bool operator!=(const Value & lhs, const Value & rhs) noexcept
{
    return ! (lhs == rhs);
}

/**
 * Returns a Value that is formed by multiplying @p value with @p factor.
 * @warning This operation is independent of the unit.
 */
inline constexpr Value operator*(const Value & value, qreal factor) noexcept
{
    return Value(value.value() * factor, value.unit());
}

/**
 * Returns a Value that is formed by multiplying @p value with @p factor.
 * @warning This operation is independent of the unit.
 */
inline constexpr Value operator*(qreal factor, const Value & value) noexcept
{
    return value * factor;
}

/**
 * Returns a Value that is formed by dividing @p value by @p divisor.
 * @warning This operation is independent of the unit.
 */
inline Value operator/(const Value & value, qreal divisor)
{
    Q_ASSERT(divisor != 0);
    return Value(value.value() / divisor, value.unit());
}

/**
 * Returns @e true if @p value is greater than @p valInPoints.
 * @warning @valInPoints is interpreted in Points.
 */
inline constexpr bool operator>(const Value & value, qreal valInPoints) noexcept
{
    return value.toPoint() > valInPoints;
}

/**
 * Returns @e true if @p value is greater than @p valInPoints.
 * @warning @valInPoints is interpreted in Points.
 */
inline constexpr bool operator>(qreal valInPoints, const Value & value) noexcept
{
    return valInPoints > value.toPoint();
}

/**
 * Returns true if @p value in Point is greater than or equal to @p valInPoints.
 * @warning @valInPoints is interpreted in Points.
 */
inline constexpr bool operator>=(const Value & value, qreal valInPoints) noexcept
{
    return value.toPoint() >= valInPoints;
}

/**
 * Returns true if @p value in Point is less than @p valInPoints.
 * @warning @valInPoints is interpreted in Points.
 */
inline constexpr bool operator<(const Value & value, qreal valInPoints) noexcept
{
    return value.toPoint() < valInPoints;
}

/**
 * Returns true if @p value in Point is less than or equal to @p valInPoints.
 * @warning @valInPoints is interpreted in Points.
 */
inline constexpr bool operator<=(const Value & value, qreal valInPoints) noexcept
{
    return value.toPoint() <= valInPoints;
}

/**
 * Returns true if Value @p lhs is greater than Valule @p rhs.
 * @note This comparison is unit-aware.
 */
inline bool operator>(const Value & lhs, const Value & rhs) noexcept
{
    Q_ASSERT(lhs.isValid());
    Q_ASSERT(rhs.isValid());
    return lhs.value() > rhs.convertTo(lhs.unit()).value();
}

/**
 * Returns true if Value @p lhs is greater than or equal to Valule @p rhs.
 * @note This comparison is unit-aware.
 */
inline bool operator>=(const Value & lhs, const Value & rhs) noexcept
{
    Q_ASSERT(lhs.isValid());
    Q_ASSERT(rhs.isValid());
    return lhs.value() >= rhs.convertTo(lhs.unit()).value();
}

/**
 * Returns true if Value @p lhs is less than Valule @p rhs.
 * @note This comparison is unit-aware.
 */
inline bool operator<(const Value & lhs, const Value & rhs) noexcept
{
    Q_ASSERT(lhs.isValid());
    Q_ASSERT(rhs.isValid());
    return lhs.value() < rhs.convertTo(lhs.unit()).value();
}

/**
 * Returns true if Value @p lhs is less than or equal to Valule @p rhs.
 * @note This comparison is unit-aware.
 */
inline bool operator<=(const Value & lhs, const Value & rhs) noexcept
{
    Q_ASSERT(lhs.isValid());
    Q_ASSERT(rhs.isValid());
    return lhs.value() <= rhs.convertTo(lhs.unit()).value();
}

/**
 * Convert @p value from unit @p from to unit @p to.
 */
inline qreal convertTo(qreal value, Unit from, Unit to)
{
    const qreal s = internal::units[from][to];
    return value * s;
}

//
// pt to *
//
inline qreal pt2mm(qreal value)
{
    const qreal s = internal::units[Unit::Point][Unit::Millimeter];
    return value * s;
}

inline qreal pt2cm(qreal value)
{
    const qreal s = internal::units[Unit::Point][Unit::Centimeter];
    return value * s;
}

inline qreal pt2in(qreal value)
{
    const qreal s = internal::units[Unit::Point][Unit::Inch];
    return value * s;
}

//
// cm to *
//
inline qreal cm2pt(qreal value)
{
    const qreal s = internal::units[Unit::Centimeter][Unit::Point];
    return value * s;
}

inline qreal cm2mm(qreal value)
{
    const qreal s = internal::units[Unit::Centimeter][Unit::Millimeter];
    return value * s;
}

inline qreal cm2in(qreal value)
{
    const qreal s = internal::units[Unit::Centimeter][Unit::Inch];
    return value * s;
}

//
// mm to *
//
inline qreal mm2pt(qreal value)
{
    const qreal s = internal::units[Unit::Millimeter][Unit::Point];
    return value * s;
}

inline qreal mm2cm(qreal value)
{
    const qreal s = internal::units[Unit::Millimeter][Unit::Centimeter];
    return value * s;
}

inline qreal mm2in(qreal value)
{
    const qreal s = internal::units[Unit::Millimeter][Unit::Inch];
    return value * s;
}

//
// in to *
//
inline qreal in2pt(qreal value)
{
    const qreal s = internal::units[Unit::Inch][Unit::Point];
    return value * s;
}

inline qreal in2cm(qreal value)
{
    const qreal s = internal::units[Unit::Inch][Unit::Centimeter];
    return value * s;
}

inline qreal in2mm(qreal value)
{
    const qreal s = internal::units[Unit::Inch][Unit::Millimeter];
    return value * s;
}

}

namespace QTest
{
    // forward declaration of template in qtestcase.h
    template<typename T> char* toString(const T&);
    
    template<>
    TIKZCORE_EXPORT char *toString(const tikz::Value & value);
}

/**
 * Declare as movable, since the members of Value are primitive types.
 */
Q_DECLARE_TYPEINFO(tikz::Value, Q_MOVABLE_TYPE);
Q_DECLARE_METATYPE(tikz::Value)

#endif // TIKZ_VALUE_H

// kate: indent-width 4; replace-tabs on;
