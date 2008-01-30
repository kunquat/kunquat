

#ifndef K_DRIVER_JACK_H
#define K_DRIVER_JACK_H


#include <stdbool.h>
#include <stdint.h>

#include <Playlist.h>


/**
 * Initialise a JACK driver.
 *
 * \param playlist   The Playlist -- must not be \c NULL.
 * \param freq       A location where the mixing frequency is stored -- must
 *                   not be \c NULL.
 *
 * \return   \c true if initialisation succeeded, otherwise \c false.
 */
bool Driver_jack_init(Playlist* playlist, uint32_t* freq);


#endif // K_PROCESS_JACK_H


