

#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "lo/lo.h"

#include "Listener.h"
#include "Listener_handlers.h"

#include <xmemory.h>


/**
 * Initialises a Listener.
 *
 * \param l   The Listener -- must not be \c NULL.
 *
 * \return   The parameter \a l if successful, or \c NULL if failed.
 *           A failure occurs if the server cannot be initialised or the
 *           server cannot be monitored using select().
 */
Listener* Listener_init(Listener* l);


/**
 * Uninitalises a Listener.
 *
 * \param l   The Listener -- must not be \c NULL.
 */
void Listener_uninit(Listener* l);


/**
 * The error handler for the server.
 *
 * \param num     The error identification.
 * \param msg     A textual description of the error.
 * \param where   The location of the error.
 */
void Listener_error(int num, const char* msg, const char* where);


int main(void)
{
	Listener* l = Listener_init(&(Listener){ .done = false });
	if (l == NULL)
	{
		exit(LISTENER_ERROR_CREATE);
	}
	do
	{
		fd_set rfds;
		FD_ZERO(&rfds);
		FD_SET(l->lo_fd, &rfds);
		int ret = select(l->lo_fd + 1, &rfds, NULL, NULL, NULL);
		if (ret == -1)
		{
			Listener_uninit(l);
			fprintf(stderr, "select() error\n");
			exit(LISTENER_ERROR_SELECT);
		}
		else if (ret > 0)
		{
			if (FD_ISSET(l->lo_fd, &rfds))
			{
				lo_server_recv_noblock(l->s, 0);
			}
		}
	} while (!l->done);
	Listener_uninit(l);
	exit(LISTENER_ERROR_NONE);
}


Listener* Listener_init(Listener* l)
{
	assert(l != NULL);
	l->done = false;
	l->s = lo_server_new("7770", Listener_error);
	if (l->s == NULL)
	{
		return NULL;
	}
	l->lo_fd = lo_server_get_socket_fd(l->s);
	if (l->lo_fd < 0)
	{
		lo_server_free(l->s);
		return NULL;
	}
	lo_server_add_method(l->s, "/kunquat/quit", "", Listener_quit, l);
	lo_server_add_method(l->s, "/kunquat/register_host", "s", Listener_register_host, l);
	lo_server_add_method(l->s, NULL, NULL, Listener_fallback, l);
	l->host = NULL;
	l->host_hostname = NULL;
	l->host_port = NULL;
	l->host_path = NULL;
	return l;
}


void Listener_uninit(Listener* l)
{
	assert(l != NULL);
	lo_server_free(l->s);
	if (l->host != NULL)
	{
		lo_address_free(l->host);
	}
	if (l->host_hostname != NULL)
	{
		free(l->host_hostname);
	}
	if (l->host_port != NULL)
	{
		free(l->host_port);
	}
	if (l->host_path != NULL)
	{
		free(l->host_path);
	}
	return;
}


void Listener_error(int num, const char* msg, const char* where)
{
	fprintf(stderr, "liblo server error %d in %s: %s\n", num, where, msg);
	return;
}


int Listener_register_host(const char* path,
		const char* types,
		lo_arg** argv,
		int argc,
		lo_message msg,
		void* user_data)
{
	assert(user_data != NULL);
	if (argc < 1)
	{
		fprintf(stderr, "Too few arguments to register_host\n");
		return 1;
	}
	Listener* l = user_data;
	if (l->host != NULL)
	{
		lo_address_free(l->host);
	}
	if (l->host_hostname != NULL)
	{
		free(l->host_hostname);
	}
	if (l->host_port != NULL)
	{
		free(l->host_port);
	}
	if (l->host_path != NULL)
	{
		free(l->host_path);
	}
	char* url = &argv[0]->s;
	l->host_hostname = lo_url_get_hostname(url);
	l->host_port = lo_url_get_port(url);
	l->host_path = lo_url_get_path(url);
	l->host = lo_address_new(l->host_hostname, l->host_port);
	if (l->host == NULL)
	{
		fprintf(stderr, "Failed to create an address object\n");
		return 1;
	}
/*	char method[] = "notify";
	char* full_path = xnalloc(char, strlen(l->host_path) + strlen(method) + 1);
	if (full_path == NULL)
	{
		fprintf(stderr, "Out of memory at %s:%d\n", __FILE__, __LINE__);
		return 1;
	}
	strcpy(full_path, l->host_path);
	strcat(full_path, method); */
	char* full_path = NULL;
	METHOD_PATH_ALLOC(full_path, l->host_path, "notify");
/*	fprintf(stderr, "send a confirmation to hostname %s, port %s, path %s\n",
			l->host_hostname,
			l->host_port,
			full_path); */
	int ret = lo_send(l->host, full_path, "s", "Hello");
	xfree(full_path);
	if (ret == -1)
	{
		fprintf(stderr, "Failed to send the confirmation message\n");
		return 1;
	}
	return 0;
}


#if 0
int Listener_version(const char* path,
		const char* types,
		lo_arg** argv,
		int argc,
		lo_message msg,
		void* user_data)
{
	assert(user_data != NULL);
}
#endif


int Listener_quit(const char* path,
		const char* types,
		lo_arg** argv,
		int argc,
		lo_message msg,
		void* user_data)
{
	assert(user_data != NULL);
	Listener* l = user_data;
	if (l->host != NULL && l->host_path != NULL)
	{
		char* full_path = NULL;
		METHOD_PATH_ALLOC(full_path, l->host_path, "notify");
		lo_send(l->host, full_path, "s", "Bye");
		xfree(full_path);
	}
	l->done = true;
	return 0;
}


int Listener_fallback(const char* path,
		const char* types,
		lo_arg** argv,
		int argc,
		lo_message msg,
		void* user_data)
{
	assert(user_data != NULL);
	Listener* l = user_data;
	if (l->host == NULL || l->host_path == NULL)
	{
		return 0;
	}
/*	char method[] = "notify";
	char* full_path = xnalloc(char, strlen(l->host_path) + strlen(method) + 1);
	if (full_path == NULL)
	{
		fprintf(stderr, "Out of memory at %s:%d\n", __FILE__, __LINE__);
		return 1;
	}
	strcpy(full_path, l->host_path);
	strcat(full_path, method);*/
	char* full_path = NULL;
	METHOD_PATH_ALLOC(full_path, l->host_path, "notify");
	int ret = lo_send(l->host, full_path, "ss", "Unrecognised command:", path);
	xfree(full_path);
	if (ret == -1)
	{
		fprintf(stderr, "Failed to send the response message\n");
		return 1;
	}
	return 0;
}


