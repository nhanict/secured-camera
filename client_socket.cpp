
// Network socket
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "socket_handler.h"


void *thread_socket_reading(void *input){
	int sock_num = *(int *)input;
	char read_msg[1024];
	int read_len;
	while((read_len = read(sock_num, read_msg, 1024)) > 0) {
		//Send the message back to client
		printf("\nReceive message: %s", read_msg);
	}
}

void *thread_socket_writing(void *input){
	int sock_num = *(int *)input;
	char msg[MAX_SOCKET_MSG_LEN];
	while(1){
		printf("\nEnter message: ");
		fgets (msg, MAX_SOCKET_MSG_LEN, stdin);
		if(write(sock_num, msg, strlen(msg)) < 0){
			perror("\nFailed to write msg to server.");
		}
	}

}

//void signal_handler(int signal){
//	printf( "\nsignal catched: %d\n", signal);
//
//}

int main(int argc, char**argv){

	// signal handler
//	signal(SIGINT, signal_handler);
//	signal(SIGTERM, signal_handler);
//	signal(SIGPIPE, signal_handler);
//	signal(SIGQUIT, signal_handler);
//	signal(SIGKILL, signal_handler);
//	signal(SIGCONT, signal_handler);
//	signal(SIGHUP, signal_handler);
//	signal(SIGABRT, signal_handler);
//	signal(SIGSEGV, signal_handler);
	//

	int client_sock;
	struct sockaddr_in server_addr;

	client_sock = socket(AF_INET, SOCK_STREAM, 0);
	if(client_sock < 0){
		perror("\nFailed to create socket.");
		return -1;
	}

	memset(&server_addr, 0, sizeof(server_addr));
	//server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(SERVER_PORT);
	if(connect(client_sock, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0){
		perror("\nFailed to connect to Server");
		return -1;
	}

	int th_reading_stt, th_writing_stt;
	pthread_t th_reading, th_writing;
	if((th_reading_stt = pthread_create(&th_reading, NULL, thread_socket_reading, (void *) &client_sock))){
		perror("\nFailed to creating reading thread.");
	}
	if((th_writing_stt = pthread_create(&th_writing, NULL, thread_socket_writing, (void *) &client_sock))){
		perror("\nFailed to creating writing thread.");
	}
	pthread_join(th_reading, NULL);
	pthread_join(th_writing, NULL);

	return 0;
}

