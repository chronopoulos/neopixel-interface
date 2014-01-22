import serial
import liblo

arduino = serial.Serial('/dev/ttyUSB0', 14400)

arduino.write(chr(254)) # start
arduino.write(chr(0))
arduino.write(chr(1))
arduino.write(chr(2))
arduino.write(chr(255)) # stop
