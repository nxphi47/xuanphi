#!/bin/env python2

import select
import socket
import sys
import time

# Specify server
server_address = ('10.80.43.50', 4270)

# what message to send
msgs = [
	'GETCFG',
	'SETCFG T-BURST=800 T-SAMPLE=48 N-SAMPLES=1000 UNKNOWN=20',
	'GETCFG',
	'START',
	'',
	'STOP',
]

config = {}

sample = {
	'meta': {},
	'timing': [],
	'envelop': [],
	'last-data': -1
}

# convert the received sample into JSON and output to <outf>
def sample_2_json (outf):
	outf.write('{ "meta": { "setup": %d, "num": %d },\n' % (sample['meta']['SETUP'], sample['meta']['NUM']))
	outf.write('  "timing": [\n\t')
	for i,t in enumerate(sample['timing']):
		if (i==0):
			outf.write(' %6.3f' % (t/1000.))
		elif (i%10 == 0):
			outf.write(',\n\t %6.3f'  % (t/1000.))
		else:
			outf.write(', %6.3f'  % (t/1000.))
	outf.write(' ],\n  "envelop": [\n\t')
	for i,v in enumerate(sample['envelop']):
		if (i==0):
			outf.write(' %4d' % v)
		elif (i%10 == 0):
			outf.write(',\n\t %4d'  % v)
		else:
			outf.write(', %4d'  % v)
	outf.write(' ]\n}\n')

# split the received message into a dictionary of name-value pair
# If value should be integer, set <isInt> to True
def resp2dict (msg, isInt=False):
	# initialize return dictionary
	d = {}
	# remove trailing NULL byte
	if msg[-1] == '\x00':
		msg = msg[:-1]
	# split into space-separated fields and ignore the first word
	fields = msg.split(' ')[1:]
	# print "DEBUG: fields=%s" % repr(fields)
	for f in fields:
		# take care of terminating NULL byte
		if (f == '\x00') or (f == ''):
			continue
		if f[-1] == '\x00':
			f = f[:-1]
		# split into name value pair
		nvpair = f.split('=')
		if isInt:
			d[nvpair[0]] = int(nvpair[1])
		else:
			d[nvpair[0]] = nvpair[1]
	return d

def handle_response (msg):
	# check what type of message
	if msg[0:6] == 'CONFIG':
		# configuration data
		config = resp2dict(msg)
		print "Current config: %s" % repr(config)
	elif msg[0:6] == 'SAMPLE':
		# initialize sample 
		sample['meta'] = resp2dict(msg, isInt=True)
		sample['timing'] = []
		sample['envelop'] = []
		sample['last-data'] = -1
	elif msg[0:5] == 'DATA-':
		# data segment -- extract the segment index
		idx = int(msg[5:7])
		if (idx != sample['last-data'] + 1):
			print "*** Warning: received out-of-sequence data %d" % idx
		sample['last-data'] = idx
		# extract binary data
		data = msg[8:]
		i=0
		while (i < len(data) - 3):
			# first 2 bytes is the timing, next 2 bytes is the envelop
			t = (ord(data[i]) << 8) + ord(data[i+1])
			v = (ord(data[i+2]) << 8) + ord(data[i+3])
			# correct overflow in timing since we only allow 16bits in timing
			while (len(sample['timing']) > 1) and (t < sample['timing'][-1]):
				t += 0x10000
			sample['timing'].append(t)
			sample['envelop'].append(v)
			i += 4
		if (i != len(data)):
			print "*** Warning: received byte stream is not 32-bit aligned."
		# check if we have received last DATA-segment
		if sample['meta'].has_key('NUM'):
			if len(sample['timing']) >= sample['meta']['NUM']:
				# yes, now we just print it out
				sample_2_json(sys.stdout)

# Create a UDP socket
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

m=0
st = time.time()
while True:
	rfds, wfds, efds = select.select([sock, sys.stdin], [], [], 3.0)
	if sock in rfds:
		msg, server = sock.recvfrom(4096)
		print "*** received %s response: %s" % (repr(server), repr(msg))
		handle_response(msg)
	if sys.stdin in rfds:
		ignore = sys.stdin.readline()
		break
	if (time.time() - st > 3.):
		if msgs[m] != '':
			print "*** sending msg: %s" % msgs[m]
			sock.sendto(msgs[m]+'\0', server_address)
		m += 1
		if (m >= len(msgs)):
			m = 0
		st = time.time()

print "*** sending msg: QUIT"
sock.sendto('QUIT\0', server_address)
sock.close()

		
