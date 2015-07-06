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

#ifndef SEARCHRESULT_H
#define SEARCHRESULT_H

class KoPAPageBase;

class KoShape;

/*!
* \brief Struct to store search results
*
*/

typedef struct {

    /*!
    * \brief What page the text was found
    *
    */
    KoPAPageBase *page;

    /*!
    * \brief What shape contained the text
    *
    */
    KoShape *shape;

    /*!
    * \brief Starting position of the searched text within the whole text in
    * shape
    *
    */
    int startPosition;

    /*!
    * \brief Ending position of the searched text within the whole text in
    * shape
    *
    */
    int length;

} SearchResult;

#endif
