/* This file is part of the TikZKit project.
 *
 * Copyright (C) 2015 Dominik Haumann <dhaumann@kde.org>
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

#include "ConfigInterface.h"

namespace tikz {
namespace core {

ConfigInterface::ConfigInterface(QObject * parent)
    : QObject(parent)
{
}

ConfigInterface::~ConfigInterface()
{
}

void ConfigInterface::beginConfig()
{
    Q_ASSERT(m_refCounter >= 0);
    ++m_refCounter;
}

void ConfigInterface::endConfig()
{
    Q_ASSERT(m_refCounter > 0);

    --m_refCounter;
    if (m_refCounter == 0) {
        emit changed();
    }
}

bool ConfigInterface::configActive() const
{
    return m_refCounter > 0;
}

void ConfigInterface::emitChangedIfNeeded()
{
    if (! configActive()) {
        emit changed();
    }
}

}
}

// kate: indent-width 4; replace-tabs on;
