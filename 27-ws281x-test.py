import machine, neopixel, time, random

num_leds = 288
np = neopixel.NeoPixel(machine.Pin(0), num_leds)

def rand_color():
    r = random.random()
    g = random.random()
    b = random.random()
    return r, g, b

def set_pos_16(pos):
    return [pos] * 16

def push_pos_16(led, pos):
    return [pos] + led[:-1]

def erase_leds(led):
    for pos in led:
        np[pos] = (0, 0, 0)

def fill_colors_16(led, r, g, b):
    r *= 255.0
    g *= 255.0
    b *= 255.0
    for pos in led:
        old_r, old_g, old_b = np[pos]
        np[pos] = (old_r | int(r), old_g | int(g), old_b | int(b))
        r /= 1.41
        g /= 1.41
        b /= 1.41

c = [0.999, 0.99, 0.99, 0.99]
s = [0.0, 0.0, 0.0, 0.0]
f = [0.02, 0.002, 0.0022, 0.0025]

y = 0.0
v = 0.0

led1 = [144 + int(144 * s[0]**2)] * 16
led2 = [144 + int(144 * c[0]**2)] * 16
led3 = [int(144.0 * y)] * 16

led3_r, led3_g, led3_b = rand_color()

def oscilate():
    while True:
        utime.sleep(0.03)

while True:
    for i in range(len(c)):
        c[i] -= s[i] * f[i]
        s[i] += c[i] * f[i]
    f[0] += c[2] * 0.00001

    v += (-0.0002)
    y += v
    if y <= 0.0:
        y = 0.0
        v = -v
        v *= (1.0/1.25)
        if v < 0.0015:
            v = 0.02
            led3_r, led3_g, led3_b = rand_color()


    led1 = push_pos_16(led1, 144 + int(144 * s[0]**2))
    led2 = push_pos_16(led2, 144 + int(144 * c[0]**2))
    led3 = push_pos_16(led3, int(y * 144.0))
    fill_colors_16(led1, s[1]**2, s[2]**2, s[3]**2)
    fill_colors_16(led2, c[1]**2, c[2]**2, c[3]**2)
    fill_colors_16(led3, led3_r, led3_g, led3_b)
    np.write()
    erase_leds(led1)
    erase_leds(led2)
    erase_leds(led3)


