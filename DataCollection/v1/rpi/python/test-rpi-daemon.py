#!/bin/env python2

import time
from comms import CommsClient

class Daemon(object):
	def __init__(self, server):
		self.comms = CommsClient(server)
		self.sensors = []
		
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
		self.print_sensors()
		
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

	def main_loop(self):
		while True:
			cmd = raw_input("Enter Command: ").split(' ')
			if cmd[0] == "exit":
				return
			elif cmd[0] == 'read-data':
				for s in self.sensors:
					if s['state']:
						print "Sending READ ..."
						msg = self.comms.send_and_recv(('READ',))
						msg.debug("Received msg: ")
						self.sensors[msg.sid]['ts'] = msg.timestamp
						self.sensors[msg.sid]['data'] = msg.data[:]
			elif cmd[0] == 'get-state':
				msg = self.comms.send_and_recv(('GET-STATE',))
				msg.debug("Received GET-STATE resposne: ")
				for i in xrange(len(msg.states)):
					self.sensors[i]['state'] = msg.states[i]
			elif cmd[0] == 'set-state':
				if len(cmd) <= len(self.sensors):
					print "-- you need to specify all sensor states"
					continue
				states = []
				for i in xrange(len(self.sensors)):
					if cmd[i+1] == '1':
						states.append(1)
					else:
						states.append(0)
				msg = self.comms.send_and_recv(('SET-STATE',states))
				msg.debug("Received SET-STATE resposne: ")
				for i in xrange(len(msg.states)):
					self.sensors[i]['state'] = msg.states[i]
			elif cmd[0] == 'get-timing':
				msg = self.comms.send_and_recv(('GET-TIMING',))
				msg.debug("Received GET-TIMING resposne: ")
				self.t_range = msg.t_range
				self.t_sample = msg.t_sample
				self.t_next = msg.t_next
				self.t_cycle = msg.t_cycle
			elif cmd[0] == 'set-timing':
				if len(cmd) <= 4:
					print "-- you need to specify all timings"
					continue
				t_r = int(cmd[1])
				t_s = int(cmd[2])
				t_n = int(cmd[3])
				t_c = int(cmd[4])
				msg = self.comms.send_and_recv(('SET-TIMING',t_r,t_s,t_n,t_c))
				msg.debug("Received SET-TIMING resposne: ")
				self.t_range = msg.t_range
				self.t_sample = msg.t_sample
				self.t_next = msg.t_next
				self.t_cycle = msg.t_cycle
			else:
				print "Unknown command"
				continue
			self.print_sensors()

		print "Sending FINI ..."
		msg = self.comms.send_and_recv(('FINI',))
		msg.debug("Received msg: ")
		self.comms.close()
		
if __name__ == "__main__":
	client = Daemon(("127.0.0.1", 2929))
	client.connect()
	client.main_loop()
