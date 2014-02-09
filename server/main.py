#!/usr/bin/env python
from twisted.internet import reactor
from txosc import osc
from txosc import dispatch
from txosc import async
import serial
import re
from datetime import datetime

arduino = serial.Serial('/dev/tty.usbserial-FTF3MN94', baudrate=9600)
last_msg_time = datetime.now()

class UDPReceiverApplication(object):
    def __init__(self, port):
        self.port = port
        self.receiver = dispatch.Receiver()
        self._server_port = reactor.listenUDP(self.port, async.DatagramServerProtocol(self.receiver))
        print("Listening on osc.udp://localhost:%s" % (self.port))

        # Simple
        self.receiver.addCallback("/1/fader1", self.hue_handler)
        self.receiver.addCallback("/1/fader2", self.saturation_handler)
        self.receiver.addCallback("/1/fader3", self.intensity_handler)
        self.receiver.addCallback("/1/fader4", self.offset_handler)
        self.receiver.addCallback("/1/fader5", self.speed_handler)
        self.receiver.addCallback("/2/*", self.mode_handler)

        # Beat Machine
        self.receiver.addCallback("/3/rotary1", self.hue_handler)
        self.receiver.addCallback("/3/rotary2", self.saturation_handler)
        self.receiver.addCallback("/3/rotary3", self.intensity_handler)

        # Pillow Room
        self.receiver.addCallback("/*", self.mode_page_handler)
        self.receiver.addCallback("/uniform/*", self.uniform_handler)

        # fallback:
        self.receiver.fallback = self.fallback

    def uniform_handler(self, message, address):
        elements_dict ={
            'hue_encoder': self.hue_handler,
            'sat_fader': self.saturation_handler,
            'intensity_fader': self.intensity_handler
        }
        print 'this doesn\'t really do anything yet'

    def mode_page_handler(self, message, address):
        mode_lookup = {
            'uniform' : 'u',
            'rainbow' : 'r',
            'wheel' : 'w',
            'fade' : 'f'
        }
        send_mode(mode_lookup[get_page(message)])

    def hue_handler(self, message, address):
        send_simple('H', message)

    def saturation_handler(self, message, address):
        send_simple('S', message)

    def intensity_handler(self, message, address):
        send_simple('I', message)

    def offset_handler(self, message, address):
        send_simple('o', message) # o for rainbowOffset

    def speed_handler(self, message, address):
        send_simple('s', message)

    def mode_handler(self, message, address):
        if (message.getValues()[0]==1): # only do stuff on button push, not button release
            element_number = get_number(message)
            if element_number < len(modes):
                send_mode(modes[element_number])
            else:
                print("No mode associated with button %d" % element_number)

    def fallback(self, message, address):
        """
        Fallback for any unhandled message
        """
        print("Unhandled message %s from %s" % (message, address))

    def quit_handler(self, message, address):
        """
        Quits the application.
        """
        reactor.stop()
        print("Goodbye.")

def send_mode(letter):
    arduino.write(chr(254)) # start
    arduino.write('m')
    arduino.write(letter)
    arduino.write(chr(255)) # stop
    print ("Send mode to %s" % letter)

def send_simple(letter, message):
    values = message.getValues()
    value = values[0]
    write_message(letter, value)

def write_message(letter, value):
    global last_msg_time
    now = datetime.now()
    elapsed_time_ms = (now - last_msg_time).total_seconds() * 1000
    if (elapsed_time_ms > 30): # if at least 30 milliseconds have elapsed
        value_to_send = int(value*250)
        arduino.write(chr(254)) # start
        arduino.write(letter)
        arduino.write(chr(value_to_send))
        arduino.write(chr(255)) # stop
        last_msg_time = now
        print ("Send %s with value %d" % (letter, value_to_send))

def get_path(message):
    return str(message).split(' ')[0]

def get_page(message):
    path = get_path(message)
    return path.split('/')[1]

def get_element(message):
    path = get_path(message)
    return path.split('/')[2]

def get_number(message):
    element_string = get_element(message)
    element_number = int(re.sub(r'\D', '', element_string))
    return element_number

modes = [None, 'u', 'r', 'R', 'w', 'f']

if __name__ == "__main__":
    app = UDPReceiverApplication(8000)
    reactor.run()
