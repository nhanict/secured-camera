CC 						:= gcc
COMPILER 				:= 	g++
TARGET					:= camera_security
VERSION 				:= 1
SUBVERSION				:= 0
VERSION_EXTEND		:= ""
VERSION_FILE 			:= version.h
BIN_PATH 				:= /opt/camera_security
LD_L_PATH 				:= -L/usr/lib/i386-linux-gnu
LDFLAGS :=  -L/usr/lib -L/usr/local/lib -L/usr/local/share/OpenCV/3rdparty/lib -Wl,-Bstatic \
			  -lopencv_objdetect \
			  -lopencv_calib3d -lopencv_features2d -lopencv_highgui -lopencv_videoio -lopencv_imgcodecs \
			  -lopencv_video -lopencv_photo -lopencv_ml -lopencv_imgproc -lopencv_flann -lopencv_core -llibwebp \
			  -lippicv -lpng -lz -ltiff -ljasper -ljpeg -lImath -lIlmImf -lIex -lHalf -lIlmThread -lgtk-x11-2.0 \
			  -lgdk-x11-2.0 -latk-1.0 -lgio-2.0 -lpangoft2-1.0 -lpangocairo-1.0 -lgdk_pixbuf-2.0 -lcairo -lpango-1.0 \
			  -lfontconfig -lfreetype -lgstvideo-0.10 -lgstapp-0.10 -lgstbase-0.10 -lgstriff-0.10 -lgstpbutils-0.10 \
			  -lgstreamer-0.10 -lgobject-2.0 -lgmodule-2.0 -lgthread-2.0 -lxml2 -lglib-2.0 -ldc1394 -lavcodec \
			  -lavformat -lavutil -lswscale -lbz2 -lstdc++ -ldl -lm -lpthread -lrt

INCLUDES 				:= -I/usr/include/i386-linux-gnu -I/usr/include -I/usr/local/include
MACROS					:= -DVERSION=$(VERSION) -DSUBVERSION=$(SUBVERSION)



all: header_make build

header_make:
	echo "Generating $(VERSION_FILE)..."
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
