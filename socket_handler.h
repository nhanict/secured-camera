#ifndef __SOCKET_HANDLER_H_
#define __SOCKET_HANDLER_H_

#define MAX_SOCKET_MSG_LEN					4096  /* io buffer length */
#define MAXPENDING 							10    /* Max connection requests */

#include <stdlib.h>
#include <cstdio>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/in.h>
#include <pthread.h>

typedef enum {
	SECURE_RUNNING = 0,	// Security thread is running.
	SECURE_STOPPED		// Security thread is stopped.
} SStatus;

typedef enum {
	SOCK_THREAD_FAILED = -1,
	SOCK_CREATE_FAILED,
	SOCK_CLIENT_ACCEPT_FAILED,
	SOCK_BIND_FAILED,
	SOCK_LISTEN_FAILED,
	SOCK_OK = 0
} ServerSocketStatus;

typedef enum {
	C_SOCK_CLOSED = -1,
	C_SOCK_OK = 0
} ClientSocketStatus;

#define SERVER_IP "192.168.20.11"
#define SERVER_PORT 8888

class SocketHandler {
public:
	SocketHandler();
	virtual ~SocketHandler();
	void 					initiate();
	ServerSocketStatus 	start(const char *ip, unsigned short port);
	int 					stop();
	void 					responseMsg(int client_sock, void *data, int len);
private:
	static void			*static_th_main_execution(void *This);
	static void			*static_th_client_handle(void *This);
	void 					*th_main_execution(void *);
	void 					*th_client_handle(void *);
	virtual void 			onClientSocketConnected(int sock_num) = 0;
	virtual void 			onReceivedMessage(int sock_num, void *msg, int len, ClientSocketStatus stt = C_SOCK_OK) = 0;
	virtual void 			onErrorReport(ServerSocketStatus stt) = 0;

	int						server_sock;
	struct sockaddr_in 	server;
	int 					socket_control_thread_des;
	pthread_t 				socket_control_thread;
	int						crrt_client_sock;
	unsigned short		port;
};

#endif // __SOCKET_HANDLER_H_
