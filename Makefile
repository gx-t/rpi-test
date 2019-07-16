#set ARM gcc compiler/cross-compiler
MAKEFLAGS+=-j 20
CC=gcc
DEB=-Wall -g
REL=-Wall -O3 -s
PROF=-Wall -pg
VCLIB=/opt/vc/lib/
VCINC=/opt/vc/include/
ifeq ($(shell uname -m), armv7l)
	MFLAGS=-mcpu=cortex-a53 -mfpu=neon -mtune=cortex-a53
else
    MFLAGS=
endif

deb: 00-deb 01-deb 02-deb 03-deb 04-deb 05-deb 06-deb 07-deb 08-deb 09-deb 10-deb 11-deb 12-deb 13-deb 14-deb 15-deb 16-deb 17-deb 18-deb 19-deb

rel: 00-rel 01-rel 02-rel 03-rel 04-rel 05-rel 06-rel 07-rel 08-rel 09-rel 10-rel 11-rel 12-rel 13-rel 14-rel 15-rel 16-rel 17-rel 18-rel 19-rel

00-deb:
	$(CC) $(DEB) -o 00-mouse 00-mouse.c
00-rel:
	$(CC) $(REL) -o 00-mouse 00-mouse.c


01-deb:
	$(CC) $(DEB) -o 01-keyboard 01-keyboard.c
01-rel:
	$(CC) $(REL) -o 01-keyboard 01-keyboard.c


02-deb:
	$(CC) $(DEB) $(MFLAGS) -o 02-neon 02-neon.c
02-rel:
	$(CC) $(REL) $(MFLAGS) -o 02-neon 02-neon.c


03-deb:
	$(CC) $(DEB) $(MFLAGS) -o 03-camera 03-camera.c
03-rel:
	$(CC) $(REL) $(MFLAGS) -o 03-camera 03-camera.c


04-deb:
	$(CC) $(DEB) $(MFLAGS) -o 04-stream 04-stream.c
04-rel:
	$(CC) $(REL) $(MFLAGS) -o 04-stream 04-stream.c


05-deb:
	$(CC) $(DEB) $(MFLAGS) -o 05-neon 05-neon.c
05-rel:
	$(CC) $(REL) $(MFLAGS) -o 05-neon 05-neon.c
05-flac:
	./05-neon | sox -r 48k -t f32 -c 1 - 05.flac
05-ogg:
	./05-neon | sox -r 48k -t f32 -c 1 - 05.ogg

06-deb:
	$(CC) $(DEB) $(MFLAGS) -o 06-fb 06-fb.c
06-rel:
	$(CC) $(REL) $(MFLAGS) -o 06-fb 06-fb.c

07-deb:
	$(CC) $(DEB) $(MFLAGS) -o 07-chirp 07-chirp.c
07-rel:
	$(CC) $(REL) $(MFLAGS) -o 07-chirp 07-chirp.c

08-deb:
	$(CC) $(DEB) $(MFLAGS) -o 08-resonance 08-resonance.c -lm
08-rel:
	$(CC) $(REL) $(MFLAGS) -o 08-resonance 08-resonance.c -lm

09-deb:
	$(CC) $(DEB) $(MFLAGS) -o 09-noise 09-noise.c
09-rel:
	$(CC) $(REL) $(MFLAGS) -o 09-noise 09-noise.c

10-deb:
	$(CC) $(DEB) $(MFLAGS) -o 10-resonance 10-resonance.c
10-rel:
	$(CC) $(REL) $(MFLAGS) -o 10-resonance 10-resonance.c

11-deb:
	$(CC) $(DEB) $(MFLAGS) -o 11-sweep 11-sweep.c
11-rel:
	$(CC) $(REL) $(MFLAGS) -o 11-sweep 11-sweep.c

12-deb:
	$(CC) $(DEB) -o 12-evol 12-evol.c
12-rel:
	$(CC) $(REL) -o 12-evol 12-evol.c
12-prof:
	$(CC) $(PROF) -o 12-evol 12-evol.c
# make 12-prof
# ./12-evol evol-quad-eq 2 -4 -2 100000 256 -100.0 100.0 1.0
# gprof 12-evol

13-deb:
	$(CC) $(DEB) -o 13-lora-tx 13-lora-tx.c -lpigpio -lpthread
13-rel:
	$(CC) $(REL) -o 13-lora-tx 13-lora-tx.c -lpigpio -lpthread

14-deb:
	$(CC) $(DEB) -o 14-lora-rx 14-lora-rx.c -lpigpio -lpthread
14-rel:
	$(CC) $(REL) -o 14-lora-rx 14-lora-rx.c -lpigpio -lpthread

15-deb:
	$(CC) $(DEB) -o 15-udp 15-udp.c
15-rel:
	$(CC) $(REL) -o 15-udp 15-udp.c

16-deb:
	$(CC) $(DEB) -o 16-png-evol 16-png-evol.c -lpng
16-rel:
	$(CC) $(REL) -o 16-png-evol 16-png-evol.c -lpng
16-mp4:
	./16-png-evol | ffmpeg -y -r 2 -i - -c:v libx264 -vf "fps=30,format=yuv420p" 16-out.mp4

17-deb:
	$(CC) $(DEB) -o 17-raw-evol 17-raw-evol.c
17-rel:
	$(CC) $(REL) -o 17-raw-evol 17-raw-evol.c
17-mp4:
	./17-raw-evol | ffmpeg -y -s 200x200 -pix_fmt rgba -f rawvideo -r 2 -i - -c:v libx264 -vf "fps=30,format=yuv420p" 17-out.mp4

18-deb:
	$(CC) $(DEB) -o 18-ffmpeg-evol 18-ffmpeg-evol.c
18-rel:
	$(CC) $(REL) -o 18-ffmpeg-evol 18-ffmpeg-evol.c

19-deb:
	$(CC) $(DEB) -o 19-ring 19-ring.c
19-rel:
	$(CC) $(REL) -o 19-ring 19-ring.c

clean:
	rm -rf 00-mouse 01-keyboard 02-neon 03-camera 04-stream 05-neon 06-fb 07-chirp 08-resonance 09-noise 10-resonance 11-sweep 12-evol 13-lora-tx 14-lora-rx 15-udp 16-png-evol 17-raw-evol 18-ffmpeg-evol 19-ring rm ./tags *.s rm *.mp4 *.flac *.ogg

tags:
	ctags -R . /usr/include/ /opt/vc/include/ /usr/lib/gcc/arm-linux-gnueabihf/6/include

asm:
#	$(CC) -c -O2 -mcpu=cortex-a53 -mfpu=neon 02-neon.c -mtune=cortex-a53 -Wa,-a,-ad > 02-neon.asm
	$(CC) -c -O2 -mcpu=cortex-a53 -mfpu=neon 02-neon.c -mtune=cortex-a53 -S
	$(CC) -c -O2 -mcpu=cortex-a53 -mfpu=neon 03-camera.c -mtune=cortex-a53 -S
	$(CC) -c -O2 -mcpu=cortex-a53 -mfpu=neon 05-neon.c -mtune=cortex-a53 -S
