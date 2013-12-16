CC := clang

PICAM_LIBS := -lpicam -lmmal -lvcos -lbcm_host
CCV_LIBS := -lccv -lm -ljpeg -lpng -lz -lgsl -lgslcblas

all:
	$(CC) main.c -o track -g -L$(HOME)/lib/ccv/lib -I$(HOME)/lib/ccv/lib -Wall -O3 -ffast-math $(PICAM_LIBS) $(CCV_LIBS)

clean:
	rm track

install:
	cp track /usr/local/bin
