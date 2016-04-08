#include <stdlib.h>
#include <cstdio>
#include <string.h>
#include "secure_camera_socket_handler.h"

SecureCameraSH::SecureCameraSH(){}
SecureCameraSH::~SecureCameraSH(){}

void SecureCameraSH::onClientSocketConnected(int sock_num){
	printf("\nConnected to client socket num: %d", sock_num);
}

void SecureCameraSH::onErrorReport(ServerSocketStatus stt){
	switch(stt){
	case SOCK_THREAD_FAILED:
		perror("\nFailed to socket hander thread.");
		break;
	case SOCK_CREATE_FAILED:
		perror("\nFailed to open server socket.");
		break;
	case SOCK_CLIENT_ACCEPT_FAILED:
		perror("\nFailed to accept client socket.");
		break;
	case SOCK_LISTEN_FAILED:
		perror("\nFailed to listen client socket.");
		break;
	case SOCK_BIND_FAILED:
		perror("\nFailed to bind server socket.");
		break;
	}
}

void SecureCameraSH::onReceivedMessage(int sock_num, void *msg, int len, ClientSocketStatus stt){
	char *rMsg = "Hello! I am server. Message confirmed!";
	switch(stt){
	case C_SOCK_OK:
		printf("\nReceived msg(%d): %s", sock_num, (char *)msg);
		responseMsg(sock_num, rMsg, strlen(rMsg));
		break;
	case C_SOCK_CLOSED:
		break;
	}
}


