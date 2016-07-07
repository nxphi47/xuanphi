#!/usr/bin/env python2

import os
import sys
import time
import datetime
import thread
import json
import random
import comms
import commshub

MOCKUP = True

# ------------------------------------------------
# Global Configuration
# ------------------------------------------------

config = { 
	'socket-port': 6557, 
	'http-port': 6551, 
	'data-path': os.path.join(os.path.dirname(os.path.realpath(__file__)), "data"),
	'rpi-address': ("127.0.0.1", 2929),
}

def parse_ajax_req (cmd):
	res = { }
	for nv in cmd.split('/'):
		if '=' in nv:
			nvp = nv.split('=')
			res[nvp[0].strip()] = nvp[1].strip()
		else:
			res[nv.strip()] = ''
	return res

def list2json(x):
	r = '['
	for i in x:
		r += str(i) + ","
	return r[:-1] + ']'

def mockData():
	res = [ 0 ] * 480
	for i in xrange(480):
		res[i] = int(random.random() * 256)
	return res

# --------------------------------------------------
# Method to handle AJAX: UPDATE
# --------------------------------------------------
def handle_update (csock, cmd):
	global rpiC
	print "*** CGI:UPDATE received ***"
	
	rpiC.update_data()
	
	res = ''
	for i in xrange(len(rpiC.sensors)):
		if rpiC.sensors[i]['state']:
			res += "sensors[%d].update(%s);" % (i, list2json(rpiC.sensors[i]['data']))
	return res


class RPiHandler(object):
	def __init__(self, server):
		self.comms = comms.CommsClient(server)
		if not MOCKUP:
			self.sensors = []
		else:
			self.sensors = [
				{ 'state': True, 'ts': 0, 'data': [], },
				{ 'state': False, 'ts': 0, 'data': [], },
				{ 'state': True, 'ts': 0, 'data': [], },
				{ 'state': False, 'ts': 0, 'data': [], },
			]
	
	def connect(self):
		self.comms.connect()
		msg = self.comms.send_and_recv(('INIT',))
		msg.debug("Received INIT resposne: ")
		msg = self.comms.send_and_recv(('GET-STATE',))
		msg.debug("Received GET-STATE resposne: ")
		self.sensors = []
		for i in xrange(len(msg.states)):
			self.sensors.append({
				'state': msg.states[i],
				'ts': 0,
				'data': []
			})
		self.t_range = 0
		self.t_sample = 0
		self.t_next = 0
		self.t_cycle = 0
		
	def print_sensors(self):
		print "-- Timings: range=%d, sample=%d, next=%d, cycle=%d" % (self.t_range, self.t_sample, self.t_next, self.t_cycle)
		for s in self.sensors:
			txt = "-- Sensor[%d]: " % self.sensors.index(s)
			if s['state'] != 0:
				txt += 'enabled'
			else:
				txt += 'disabled'
			print txt
			if len(s['data']) == 0:
				print "  -- no data"
				continue
			print "  -- last-data-timestamp: %d" % s['ts']
			i = 0
			for d in s['data']:
				if i==0:
					txt = "  -- "
				txt += "[%03d]" % d
				i += 1
				if i == 10:
					print txt
					i = 0
			if i > 0:
				print txt
				
	def update_state(self):
		msg = self.comms.send_and_recv(('GET-STATE',))
		msg.debug("Received GET-STATE resposne: ")
		for i in xrange(len(msg.states)):
			self.sensors[i]['state'] = msg.states[i]
			
	def update_data(self):
		for s in self.sensors:
			if s['state']:
				if not MOCKUP:
					print "Sending READ ..."
					msg = self.comms.send_and_recv(('READ',))
					msg.debug("Received msg: ")
					self.sensors[msg.sid]['ts'] = msg.timestamp
					self.sensors[msg.sid]['data'] = msg.data[:]
				else:
					s['data'] = mockData()

	def set_states(self, states):
		msg = self.comms.send_and_recv(('SET-STATE',states))
		msg.debug("Received SET-STATE resposne: ")
		for i in xrange(len(msg.states)):
			self.sensors[i]['state'] = msg.states[i]

	def get_timings(self, states):
		msg = self.comms.send_and_recv(('GET-TIMING',))
		msg.debug("Received GET-TIMING resposne: ")
		self.t_range = msg.t_range
		self.t_sample = msg.t_sample
		self.t_next = msg.t_next
		self.t_cycle = msg.t_cycle

	def set_timings(self):
		msg = self.comms.send_and_recv(('SET-TIMING', self.t_range, self.t_sample, self.t_next, self.t_cycle))
		msg.debug("Received SET-TIMING resposne: ")
		self.t_range = msg.t_range
		self.t_sample = msg.t_sample
		self.t_next = msg.t_next
		self.t_cycle = msg.t_cycle
		
	
if __name__ == "__main__":
	# set up Comms
	sock_handlers = { }
	sock_handlers['CGI:UPDATE'] = handle_update
	stdin_handlers = { }
	chub = commshub.CommsHub (config, sock_handlers, stdin_handlers)
	
	rpiC = RPiHandler(config['rpi-address'])
	
	if not MOCKUP:
		rpiC.connect()
		rpiC.set_states([1,0,0,0])000
	
	# run main loop
	chub.run()
	
