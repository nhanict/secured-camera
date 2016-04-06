
#include "socket_handler.h"


SocketHandler::SocketHandler(){}
SocketHandler::~SocketHandler(){}

ServerSocketStatus SocketHandler::start(const char *ip, unsigned short port){
	this->port = port;
	if((socket_control_thread_des = pthread_create(&socket_control_thread, NULL, static_th_main_execution, this))){
		perror("\nError in create Socket handler thread.");
		return SOCK_THREAD_FAILED;
	}
	return SOCK_OK;
}

void SocketHandler::initiate(){
}

int SocketHandler::stop(){
	return	pthread_cancel(socket_control_thread);
}

void SocketHandler::responseMsg(int sock_client, void *data, int len){
	int ret = write(sock_client, data, len);
}

void *SocketHandler::static_th_main_execution(void *This){
	((SocketHandler *)This)->th_main_execution(&((SocketHandler *)This)->port);
	return NULL;
}

void *SocketHandler::th_main_execution(void *input){
	if((server_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		onErrorReport(SOCK_CREATE_FAILED);
	}
	memset(&server, 0, sizeof(server));
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = htonl(INADDR_ANY);
	server.sin_port = htons(*(unsigned short *)input);
	if(bind(server_sock, (sockaddr *) &server, sizeof(server))){
		onErrorReport(SOCK_BIND_FAILED);
	}
	if(listen(server_sock, MAXPENDING) < 0){
		onErrorReport(SOCK_LISTEN_FAILED);
	}
	while(1){
		pthread_t th_client;
		int thread_num;
		int client_sock;
		struct sockaddr_in client;
		unsigned int clientlen = sizeof(struct sockaddr_in);
		if((client_sock = accept(server_sock, (struct sockaddr *) &client, (socklen_t *)&clientlen)) < 0){
			onErrorReport(SOCK_CLIENT_ACCEPT_FAILED);
		}
		printf("\nConnected to client : %s Port %d", inet_ntoa(client.sin_addr), ntohs(client.sin_port));
		onClientSocketConnected(client_sock);
		this->crrt_client_sock = client_sock;
		thread_num = pthread_create(&th_client, NULL, static_th_client_handle, this);
	}
}

void *SocketHandler::static_th_client_handle(void *This){
	((SocketHandler *)This)->th_client_handle(&((SocketHandler *)This)->crrt_client_sock);
}

void *SocketHandler::th_client_handle(void *sock_client){
	int sock = *(int *)sock_client;
	printf("\nI am in thread_handle_client: %d", sock);
	int write_len, read_len;
	void *write_msg, *read_msg;
	read_msg = malloc(MAX_SOCKET_MSG_LEN);
	// read message from client
	while(1) {
		read_len = read(sock, read_msg, MAX_SOCKET_MSG_LEN);
		//Send the message back to client
		if (read_len > 0) {
			onReceivedMessage(sock, read_msg, read_len);
		} else if(read_len ==0 ){
			close(sock);
			onReceivedMessage(sock, NULL, 0, C_SOCK_CLOSED);
			break;
		} else {
			onReceivedMessage(sock, NULL, 0, C_SOCK_CLOSED);
		}
		memset(read_msg, 0, MAX_SOCKET_MSG_LEN);
	}
}

////
//
//void SocketHandler::onClientSocketConnected(int sock_num){
//	printf("\nConnected to client %d");
//}
//
//void SocketHandler::onErrorReport(ServerSocketStatus stt){
//	switch(stt){
//	case SOCK_THREAD_FAILED:
//		perror("\nFailed to socket hander thread.");
//		break;
//	case SOCK_CREATE_FAILED:
//		perror("\nFailed to open server socket.");
//		break;
//	case SOCK_CLIENT_ACCEPT_FAILED:
//		perror("\nFailed to accept client socket.");
//		break;
//	case SOCK_LISTEN_FAILED:
//		perror("\nFailed to listen client socket.");
//		break;
//	case SOCK_BIND_FAILED:
//		perror("\nFailed to bind server socket.");
//		break;
//	}
//}
//
//void SocketHandler::onReceivedMessage(int sock_num, void *msg, int len, ClientSocketStatus stt){
//	char *rMsg = "Hello! I am server. Message confirmed!";
//	switch(stt){
//	case C_SOCK_OK:
//		printf("\nReceived msg: %s", (char *)msg);
//		responseMsg(sock_num, msg, strlen(rMsg));
//		break;
//	case C_SOCK_CLOSED:
//		break;
//	}
//}
