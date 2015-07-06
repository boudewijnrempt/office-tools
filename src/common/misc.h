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

#ifndef MISC_H
#define MISC_H

class MPannableViewport;

class MWidget;

#include <common_export.h>

namespace Misc
{
/*!
 * \brief Translates point into relative point in given area.
 * \param point is the point to be translated
 * \param size is area where to be translated
 * \returns a relative point, between QPointF(0.0, 0.0) and QPointF(1.0, 1.0)
 */
QPointF  getRelativePoint(const QPointF &point, const QSizeF &size);

/*!
 * \brief Translates size into relative size in given area.
 * \param partsize is the area to be translated
 * \param size is area where to be translated
 * \returns a relative area, between QSizeF(0.0, 0.0) and QSizeF(1.0, 1.0)
 */
QSizeF getRelativeSize(const QSizeF &partsize, const QSizeF &size);

/*!
 * \brief Translates area into relative area in given area.
 * \param rect is the area to be translated
 * \param size is area where to be translated
 * \returns a relative area, between QRectf(QPoint(0.0, 0.0), QSizeF(0.0, 0.0)) and
 * QRectf(QPoint(1.0, 1.0), QSizeF(1.0, 1.0))
 */
QRectF getRelativeRect(const QRectF &rect, const QSizeF &size);

/*!
 * \brief Translates relative point into point in given area.
 * \param rpoint is the relative point to be translated
 * \param size is area where to be translated
 * \returns a point in given area
 */
QPointF translateRelativePoint(const QPointF &rpoint, const QSizeF &size);

/*!
 * \brief Translates relative size into size in given area.
 * \param rsize is the relative area to be translated
 * \param size is area where to be translated
 * \returns an area in given area
 */
QSizeF translateRelativeSize(const QSizeF &rsize, const QSizeF &size);

/*!
 * \brief Translates relative area into area in given area.
 * \param rect is the relative area to be translated
 * \param size is area where to be translated
 * \returns an rect inside QRectf(QPoint(0,0), size)
 */
QRectF translateRelativeRect(const QRectF &rect, const QSizeF &size);


/*!
 * \brief Calculates maximum size that fits the given size.
 * \param size is the size to be transformed
 * \param maxSize is area where new size must fit into
 * \returns new size that fits into given size and has same aspect ration
 * then orginal size hade.
 */
QSizeF getMaxSizeInGivenSize(const QSizeF &size, const QSizeF &maxSize);

/*!
 * \brief A static function that creates new MWidget where given
 * MWidget is centralized horizontally.
 * \param child the MWidget to be centralized.
 */
MWidget * createHorizontalWidget(MWidget * child);

QString getFileTypeFromFile(QString filePath);

QString getFileTypeFromMime(QString mimeType, QString extension = QString());

};


#endif //end of MISC_H
