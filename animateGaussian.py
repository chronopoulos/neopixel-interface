import serial, time, random
import numpy as np

arduino = serial.Serial('/dev/ttyUSB0', 14400)

x = np.linspace(-10,10,60)
c = -10.
inc = 0.5

while True:
    print c
    y = np.exp(-(x-c)**2/0.5)
    for i in range(60):
        if random.random() < 0.01:
            # "camera flash" effect
            arduino.write(chr(255)) # r
            arduino.write(chr(255))   # g
            arduino.write(chr(255))   # b
        else:
            arduino.write(chr(0)) # r
            arduino.write(chr(0))   # g
            arduino.write(chr(int(y[i]*255)))   # b
    c += inc
    if c>10.:
        inc = -0.5
    elif c<-10.:
        inc = 0.5
    time.sleep(0.1)
