#set ARM gcc compiler/cross-compiler

CC=gcc

deb: 00-deb 01-deb 02-deb 03-deb 04-deb 05-deb 06-deb

rel: 00-rel 01-rel 02-rel 03-rel 04-rel 05-rel 06-rel

00-deb:
	$(CC) -Wall -g -o 00-mouse 00-mouse.c
00-rel:
	$(CC) -Wall -O2 -s -o 00-mouse 00-mouse.c


01-deb:
	$(CC) -Wall -g -o 01-keyboard 01-keyboard.c
01-rel:
	$(CC) -Wall -O2 -s -o 01-keyboard 01-keyboard.c


02-deb:
	$(CC) -Wall -g -mcpu=cortex-a53 -mfpu=neon -mtune=cortex-a53 -o 02-neon 02-neon.c
02-rel:
	$(CC) -Wall -O2 -s -mcpu=cortex-a53 -mfpu=neon -mtune=cortex-a53 -o 02-neon 02-neon.c


03-deb:
	$(CC) -Wall -g -mcpu=cortex-a53 -mfpu=neon -o 03-camera 03-camera.c
03-rel:
	$(CC) -Wall -O3 -s -mcpu=cortex-a53 -mfpu=neon -o 03-camera 03-camera.c


04-deb:
	$(CC) -Wall -g -mcpu=cortex-a53 -mfpu=neon -o 04-stream 04-stream.c
04-rel:
	$(CC) -Wall -O3 -s -mcpu=cortex-a53 -mfpu=neon -o 04-stream 04-stream.c


05-deb:
	$(CC) -Wall -g -mcpu=cortex-a53 -mfpu=neon -mtune=cortex-a53 -o 05-neon 05-neon.c
05-rel:
	$(CC) -Wall -O2 -s -mcpu=cortex-a53 -mfpu=neon -mtune=cortex-a53 -o 05-neon 05-neon.c

06-deb:
	$(CC) -Wall -g -mcpu=cortex-a53 -mfpu=neon -mtune=cortex-a53 -o 06-fb 06-fb.c
06-rel:
	$(CC) -Wall -O2 -s -mcpu=cortex-a53 -mfpu=neon -mtune=cortex-a53 -o 06-fb 06-fb.c


clean:
	rm -rf 00-mouse 01-keyboard 02-neon 03-camera 04-stream 05-neon 06-fb rm *.s

ctags:
	ctags -R . /usr/include/ /opt/vc/include/

asm:
#	$(CC) -c -O2 -mcpu=cortex-a53 -mfpu=neon 02-neon.c -mtune=cortex-a53 -Wa,-a,-ad > 02-neon.asm
	$(CC) -c -O2 -mcpu=cortex-a53 -mfpu=neon 02-neon.c -mtune=cortex-a53 -S
	$(CC) -c -O2 -mcpu=cortex-a53 -mfpu=neon 03-camera.c -mtune=cortex-a53 -S
	$(CC) -c -O2 -mcpu=cortex-a53 -mfpu=neon 05-neon.c -mtune=cortex-a53 -S
