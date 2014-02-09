import liblo, serial

def printAll(pathstr, arg, typestr, server, usrData):
    print pathstr, arg

s = liblo.Server(8000)


arduino = serial.Serial('/dev/ttyACM0', 9600)

def writeMessage(byte1, byte2):
    arduino.write(chr(254)) # start
    arduino.write(byte1)
    arduino.write(byte2)
    arduino.write(chr(255)) # stop

def handlePage1(pathlist, arg):
    if pathlist[0]=='fader1':
        writeMessage('H', chr(int(arg[0]*250)))
    elif pathlist[0]=='fader2':
        writeMessage('S', chr(int(arg[0]*250)))
    elif pathlist[0]=='fader3':
        writeMessage('I', chr(int(arg[0]*250)))
    elif pathlist[0]=='fader4':
        writeMessage('o', chr(int(arg[0]*250)))

def handlePage2(pathlist, arg):
    if pathlist[0]=='push1':
        if arg[0]==1.0:
            writeMessage('m', 'u')
    elif pathlist[0]=='push2':
        if arg[0]==1.0:
            writeMessage('m', 'r')
    elif pathlist[0]=='push3':
        if arg[0]==1.0:
            writeMessage('m', 'R')

def routeByName(pathstr, arg, typestr, server, usrData):
    print pathstr, arg
    pathlist = pathstr[1:].split('/')
    if pathlist[0]=='1':
        handlePage1(pathlist[1:], arg)
    elif pathlist[0]=='2':
        handlePage2(pathlist[1:], arg)


#############################################

s.add_method(None, None, routeByName)

#############################################
print 'Starting OSC server on 8000...'
while True:
    s.recv(1) # 1 ms refresh rate
