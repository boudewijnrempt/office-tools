/*
 * This file is part of Meego Office UI for KOffice
 *
 * Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
 *
 * Contact: Suresh Chande suresh.chande@nokia.com
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 *
 */

#include "zoomlevel.h"
#include <QDebug>

ZoomLevel::ZoomLevel(Mode mode, qreal factor, bool userDefined)
    : mode(mode)
    , zoomType(ZoomLevel::Invalid)
    , factor(factor)
    , userDefined(userDefined)
{
}

ZoomLevel::~ZoomLevel()
{
    qDebug() << __PRETTY_FUNCTION__;
}

bool ZoomLevel::getFactor(qreal & value) const
{
    value = factor;
    return true;
}

void ZoomLevel::setFactor(qreal newfactor)
{
    mode = FactorMode;
    factor = newfactor;
}

ZoomLevel::Mode ZoomLevel::getMode() const
{
    return mode;
}

void ZoomLevel::setMode(ZoomLevel::Mode newmode)
{
    mode = newmode;
}

void ZoomLevel::setRelativeFactor(qreal relativeFactor)
{
    mode = Relative;
    factor = relativeFactor;
}

void ZoomLevel::setZoomType(ZoomLevel::Type type)
{
    zoomType = type;
}

ZoomLevel::Type ZoomLevel::getZoomType() const
{
    return zoomType;
}

bool ZoomLevel::operator ==(const ZoomLevel & other) const
{
    bool retval = false;

    if(Relative != mode && other.mode == mode) {
        retval = true;

        if(FactorMode == mode) {
            retval = (other.factor == factor);
        }
    }

    return retval;
}

ZoomLevel &ZoomLevel::operator=(const ZoomLevel &other)
{
    mode = other.mode;
    factor = other.factor;
    userDefined = other.userDefined;
    return *this;
}

bool ZoomLevel::operator !=(const ZoomLevel & other) const
{
    bool retval = true;

    if(Relative != mode && other.mode == mode) {
        retval = false;

        if(FactorMode == mode) {
            retval = (other.factor != factor);
        }
    }

    return retval;
}

bool ZoomLevel::isFitTo() const
{
    if(FitToHeight == mode ||
       FitToWidth == mode ||
       FitToPage == mode) {
        return true;
    }

    return false;
}

bool ZoomLevel::isUserDefined() const
{
    return userDefined;
}

void ZoomLevel::setUserDefined(bool value)
{
    userDefined = value;
}
