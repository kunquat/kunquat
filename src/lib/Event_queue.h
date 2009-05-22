

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


#ifndef K_EVENT_QUEUE_H
#define K_EVENT_QUEUE_H


#include <stdint.h>
#include <stdbool.h>

#include <Event.h>


/**
 * Event queue is a buffer for upcoming Events during playback.
 */
typedef struct Event_queue Event_queue;


/**
 * Creates a new Event queue.
 *
 * \param size   The size of the queue -- must be > \c 0.
 *
 * \return   The new Event queue if successful, or \c NULL if memory
 *           allocation failed.
 */
Event_queue* new_Event_queue(int size);


/**
 * Inserts an Event into the Event queue.
 *
 * \param q       The Event queue -- must not be \c NULL.
 * \param event   The Event -- must not be \c NULL.
 * \param pos     The starting frame of the Event.
 *
 * \return   \c true if successful, or \c false if the queue is full.
 */
bool Event_queue_ins(Event_queue* q, Event* event, uint32_t pos);


/**
 * Gets an Event from the Event queue and removes the Event.
 *
 * \param q      The Event queue -- must not be \c NULL.
 * \param dest   A pointer to the Event object reference -- must not be
 *               \c NULL. Nothing will be written to the address if the queue
 *               is empty.
 * \param pos    The location where the starting frame is stored -- must not
 *               be \c NULL. Nothing will be written to the address if the
 *               queue is empty.
 *
 * \return   \c true if an Event was retrieved, or \c false if the queue is
 *           empty.
 */
bool Event_queue_get(Event_queue* q, Event** dest, uint32_t* pos);


/**
 * Gets any Event from the Event queue without removing the Event.
 *
 * \param q       The Event queue -- must not be \c NULL.
 * \param index   The index of the Event -- must be >= \c 0. Value \c 0 means
 *                the next Event to be removed from the queue.
 * \param dest    A pointer to the Event object reference -- must not be
 *                \c NULL.
 * \param pos     The location where the starting frame is stored -- must not
 *                be \c NULL.
 *
 * \return   \c true if an Event was retrieved, or \c false if no Event exists
 *           at \a index.
 */
bool Event_queue_peek(Event_queue* q, int index, Event** dest, uint32_t* pos);


/**
 * Clears an Event queue from Events.
 *
 * \param q   The Event queue -- must not be \c NULL.
 */
void Event_queue_clear(Event_queue* q);


/**
 * Resizes the Event queue.
 *
 * The queue will become empty as a result.
 *
 * \param q          The Event queue -- must not be \c NULL.
 * \param new_size   The new size of the queue -- must be > \c 0.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Event_queue_resize(Event_queue* q, int new_size);


/**
 * Destroys an existing Event queue.
 *
 * \param q   The Event queue -- must not be \c NULL.
 */
void del_Event_queue(Event_queue* q);


#endif // K_EVENT_QUEUE_H


