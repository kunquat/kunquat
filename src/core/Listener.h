

#ifndef K_LISTENER_H
#define K_LISTENER_H


#include <Voice_pool.h>
#include <Player.h>

#include "lo/lo.h"


#define MAX_VOICES (1024)


typedef enum
{
	LISTENER_ERROR_NONE = 0,
	LISTENER_ERROR_CREATE,
	LISTENER_ERROR_SELECT
} Listener_err;


typedef struct Listener
{
	/// Used to indicate exit.
	bool done;
	/// The OSC server.
	lo_server s;
	/// The file descriptor of the OSC server socket.
	int lo_fd;
	/// The OSC client address of the host application.
	lo_address host;
	/// The hostname (location) of the host application.
	char* host_hostname;
	/// The port of the host application.
	char* host_port;
	/// The path of the host application.
	char* host_path;

	/// The Voice pool used for mixing.
	Voice_pool* voices;
	/// Playback state information.
	Player* player;
} Listener;


#define METHOD_PATH_ALLOC(full, path, method) do\
	{\
		(full) = xnalloc(char, strlen(path) + strlen(method) + 1);\
		if ((full) == NULL)\
		{\
			fprintf(stderr, "Out of memory at %s:%d\n", __FILE__, __LINE__);\
			return 1;\
		}\
		strcpy((full), (path));\
		strcat((full), (method));\
	} while (0)


/**
 * Registers a host application that uses Kunquat.
 * 
 * \param s   The OSC URL of the host application with base path.
 *
 * The Listener sends a confirmation message to the host on success.
 */
int Listener_register_host(const char* path,
		const char* types,
		lo_arg** argv,
		int argc,
		lo_message msg,
		void* user_data);


/**
 * Gets the Kunquat version.
 */
int Listener_version(const char* path,
		const char* types,
		lo_arg** argv,
		int argc,
		lo_message msg,
		void* user_data);


/**
 * Quits Kunquat.
 */
int Listener_quit(const char* path,
		const char* types,
		lo_arg** argv,
		int argc,
		lo_message msg,
		void* user_data);


/**
 * A fallback method. A host, if registered, will be sent a notification.
 */
int Listener_fallback(const char* path,
		const char* types,
		lo_arg** argv,
		int argc,
		lo_message msg,
		void* user_data);


/**
 * Set the number of Voices.
 *
 * \param i   The number of Voices. This should be > \c 0 and
 *            <= \c MAX_VOICES.
 */
int Listener_set_voices(const char* path,
		const char* types,
		lo_arg** argv,
		int argc,
		lo_message msg,
		void* user_data);


#endif // K_LISTENER_H


