#!/usr/bin/env python2.7

import os
import sys
import time
import RPi.GPIO as GPIO

GPIO_PIN_CFG = {
	'A0': (5, GPIO.OUT, None),
	'A1': (6, GPIO.OUT, None),
	'A2': (13, GPIO.OUT, None),
	'A3': (26, GPIO.OUT, None),
	'ALE': (19, GPIO.OUT, None),
	'D0': (4, GPIO.IN, GPIO.PUD_DOWN),
	'D1': (17, GPIO.IN, GPIO.PUD_DOWN),
	'D2': (18, GPIO.IN, GPIO.PUD_DOWN),
	'D3': (27, GPIO.IN, GPIO.PUD_DOWN),
	'D4': (22, GPIO.IN, GPIO.PUD_DOWN),
	'D5': (23, GPIO.IN, GPIO.PUD_DOWN),
	'D6': (24, GPIO.IN, GPIO.PUD_DOWN),
	'D7': (25, GPIO.IN, GPIO.PUD_DOWN),
	'BUSY': (12, GPIO.IN, GPIO.PUD_DOWN),
}

class RPiGPIO(object):
	def __init__(self):
		# setup the hardware
		self.pin = {}
		GPIO.setmode(GPIO.BCM)
		GPIO.setwarnings(False)
		for p in GPIO_PIN_CFG:
			(n,io,ud) = GPIO_PIN_CFG[p]
			self.pin[p] = n
			if io == GPIO.OUT:
				GPIO.setup(n, GPIO.OUT)
			else:
				GPIO.setup(n, GPIO.IN, pull_up_down=ud)
		# disable ALE
		GPIO.output(self.pin['ALE'], 1)
	
	# simple test purpose output		
	def _out(self, p, v):
		GPIO.output(self.pin[p], v)

	# simple test purpose input
	def _in(self, p):
		return GPIO.input(self.pin[p])
		
	# read in a byte from D0-D7
	def read_data(self):
		ret = 0
		for d in ['D7', 'D6', 'D5', 'D4', 'D3', 'D2', 'D1', 'D0']:
			ret <<= 1
			if GPIO.input(self.pin[d]):
				ret |= 1
		return ret
		
def read_speed_test(gpio, n):
	st = time.time()
	for i in xrange(n):
		gpio._out('A2',0)
		# GPIO.wait_for_edge(gpio.pin['BUSY'], GPIO.FALLING)
		gpio._out('A2',1)
		gpio.read_data()
	et = time.time()
	print "Start: %.2f   End: %.2f   Speed: %.2e bytes/s" % (st, et, 1.*n/(et-st))
		
# testing purposes
if __name__ == "__main__":
	gpio = RPiGPIO()
	while True:
		cmd = raw_input("Enter Command: ").split(' ')
		if cmd[0] == "exit":
			break
		elif cmd[0] == 'read':
			if len(cmd) < 2:
				print "read what?"
				continue
			if cmd[1] in ['D7', 'D6', 'D5', 'D4', 'D3', 'D2', 'D1', 'D0', 'BUSY']:
				print "%s = %d" % (cmd[1], gpio._in(cmd[1]))
			elif cmd[1] == 'DATA':
				print "%s = %d" % (cmd[1], gpio.read_data())
			else:
				print "pin '%s' is either unknown or not configured as input" % cmd[1]
		elif cmd[0] == 'set':
			if len(cmd) < 3:
				print "set what?"
				continue
			if cmd[1] in ['A3', 'A2', 'A1', 'A0', 'ALE']:
				if cmd[2] in ['1', 'high', 'H', 'true']:
					gpio._out(cmd[1], 1)
					print "set %s to 1" % cmd[1]
				else:
					gpio._out(cmd[1], 0)
					print "set %s to 0" % cmd[1]
		elif cmd[0] == 'speed':
			read_speed_test(gpio, int(cmd[1]))
		else:
			print "Unknown command %s" % cmd[0]

