CC 						:= gcc
COMPILER 				:= 	g++
TARGET					:= camera_security
TARGET2				:= client_socket
VERSION 				:= 1
SUBVERSION				:= 0
VERSION_EXTEND		:= ""
VERSION_FILE 			:= version.h
BIN_PATH 				:= /opt/camera_security
LD_L_PATH 				:= -L/usr/lib/i386-linux-gnu
INCLUDES 				:= -I/usr/include/i386-linux-gnu -I/usr/include -I/usr/local/include
MACROS					:= -DVERSION=$(VERSION) -DSUBVERSION=$(SUBVERSION)



all: header_make build

header_make:
	@echo "Generating $(VERSION_FILE)..."
build:	
	$(COMPILER) -std=c++11 -O3 $(INCLUDES) -fPIC -c camera_secure.cpp
	$(COMPILER) -std=c++11 -O3 $(INCLUDES) -fPIC -c main.cpp $(MACROS)
	$(COMPILER) -std=c++11 -O3 $(INCLUDES) -fPIC -c client_socket.cpp
	$(COMPILER) -std=c++11 -O3 $(INCLUDES) -fPIC -c socket_handler.cpp
	$(COMPILER) -std=c++11 -O3 $(INCLUDES) -fPIC -c secure_camera_socket_handler.cpp
	$(COMPILER) -O3 -o $(TARGET) main.o camera_secure.o socket_handler.o secure_camera_socket_handler.o `pkg-config --static --cflags --libs opencv` $(LD_L_PATH)
	$(COMPILER) -O3 -o $(TARGET2) client_socket.o $(LD_L_PATH) -lpthread
	strip $(TARGET)
#`pkg-config --static --cflags --libs opencv`
#$(LDFLAGS)

clean:
	rm -rf *.o *.a
	rm $(TARGET)
	rm $(TARGET2)
install:
	#sudo mkdir $(BIN_PATH)
	sudo rm $(BIN_PATH)/$(TARGET)
	sudo cp $(TARGET) $(BIN_PATH)
	sudo strip $(BIN_PATH)/$(TARGET)
