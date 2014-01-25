#!/usr/bin/env python
"""
Example of a UDP txosc receiver with Twisted.

This example is in the public domain.
"""
from twisted.internet import reactor
from txosc import osc
from txosc import dispatch
from txosc import async
import serial
import re

arduino = serial.Serial('/dev/tty.usbmodem1411', baudrate=9600)

# /dev/tty.usbserial-FTF3MN94
class UDPReceiverApplication(object):
    """
    Example that receives UDP OSC messages.
    """
    def __init__(self, port):
        self.port = port
        self.receiver = dispatch.Receiver()
        self._server_port = reactor.listenUDP(self.port, async.DatagramServerProtocol(self.receiver))
        print("Listening on osc.udp://localhost:%s" % (self.port))

        # Simple
        self.receiver.addCallback("/1/fader1", self.hue_handler)
        self.receiver.addCallback("/1/fader2", self.sat_handler)
        self.receiver.addCallback("/1/fader3", self.val_handler)
        self.receiver.addCallback("/1/fader5", self.speed_handler)
        self.receiver.addCallback("/2/*", self.mode_handler)

        # Beat Machine
        self.receiver.addCallback("/3/rotary1", self.hue_handler)
        self.receiver.addCallback("/3/rotary2", self.sat_handler)
        self.receiver.addCallback("/3/rotary3", self.val_handler)

        # quit
        self.receiver.addCallback("/quit", self.quit_handler)

        # fallback:
        self.receiver.fallback = self.fallback


    def hue_handler(self, message, address):
        send_simple('H', message)

    def sat_handler(self, message, address):
        send_simple('S', message)

    def val_handler(self, message, address):
        send_simple('V', message)

    def speed_handler(self, message, address):
        send_simple('s', message)

    def mode_handler(self, message, address):
        if (message.getValues()[0] > 0.5): # only do stuff on button push, not button release
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
        Method handler for /quit
        Quits the application.
        """
        print("quit_handler")
        print("  Got %s from %s" % (message, address))
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
    value_to_send = int(value*250)
    arduino.write(chr(254)) # start
    arduino.write(letter)
    arduino.write(chr(value_to_send))
    arduino.write(chr(255)) # stop
    print ("Send %s with value %d" % (letter, value_to_send))

def get_address(message):
    return str(message).split(' ')[0]

def get_number(message):
    address = get_address(message)
    element_string = address.split('/')[2]
    element_number = int(re.sub(r'\D', '', element_string))
    return element_number

modes = [None, 'u', 'r', 'f']

if __name__ == "__main__":
    app = UDPReceiverApplication(17779)
    reactor.run()
