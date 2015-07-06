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
#ifndef ZOOMLEVEL_H
#define ZOOMLEVEL_H

#include <QtGlobal>
#include <QMetaType>
#include <common_export.h>
/*!
 * \class ZoomLevel
 * \brief The class provides zooming mode and factor.
 */

class COMMON_EXPORT ZoomLevel
{

public:
    /*!
     * \brief Defines the mode of the zooming.
     */
    enum Mode {
        //! Fit page height to screen
        FitToHeight,
        //! Fit page width to screen
        FitToWidth,
        //! Fit page width and height to screen
        FitToPage,
        //! Change zooming relativey, that is zoom out or zoom in
        Relative,
        //! The zoom factor is used
        FactorMode
    };
    enum Type {
        //Zoom Out
        ZoomOut,
        //Zoom In
        ZoomIn,
        //Invalid
        Invalid
    };

    /*!
     * \brief Constructs a new object of given mode and factor.
     * \param mode          Defines if zoom is FitToWidth/FitToHeight or custom zoom. Default is custom zoom.
     * \param factor        Defines the custom zoom factor. Default is 1.0 which corresponds to 100%.
     * \param factor        Defines if zoom is user defined or not.
     * \param userDefined   Defines if zoom is user defined or action defined
     */
    ZoomLevel(Mode mode=FactorMode, qreal factor=1.0, bool userDefined=true);
    virtual ~ZoomLevel();

    /*!
     * \brief Set factor. Mode is set to FactorMode.
     * \param newfactor Defines the custom zoom factor (0.25 <-> 4.0).
     */
    void setFactor(qreal newfactor);

    /*!
     * \brief Set relative factor. Mode is set to Relative.
     * \param relativeFactor Define change on zoom level, for exampe use 0.9 to decrease zoom with 10%.
     */
    void setRelativeFactor(qreal relativeFactor);

    /*!
     * \brief Get factor.
     * \param value     The value of factor. The meaning of value is depending of the mode.
     * \return Returns false if factor value is not be defined.
     */
    bool getFactor(qreal & value) const;

    /*!
     * \brief Set Mode. Factor will be undefined.
     * \param newtype the new zoom mode.
     */
    void setMode(ZoomLevel::Mode newtype);

    /*!
     * \brief Get Mode.
     * \return #Mode
     */
    ZoomLevel::Mode getMode() const;

    /*!
     * \brief Set zoomType.
     * \param newtype the new zoom type.
     */
    void setZoomType(ZoomLevel::Type type);

    /*!
     * \brief Get zoomType.
     * \return #zoomType
     */
    ZoomLevel::Type getZoomType() const;

    /*!
    * \brief Checks if zoom is depending on view size.
    * @return true if zoom is depending to view size
    */
    bool isFitTo() const;

    /*!
     * \brief Checks if zoom is user defined.
     * @return true if user defined
     */
    bool isUserDefined() const;

    /*!
     * \brief Checks if zoom is user defined.
     * \param value true if user defined
     */
    void setUserDefined(bool value);

    /*!
     * \brief Equality operator.
     *
     * This operator checks if zomms leves are the same.
     * Note that Relative level always returns false as
     * real zoom level depends on how many Relative zooming
     * was done.
     * @param other The item to compare with this one.
     *
     * @return true if same levels.
     */
    bool operator ==(const ZoomLevel &other) const;

    /*!
     * \brief Not Equality operator.
     *
     * This operator checks if zoom levels are different.
     * Note that Relative level always returns true as
     * real zoom level depends on how many Relative zooming
     * was done.
     * @param other The item to compare with this one.
     *
     * @return true if same levels.
     */
    bool operator !=(const ZoomLevel &other) const;

    /*!
     * \brief Equality operator
     */
    ZoomLevel &operator=(const ZoomLevel &other);

private:
    Mode  mode;
    Type  zoomType;
    qreal factor;
    bool  userDefined;
};

Q_DECLARE_METATYPE(ZoomLevel)

#endif // ZOOMLEVEL_H

