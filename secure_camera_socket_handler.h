#ifndef __SECURE_CAMERA_SH_H_
#define __SECURE_CAMERA_SH_H_

#include "socket_handler.h"

class SecureCameraSH : public SocketHandler {
public:
	SecureCameraSH();
	~SecureCameraSH();
private:
	void 			onClientSocketConnected(int sock_num);
	void 			onReceivedMessage(int sock_num, void *msg, int len, ClientSocketStatus stt = C_SOCK_OK);
	void 			onErrorReport(ServerSocketStatus stt);
};

#endif // __SECURE_CAMERA_SH_H_
