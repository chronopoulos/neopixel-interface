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
        # self.receiver.addCallback("/*", self.wild_handler)
        # self.receiver.addCallback("/ping/*/*", self.ping_handler)
        self.receiver.addCallback("/quit", self.quit_handler)
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
        # self.receiver.addCallback("/ham/egg", self.ham_egg_handler)
        # self.receiver.addCallback("/*/egg", self.any_egg_handler)

        # # Now, let's demonstrate how to use address nodes:
        # # /cheese:
        # self.cheese_node = dispatch.AddressNode("cheese")
        # self.cheese_node.addCallback("*", self.cheese_handler)
        # self.receiver.addNode("cheese", self.cheese_node)
        # # /cheese/cheddar:
        # self.cheddar_node = dispatch.AddressNode("cheddar")
        # self.cheddar_node.addCallback("*", self.cheese_cheddar_handler)
        # self.cheese_node.addNode("cheddar", self.cheddar_node)

        # fallback:
        self.receiver.fallback = self.fallback

    # def wild_handler(self, message, address):
    #     print("  Got %s from %s" % (message, address))

    # def cheese_handler(self, message, address):
    #     """
    #     Method handler for /ping
    #     """
    #     print("cheese_handler")
    #     print("  Got %s from %s" % (message, address))

    # def cheese_cheddar_handler(self, message, address):
    #     """
    #     Method handler for /cheese/cheddar
    #     """
    #     print("cheese_cheddar_handler")
    #     print("  Got %s from %s" % (message, address))

    # def any_egg_handler(self, message, address):
    #     """
    #     Method handler for /*/egg
    #     """
    #     print("any_egg_handler")
    #     print("  Got %s from %s" % (message, address))

    def hue_handler(self, message, address):
        send_simple('H', message)

    def sat_handler(self, message, address):
        send_simple('S', message)

    def val_handler(self, message, address):
        send_simple('V', message)

    def speed_handler(self, message, address):
        send_simple('s', message)

    def mode_handler(self, message, address):
        # print dir(message)
        if (message.getValues()[0] > 0.5): # only do stuff on button push, not button release
            element_number = get_number(message)
            if element_number < len(modes):
                send_mode(modes[element_number])

        # print("  Got %s from %s" % (message, message))
        # send_simple('m', message)

    def fallback(self, message, address):
        """
        Fallback for any unhandled message
        """
        # print("Fallback:")
        # print dir(message)
        # print message.address
        print("  Got %s from %s" % (message, address))
        print message.getValues()
        arduino.write(chr(254)) # start
        for x in message.getValues():
            print x
            arduino.write(chr(int(x*250)))
        arduino.write(chr(255)) # stop

    # def ping_handler(self, message, address):
    #     """
    #     Method handler for /ping
    #     """
    #     print("ping_handler")
    #     print("  Got %s from %s" % (message, address))

    # def ham_egg_handler(self, message, address):
    #     """
    #     Method handler for /ham/egg
    #     """
    #     print("ham_egg_handler")
    #     print("  Got %s from %s" % (message, address))

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
    print 'm'
    print letter

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
    print letter
    print value_to_send

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
