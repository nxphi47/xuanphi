#!/usr/bin/env python2.7
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
	def __init__(self, sid, gpio, cfg):
		self.sid = sid			# sensor ID
		self.gpio = gpio		# GPIO control object
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
		
	# initialize the address bus for controlling this sensor
	def init_address(self):
		self.gpio._out('ALE', 1)	# disable ALE
		self.gpio._out('A0', self.A0)
		self.gpio._out('A1', self.A1)
		
	# perform ranging on this sensor
	#	idle_cb: callback when idle
	def do_ranging(self, idle_cb):
		# make sure we don't trigger ranging before T_cycle is up
		while True:
			if time.time() - self.t_lastrange >= self.T_cycle:
				break;
			idle_cb()
		# start ranging
		self.t_lastrange = time.time()
		self.gpio._out('A3', 0)
		self.gpio._out('A2', 1)		# A3=0, A2=1 : set RX on MB13xx
		self.gpio._out('ALE', 0)	# enable ALE
		# wait for autocalibration
		while True:
			if time.time() - self.t_lastrange >= self.T_range:
				break;
			idle_cb()
		# start sampling
		self.gpio._out('A2', 0)		# A3=0, A2=0 : Nothing
		self.gpio._out('A3', 1)		# A3=1, A2=0 : Start ADC conversion
		st = time.time()
		new_data = {
			'ts': comms.get_curr_timestamp(st),
			'samples': [],
		}
		while True:
			self.gpio._out('A2', 0)	# A3=1, A2=0 : Start ADC Conversion
			self.gpio.wait_not_busy()
			self.gpio._out('A2', 1) # A3=1, A2=1 : CS/RD on ADC latch
			new_data['samples'].append(self.gpio.read_data())
			if time.time() - st > self.T_sample:
				break
		# store the samples
		self.last_data = new_data
		# wait before triggering next sensor
		while True:
			if time.time() - self.t_lastrange >= self.T_next:
				break;
			idle_cb()

	# set timings
	def set_timing(self, T_range, T_sample, T_next, T_cycle):
		self.T_range = T_range * .001
		self.T_sample = T_sample * .001
		self.T_next = T_next * .001
		self.T_cycle = T_cycle * .001
		
