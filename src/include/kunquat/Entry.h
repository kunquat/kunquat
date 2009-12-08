

/*
 * Copyright 2009 Tomi Jylhä-Ollila
 *
 * This file is part of Kunquat.
 *
 * Kunquat is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Kunquat is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Kunquat.  If not, see <http://www.gnu.org/licenses/>.
 */


#ifndef KQT_ENTRY_H
#define KQT_ENTRY_H


#ifdef __cplusplus
extern "C" {
#endif


/**
 * \defgroup Entry Accessing data inside a Kunquat Handle
 * \{
 *
 * \brief
 * This module describes Kunquat Entry, the object for accessing
 * individual elements of data inside a Kunquat Handle.
 */
typedef struct kqt_Entry kqt_Entry;


/**
 * Gets the data of the Kunquat Entry.
 *
 * \param entry   The Kunquat Entry -- should not be \c NULL.
 *
 * \return   The data.
 */
char* kqt_Entry_get_data(kqt_Entry* entry);


/**
 * Returns the size of the data in the Kunquat Entry in bytes.
 *
 * \param entry   The Kunquat Entry -- should not be \c NULL.
 *
 * \return   The size of the data.
 */
long kqt_Entry_get_size(kqt_Entry* entry);


/**
 * Sets the data in the Kunquat Entry.
 *
 * The function discards all the data previously contained in the Entry.
 *
 * \param entry   The Kunquat Entry -- should not be \c NULL.
 * \param data    The data to be set -- should not be \c NULL.
 * \param size    The length of the data in bytes -- should not be negative.
 *
 * \return   \c 1 if successful, otherwise \c 0.
 */
int kqt_Entry_set_data(kqt_Entry* entry, char* data, long size);


/**
 * Closes the Kunquat Entry.
 *
 * \param entry   The Kunquat Entry -- should not be \c NULL.
 *
 * \return   \c 1 if successful, otherwise \c 0.
 */
int kqt_Entry_close(kqt_Entry* entry);


/* \} */


#ifdef __cplusplus
}
#endif


#endif // KQT_ENTRY_H


