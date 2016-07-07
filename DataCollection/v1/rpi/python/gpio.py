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
	'DBG': (21, GPIO.IN, GPIO.PUD_DOWN),
}

def callback(n):
	print "Event detected for %d" % n

class RPiGPIO(object):
	def __init__(self, use_event=False):
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
		self.datapins = []
		for d in ['D7', 'D6', 'D5', 'D4', 'D3', 'D2', 'D1', 'D0']:
			self.datapins.append(self.pin[d])
		# disable ALE
		GPIO.output(self.pin['ALE'], 1)
		# set up busy interrupt -- deprecated
		if use_event:
			GPIO.add_event_detect(self.pin['BUSY'], GPIO.BOTH, callback)
			#GPIO.add_event_detect(self.pin['BUSY'], GPIO.FALLING)
			GPIO.add_event_detect(self.pin['DBG'], GPIO.BOTH, callback)
	
	# simple test purpose output		
	def _out(self, p, v):
		GPIO.output(self.pin[p], v)

	# simple test purpose input
	def _in(self, p):
		return GPIO.input(self.pin[p])

	# simple test purpose output		
	def _outs(self, p, v):
		GPIO.output(self.pin[p], v)
		print "setting %s to %d" % (p, v)

	# simple test purpose input
	def _ins(self, p):
		v = GPIO.input(self.pin[p])
		print "%s => %d" % (p, v)
		return v
		
	# read in a byte from D0-D7
	def read_data(self):
		ret = 0
		for p in self.datapins:
			ret <<= 1
			if GPIO.input(p):
				ret |= 1
		return ret

	# wait for Busy - Deprecated
	def wait_not_busy_deprecated(self):
		GPIO.wait_for_edge(self.pin['BUSY'], GPIO.FALLING)
		
	# check for busy - Deprecated
	def wait_not_busy(self):
		while not GPIO.event_detected(self.pin['BUSY']):
			time.sleep(0.000002)		# 2usec sleep (ADC conversion time is max 4.5us)
		
def read_speed_test(gpio, n):
	st = time.time()
	for i in xrange(n):
		gpio._out('A2',0)
		# GPIO.wait_for_edge(gpio.pin['BUSY'], GPIO.FALLING)
		gpio._out('A2',1)
		gpio.read_dara()
	et = time.time()
	print "Start: %.2f   End: %.2f   Speed: %.2e bytes/s" % (st, et, 1.*n/(et-st))
		
def convert_and_read(gpio,busy=True):
	gpio._outs('ALE', 1)
	gpio._outs('A0', 0)
	gpio._outs('A1', 0)
	gpio._outs('A2', 0)
	gpio._outs('A3', 1)	# start conversion
	gpio._outs('ALE', 0)
	if busy:
		print "waiting for busy ..."
		gpio.wait_not_busy()
	else:
		time.sleep(0.1)
	gpio._outs('A2', 1) # RD/CS
	ret = 0
	for d in ['D7', 'D6', 'D5', 'D4', 'D3', 'D2', 'D1', 'D0']:
		ret <<= 1
		ret |= gpio._ins(d)
	print "data=%d" % ret	

def range_and_read(gpio,busy=True):
	gpio._outs('ALE', 1)
	gpio._outs('A0', 0)
	gpio._outs('A1', 0)
	gpio._outs('A2', 1)
	gpio._outs('A3', 0)	# start ranging
	gpio._outs('ALE', 0)
	time.sleep(0.017)
	t = time.time()
	while (time.time() - t < 0.04):
		gpio._outs('A2', 0)
		gpio._outs('A3', 1)	# start conversion
		while (gpio._ins('BUSY') == 1):
			time.sleep(0.0001)
		gpio._outs('A2', 1) # RD/CS
		ret = 0
		for d in ['D7', 'D6', 'D5', 'D4', 'D3', 'D2', 'D1', 'D0']:
			ret <<= 1
			ret |= gpio._ins(d)
		print "data=%d" % ret	

# testing purposes
if __name__ == "__main__":
	gpio = RPiGPIO(False)
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
		elif cmd[0] == 'convert-&-read':
			convert_and_read(gpio, True)
		elif cmd[0] == 'convert-then-read':
			convert_and_read(gpio, False)
		elif cmd[0] == 'range-then-read':
			range_and_read(gpio, False)
		else:
			print "Unknown command %s" % cmd[0]

