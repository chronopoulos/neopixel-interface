import liblo, serial

def printAll(pathstr, arg, typestr, server, usrData):
    print pathstr, arg

s = liblo.Server(8000)
s.add_method(None, None, printAll)

arduino = serial.Serial('/dev/ttyUSB0', 9600)
arduino.write(chr(254)) # start
for x in message.getValues():
    arduino.write(chr(int(x*250)))
    arduino.write(chr(255)) # stop

print 'Starting OSC server on 8000...'
while True:
    s.recv(1) # 1 ms refresh rate
