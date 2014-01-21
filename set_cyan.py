import serial

arduino = serial.Serial('/dev/ttyUSB0', 57600)

for i in range(60):
    arduino.write(chr(0))       # r
    arduino.write(chr(255))     # g
    arduino.write(chr(255))     # b
