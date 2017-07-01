#set ARM gcc compiler/cross-compiler

CC=gcc
DEB=-g
REL=-O2 -s
VCLIB=/opt/vc/lib/
VCINC=/opt/vc/include/

deb: 00-deb 01-deb 02-deb 03-deb 04-deb 05-deb 06-deb 07-deb 08-deb 09-deb

rel: 00-rel 01-rel 02-rel 03-rel 04-rel 05-rel 06-rel 07-rel 08-rel 09-rel

00-deb:
	$(CC) -Wall $(DEB) -o 00-mouse 00-mouse.c
00-rel:
	$(CC) -Wall $(REL) -o 00-mouse 00-mouse.c


01-deb:
	$(CC) -Wall $(DEB) -o 01-keyboard 01-keyboard.c
01-rel:
	$(CC) -Wall $(REL) -o 01-keyboard 01-keyboard.c


02-deb:
	$(CC) -Wall $(DEB) -mcpu=cortex-a53 -mfpu=neon -mtune=cortex-a53 -o 02-neon 02-neon.c
02-rel:
	$(CC) -Wall $(REL) -mcpu=cortex-a53 -mfpu=neon -mtune=cortex-a53 -o 02-neon 02-neon.c


03-deb:
	$(CC) -Wall $(DEB) -mcpu=cortex-a53 -mfpu=neon -o 03-camera 03-camera.c
03-rel:
	$(CC) -Wall $(REL) -mcpu=cortex-a53 -mfpu=neon -o 03-camera 03-camera.c


04-deb:
	$(CC) -Wall $(DEB) -mcpu=cortex-a53 -mfpu=neon -o 04-stream 04-stream.c
04-rel:
	$(CC) -Wall $(REL) -mcpu=cortex-a53 -mfpu=neon -o 04-stream 04-stream.c


05-deb:
	$(CC) -Wall $(DEB) -mcpu=cortex-a53 -mfpu=neon -mtune=cortex-a53 -o 05-neon 05-neon.c
05-rel:
	$(CC) -Wall $(REL) -mcpu=cortex-a53 -mfpu=neon -mtune=cortex-a53 -o 05-neon 05-neon.c

06-deb:
	$(CC) -Wall $(DEB) -mcpu=cortex-a53 -mfpu=neon -mtune=cortex-a53 -o 06-fb 06-fb.c
06-rel:
	$(CC) -Wall $(REL) -mcpu=cortex-a53 -mfpu=neon -mtune=cortex-a53 -o 06-fb 06-fb.c

07-deb:
	$(CC) -Wall -g -mcpu=cortex-a53 -mfpu=neon -mtune=cortex-a53 -o 07-chirp 07-chirp.c -lm
07-rel:
	$(CC) -Wall -O2 -s -mcpu=cortex-a53 -mfpu=neon -mtune=cortex-a53 -o 07-chirp 07-chirp.c -lm

08-deb:
	$(CC) -Wall -g -mcpu=cortex-a53 -mfpu=neon -mtune=cortex-a53 -o 08-resonance 08-resonance.c -lm
08-rel:
	$(CC) -Wall -O2 -s -mcpu=cortex-a53 -mfpu=neon -mtune=cortex-a53 -o 08-resonance 08-resonance.c -lm

09-deb:
	$(CC) -Wall -g -mcpu=cortex-a53 -mfpu=neon -mtune=cortex-a53 -o 09-noise 09-noise.c -lm
09-rel:
	$(CC) -Wall -O2 -s -mcpu=cortex-a53 -mfpu=neon -mtune=cortex-a53 -o 09-noise 09-noise.c -lm


clean:
	rm -rf 00-mouse 01-keyboard 02-neon 03-camera 04-stream 05-neon 06-fb 07-chirp 08-resonance 09-noise rm *.s

ctags:
	ctags -R . /usr/include/ /opt/vc/include/

asm:
#	$(CC) -c -O2 -mcpu=cortex-a53 -mfpu=neon 02-neon.c -mtune=cortex-a53 -Wa,-a,-ad > 02-neon.asm
	$(CC) -c -O2 -mcpu=cortex-a53 -mfpu=neon 02-neon.c -mtune=cortex-a53 -S
	$(CC) -c -O2 -mcpu=cortex-a53 -mfpu=neon 03-camera.c -mtune=cortex-a53 -S
	$(CC) -c -O2 -mcpu=cortex-a53 -mfpu=neon 05-neon.c -mtune=cortex-a53 -S
