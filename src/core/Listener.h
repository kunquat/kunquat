

#ifndef K_LISTENER_H
#define K_LISTENER_H


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
} Listener;


#endif // K_LISTENER_H


