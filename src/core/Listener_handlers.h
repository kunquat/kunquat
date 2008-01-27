

#ifndef K_LISTENER_HANDLERS_H
#define K_LISTENER_HANDLERS_H


#include "lo/lo.h"


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
//lo_method_handler Listener_version;


/**
 * Quits Kunquat.
 */
int Listener_quit(const char* path,
		const char* types,
		lo_arg** argv,
		int argc,
		lo_message msg,
		void* user_data);


int Listener_fallback(const char* path,
		const char* types,
		lo_arg** argv,
		int argc,
		lo_message msg,
		void* user_data);


#endif // K_LISTENER_HANDLERS_H


