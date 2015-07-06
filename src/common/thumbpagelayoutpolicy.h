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

#ifndef THUMBPAGELAYOUTPOLICY_H
#define THUMBPAGELAYOUTPOLICY_H

#include <QObject>
#include <MGridLayoutPolicy>

#include <common_export.h>

/*!
 * \class ThumbPageLayoutPolicy
 * \brief A MGridLayoutPolicy class with signal and slot
 * The #ThumbPageLayoutPolicy adds signal and slot to a MGridLayoutPolicy class.
 */

class COMMON_EXPORT ThumbPageLayoutPolicy: public QObject, public MGridLayoutPolicy
{
    Q_OBJECT

signals:
    /*!
    * \brief The signal is emited when policy comes active
    */
    void policyActivated();

public:
    ThumbPageLayoutPolicy(MLayout *layout, int thumbsPerRow) : MGridLayoutPolicy(layout), thumbsPerRow(thumbsPerRow) {};

    /*!
    * \brief Gets thumbs per row.
    * \returns thumbs per row
    */
    int getThumbsPerRow() {
        return thumbsPerRow;
    };

public slots:
    /*!
     * \brief A slot for activate a policy.
     * The signal #ThumbPageLayoutPolicy::policyActivated is sent when activate is called.
     */
    void activate() {
        MGridLayoutPolicy::activate();
        emit policyActivated();
    };

private:
    int thumbsPerRow;

};

#endif //end of THUMBPAGELAYOUTPOLICY_H
