CC 						:= gcc
COMPILER 				:= 	g++
TARGET					:= camera_security
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
	$(COMPILER) -O3 -o $(TARGET) main.o camera_secure.o `pkg-config --static --cflags --libs opencv` $(LD_L_PATH)
#`pkg-config --static --cflags --libs opencv`
#$(LDFLAGS)

clean:
	rm -rf *.o *.a
	rm $(TARGET)
install:
	#sudo mkdir $(BIN_PATH)
	sudo rm $(BIN_PATH)/$(TARGET)
	sudo cp $(TARGET) $(BIN_PATH)
	sudo strip $(BIN_PATH)/$(TARGET)
