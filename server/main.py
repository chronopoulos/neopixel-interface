#!/usr/bin/env python
from twisted.internet import reactor
from txosc import osc
from txosc import dispatch
from txosc import async
import serial
import re
from datetime import datetime
import time

arduino = serial.Serial('/dev/ttyACM0', baudrate=9600) # tty.usbmodem1421 # ttyACM0
time.sleep(1.3) # sleep while wait for handshake to complete

last_msg_time = datetime.now()
uniform_hue = 0
offset = 0

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
        self.receiver.addCallback("/rainbow/*", self.rainbow_handler)
        self.receiver.addCallback("/wheel/*", self.wheel_handler)
        self.receiver.addCallback("/fade/*", self.fade_handler)
        self.receiver.addCallback("/inertial_random/*", self.inertial_random_handler)


        # fallback:
        self.receiver.fallback = self.fallback


        send_mode('u'); # when server starts, set mode to uniform

    def uniform_handler(self, message, address):
        elements_dict = {
            'hue_encoder': self.send_encoder_hue,
            'sat_fader': self.saturation_handler,
            'intensity_fader': self.intensity_handler
        }
        element = get_element(message)
        if element in elements_dict:
            elements_dict[element](message)
        else:
            print 'invalid element sent'

    def rainbow_handler(self, message, address):
        elements_dict = {
            'position_encoder': self.send_encoder_offset,
            'speed_fader': self.speed_handler
        }
        element = get_element(message)
        if element in elements_dict:
            elements_dict[element](message)
        else:
            print 'invalid element sent'

    def wheel_handler(self, message, address):
        elements_dict = {
            'speed_fader': self.speed_handler,
            'sat_fader': self.saturation_handler,
            'intensity_fader': self.intensity_handler
        }
        element = get_element(message)
        if element in elements_dict:
            elements_dict[element](message)
        else:
            print 'invalid element sent'

    def fade_handler(self, message, address):
        elements_dict = {
            'hue_encoder': self.send_encoder_hue,
            'sat_fader': self.saturation_handler,
            'intensity_fader': self.intensity_handler
        }
        element = get_element(message)
        if element in elements_dict:
            elements_dict[element](message)
        else:
            print 'invalid element sent'

    def inertial_random_handler(self, message, address):
        elements_dict = {
            'speed_fader': self.speed_handler,
            'acceleration_fader': self.acceleration_handler,
            'sat_fader': self.saturation_handler,
            'intensity_fader': self.intensity_handler
        }
        element = get_element(message)
        if element in elements_dict:
            elements_dict[element](message)
        else:
            print 'invalid element sent'

    def mode_page_handler(self, message, address):
        mode_char_lookup = {
            'uniform' : 'u',
            'rainbow' : 'r',
            'wheel' : 'w',
            'fade' : 'f',
            'inertial_random' : 'i'
        }
        mode_string = get_page(message)
        if mode_string in mode_char_lookup:
            send_mode(mode_char_lookup[mode_string])
        else:
            print 'invalid mode sent'

    def send_encoder_hue(self, message):
        if (check_ready()):
            value = message.getValues()[0]
            global uniform_hue
            if (value == 1):
                uniform_hue += 1
            else:
                uniform_hue -= 1
            uniform_hue = uniform_hue % 251
            write_message('H', uniform_hue)

    def send_encoder_offset(self, message):
        if (check_ready()):
            value = message.getValues()[0]
            global offset
            if (value == 1):
                offset += 1
            else:
                offset -= 1
            offset = offset % 251
            write_message('o', offset)

    def hue_handler(self, message, address):
        send_simple('H', message)

    def saturation_handler(self, message, *args):
        send_simple('S', message)

    def intensity_handler(self, message, *args):
        send_simple('I', message)

    def offset_handler(self, message, address):
        send_simple('o', message) # o for rainbowOffset

    def speed_handler(self, message, *args):
        send_simple('s', message)

    def acceleration_handler(self, message, *args):
        send_simple('a', message)

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
    value_to_send = int(value*250)
    write_message(letter, value_to_send)

def write_message(letter, value_to_send):
    global last_msg_time
    if (check_ready()): # if at least n milliseconds have elapsed
        arduino.write(chr(254)) # start
        arduino.write(letter)
        arduino.write(chr(value_to_send))
        arduino.write(chr(255)) # stop
        last_msg_time = datetime.now()
        print ("Send %s with value %d" % (letter, value_to_send))
    else:
        print 'throttled'

def check_ready(timeout=5):
    global last_msg_time
    now = datetime.now()
    elapsed_time_ms = (now - last_msg_time).total_seconds() * 1000
    if (elapsed_time_ms > timeout): # if at least n milliseconds have elapsed
        return True
    else:
        return False

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
