#set ARM gcc compiler/cross-compiler
MAKEFLAGS+=-j 24
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

deb: 00-deb 01-deb 02-deb 03-deb 04-deb 05-deb 06-deb 07-deb 08-deb 09-deb 10-deb 11-deb 12-deb 13-deb 14-deb 15-deb 16-deb 17-deb 18-deb 19-deb 20-deb 21-deb 22-deb 23-deb 24-deb

rel: 00-rel 01-rel 02-rel 03-rel 04-rel 05-rel 06-rel 07-rel 08-rel 09-rel 10-rel 11-rel 12-rel 13-rel 14-rel 15-rel 16-rel 17-rel 18-rel 19-rel 20-rel 21-rel 22-rel 23-rel 24-rel

mp4: 16-mp4 17-mp4 18-mp4

flac: 19-flac

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
	./05-neon | sox -r 48k -t f32 -c 1 - flac/05.flac
05-ogg:
	./05-neon | sox -r 48k -t f32 -c 1 - ogg/05.ogg

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
16-png-evol: 16-png-evol.c
	$(CC) $(REL) -o 16-png-evol 16-png-evol.c -lpng
16-mp4: 16-png-evol
	./16-png-evol | ffmpeg -y -r 2 -i - -c:v libx264 -vf "fps=30,format=yuv420p" mp4/16-out.mp4

17-deb:
	$(CC) $(DEB) -o 17-raw-evol 17-raw-evol.c
17-rel:
	$(CC) $(REL) -o 17-raw-evol 17-raw-evol.c
17-raw-evol: 17-raw-evol.c
	$(CC) $(REL) -o 17-raw-evol 17-raw-evol.c
17-mp4: 17-raw-evol
	ffmpeg -i flower-400x400.jpeg -vf scale=400:400 -f rawvideo -pix_fmt rgba - | ./17-raw-evol | ffmpeg -y -s 400x400 -pix_fmt rgba -f rawvideo -r 30 -i - -c:v libx264 mp4/17-out.mp4

18-deb:
	$(CC) $(DEB) -o 18-ffmpeg-evol 18-ffmpeg-evol.c
18-rel:
	$(CC) $(REL) -o 18-ffmpeg-evol 18-ffmpeg-evol.c
18-ffmpeg-evol: 18-ffmpeg-evol.c
	$(CC) $(REL) -o 18-ffmpeg-evol 18-ffmpeg-evol.c
18-mp4: 18-ffmpeg-evol
	cd mp4 && ../18-ffmpeg-evol

19-deb:
	$(CC) $(DEB) -o 19-ring 19-ring.c
19-rel:
	$(CC) $(REL) -o 19-ring 19-ring.c
19-ring: 19-ring.c
	$(CC) $(REL) -o 19-ring 19-ring.c

19-flac: 19-0 19-1 19-2 19-3 19-4 19-5

19-0: 19-ring
	./19-ring 0 | sox -r 96k -t f32 -c 1 - flac/19-0.flac
19-1: 19-ring
	./19-ring 1 | sox -r 96k -t f32 -c 1 - flac/19-1.flac
19-2: 19-ring
	./19-ring 2 | sox -r 96k -t f32 -c 1 - flac/19-2.flac
19-3: 19-ring
	./19-ring 3 | sox -r 96k -t f32 -c 1 - flac/19-3.flac
19-4: 19-ring
	./19-ring 4 | sox -r 96k -t f32 -c 1 - flac/19-4.flac
19-5: 19-ring
	./19-ring 5 | sox -r 96k -t f32 -c 1 - flac/19-5.flac

20-deb:
	$(CC) $(DEB) -o 20-ll 20-ll.c
20-rel:
	$(CC) $(REL) -o 20-ll 20-ll.c

21-deb:
	$(CC) $(DEB) -o 21-nrf24l01 21-nrf24l01.c -pthread -lpigpio
	sudo chown root 21-nrf24l01
	sudo chmod +s 21-nrf24l01
21-rel:
	$(CC) $(REL) -o 21-nrf24l01 21-nrf24l01.c -pthread -lpigpio
	sudo chown root 21-nrf24l01
	sudo chmod +s 21-nrf24l01

22-deb:
	$(CC) $(DEB) -Wno-unused-function -o 22-temp-cgi 22-temp-cgi.c
22-rel:
	$(CC) $(REL) -Wno-unused-function -o 22-temp-cgi 22-temp-cgi.c

23-deb:
	$(CC) -g $(MFLAGS) -o 23-bench 23-bench.c -ldl
23-rel:
	$(CC) -O3 -s $(MFLAGS) -o 23-bench 23-bench.c -ldl

24-deb:
	$(CC) -g $(MFLAGS) -o 24-openssl-tests 24-openssl-tests.c -lcrypto
24-rel:
	$(CC) -O3 -s $(MFLAGS) -o 24-openssl-tests 24-openssl-tests.c -lcrypto

clean:
	rm -rf 00-mouse 01-keyboard 02-neon 03-camera 04-stream 05-neon 06-fb 07-chirp 08-resonance 09-noise 10-resonance 11-sweep 12-evol 13-lora-tx 14-lora-rx 15-udp 16-png-evol 17-raw-evol 18-ffmpeg-evol 19-ring 20-ll 21-nrf24l01 22-temp-cgi 23-bench 24-openssl-tests ./tags *.s rm mp4/*.mp4 flac/*.flac ogg/*.ogg

tag:
	ctags -R . /usr/include/ /opt/vc/include/ /usr/lib/gcc/arm-linux-gnueabihf/6/include
	cscope -bcqR -I /usr/include/ -I .

asm:
#	$(CC) -c -O2 -mcpu=cortex-a53 -mfpu=neon 02-neon.c -mtune=cortex-a53 -Wa,-a,-ad > 02-neon.asm
	$(CC) -c -O2 -mcpu=cortex-a53 -mfpu=neon 02-neon.c -mtune=cortex-a53 -S
	$(CC) -c -O2 -mcpu=cortex-a53 -mfpu=neon 03-camera.c -mtune=cortex-a53 -S
	$(CC) -c -O2 -mcpu=cortex-a53 -mfpu=neon 05-neon.c -mtune=cortex-a53 -S
	$(CC) $(REL) -c -S 19-ring.c
