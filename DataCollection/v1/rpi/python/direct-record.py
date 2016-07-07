#!/usr/bin/env python2.7

import os
import sys
import time
import comms 
from gpio import RPiGPIO as RPiGPIO

config = {
	'port': 2929,
	'T-range': 17,				# time to wait after start of ranging before sampling (msec)
	'T-sample': 23,				# duration to perform sampling (msec)
	'T-next': 40,				# minimum time between successive ranging of two sensors
	'T-cycle': 100,				# minimum time between successive ranging of same sensor (hardware limits to minimum of 100msec)
	'num-sensors': 4,
}

# Sensor object for each sonar sensor
class Sensor(object):
	def __init__(self, sid, cfg):
		self.sid = sid			# sensor ID
		self.gpio = RPiGPIO()	# GPIO control object
		self.enabled = False	# Disable Sensor at init
		self.last_data = {
			'ts': 0,			# timestamp
			'samples': [],		# samples collected
		}
		# initialize addressing
		self.A0 = sid & 1
		self.A1 = (sid >> 1) & 1
		# initialize timing
		self.T_range = cfg['T-range'] * .001
		self.T_sample = cfg['T-sample'] * .001
		self.T_next = cfg['T-next'] * .001
		self.T_cycle = cfg['T-cycle'] * .001
		self.t_lastrange = 0.

	# wait until t_period since last time (t_last) has passed
	def _wait(self, t_last, t_period, sleep=True):
		if time.time() - t_last >= t_period:
			return False
		if sleep:
			t = t_period + t_last - time.time()
			if (t > 0.00001):
				sleep(t)
		return True
		
	# initialize the address bus for controlling this sensor
	def init_address(self):
		self.gpio._out('ALE', 1)	# disable ALE
		self.gpio._out('A0', self.A0)
		self.gpio._out('A1', self.A1)
		
	# perform ranging on this sensor
	def do_ranging(self):
		# make sure we don't trigger ranging before T_cycle is up
		while self._wait(self.t_lastrange, self.T_cycle):
			pass
		# start ranging
		self.t_lastrange = time.time()
		self.gpio._out('A3', 0)
		self.gpio._out('A2', 1)		# A3=0, A2=1 : set RX on MB13xx
		self.gpio._out('ALE', 0)	# enable ALE
		# wait for autocalibration
		while self._wait(self.t_lastrange, self.T_range):
			pass
		# start sampling
		self.gpio._out('A2', 0)		# A3=0, A2=0 : Nothing
		self.gpio._out('A3', 1)		# A3=1, A2=0 : Start ADC conversion
		st = time.time()
		new_data = {
			'ts': comms.get_curr_timestamp(st),
			'samples': [],
		}
		while aelf._wait(st, self.T_sample, sleep=False):
			self.gpio._out('A2', 0)	# A3=1, A2=0 : Start ADC Conversion
			while self.gpio._in('BUSY'):
				time.sleep(0.000002)	# 2 usec sleep
			# self.gpio.wait_not_busy()
			self.gpio._out('A2', 1) # A3=1, A2=1 : CS/RD on ADC latch
			new_data['samples'].append(self.gpio.read_data())
			
		self.gpio._out('ALE', 1)
		# store the samples
		self.last_data = new_data

	# set timings
	def set_timing(self, T_range, T_sample, T_next, T_cycle):
		self.T_range = T_range * .001
		self.T_sample = T_sample * .001
		self.T_next = T_next * .001
		self.T_cycle = T_cycle * .001
		
sen = Sensor(0, config)
sen.set_timing(15, 100, 100, 1000)
fp = open(time.strftime("%Y%m%d-%H%M%S") + ".data", "w")

for n in xrange(10):
	sen.init_address()
	sen.do_ranging()
	fp.write("%s: ts=%d\n - " % (time.strftime("%Y%m%d-%H%M%S"), sen.last_data['ts']))
	fp.write(" ".join("%d" % x for x in sen.last_data['samples']))
	fp.write("\n")
	
fp.close()
