deb: 00-deb 01-deb 02-deb 03-deb 04-deb 05-deb

rel: 00-rel 01-rel 02-rel 03-rel 04-rel 05-rel

00-deb:
	gcc -Wall -g -o 00-mouse 00-mouse.c
00-rel:
	gcc -Wall -O2 -s -o 00-mouse 00-mouse.c


01-deb:
	gcc -Wall -g -o 01-keyboard 01-keyboard.c
01-rel:
	gcc -Wall -O2 -s -o 01-keyboard 01-keyboard.c


02-deb:
	gcc -Wall -g -mcpu=cortex-a53 -mfpu=neon -mtune=cortex-a53 -o 02-neon 02-neon.c
02-rel:
	gcc -Wall -O2 -s -mcpu=cortex-a53 -mfpu=neon -mtune=cortex-a53 -o 02-neon 02-neon.c


03-deb:
	gcc -Wall -g -mcpu=cortex-a53 -mfpu=neon -o 03-camera 03-camera.c
03-rel:
	gcc -Wall -O3 -s -mcpu=cortex-a53 -mfpu=neon -o 03-camera 03-camera.c


04-deb:
	gcc -Wall -g -mcpu=cortex-a53 -mfpu=neon -o 04-stream 04-stream.c
04-rel:
	gcc -Wall -O3 -s -mcpu=cortex-a53 -mfpu=neon -o 04-stream 04-stream.c


05-deb:
	gcc -Wall -g -mcpu=cortex-a53 -mfpu=neon -mtune=cortex-a53 -o 05-neon 05-neon.c
05-rel:
	gcc -Wall -O2 -s -mcpu=cortex-a53 -mfpu=neon -mtune=cortex-a53 -o 05-neon 05-neon.c


clean:
	rm -rf 00-mouse 01-keyboard 02-neon 03-camera 04-stream 05-neon rm *.s

asm:
#	gcc -c -O2 -mcpu=cortex-a53 -mfpu=neon 02-neon.c -mtune=cortex-a53 -Wa,-a,-ad > 02-neon.asm
	gcc -c -O2 -mcpu=cortex-a53 -mfpu=neon 02-neon.c -mtune=cortex-a53 -S
	gcc -c -O2 -mcpu=cortex-a53 -mfpu=neon 03-camera.c -mtune=cortex-a53 -S
	gcc -c -O2 -mcpu=cortex-a53 -mfpu=neon 05-neon.c -mtune=cortex-a53 -S
