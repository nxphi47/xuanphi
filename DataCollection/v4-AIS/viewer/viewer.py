#!/bin/env python2

import select
import socket
import sys
import time

# Specify server
#server_address = ('10.80.43.112', 4270)
server_address = ('', 4270)		# use '' to automatically get server address (by broadcast)

# Sensor Configuration Parameter
# config = 'T-BURST=100 T-SAMPLE=50 N-SAMPLES=800';
rpi_config = {
	'SETUP': 0,			# MeasurementSetup ID to use
	'T-BURST': 100, 	# msecs between burst
	'T-SAMPLE': 50,		# usecs between each ADC sample
	'N-SAMPLES': 800,	# number of samples to collect for each burst
	'T-WAIT': 15000,	# usecs to wait after sending 'StartMeasurement' command and start of ADC sampling
}

# temporary storage of collected samples
samples = []

def usage():
	print "%s [-d] [-1] out=<json-filename> [rpi=<IP-address>] [port=<Port-number>]" % sys.argv[0]
	print "[-1] if specified, after successfully stopping the ranging, the program will quit"
	print "[-d] if specified, the <json-filename> will be appended with datetime in 'YYYYMMDD-HHMM' format"
	print 
	exit (0)
	
# append datetime prefix to a filename
def append_dt (fname):
	dt = time.strftime("%Y%m%d-%H%M%S-")
	dp = fname.rfind('/')
	return fname[:dp+1] + dt +fname[dp+1:]

# convert the received sample into JSON and output to <outf>
def sample_2_json (sample, outf, isJS=False):
	if isJS:
		endc = '\\\n'
	else:
		endc = '\n'
	if sample['meta'].has_key('START'):
		timestr = ', "start": "%.3f" ' % (sample['meta']['START'] /1000000.)
	else:
		timestr = ' '
	outf.write('{ "meta": { "setup": %d, "num": %d %s},%s' % (sample['meta']['SETUP'], sample['meta']['NUM'], timestr, endc))
	outf.write('  "timing": [%s\t' % endc)
	for i,t in enumerate(sample['timing']):
		if (i==0):
			outf.write(' %6.3f' % (t/1000.))
		elif (i%10 == 0):
			outf.write(',%s\t %6.3f'  % (endc, t/1000.))
		else:
			outf.write(', %6.3f'  % (t/1000.))
	outf.write(' ],%s  "envelop": [%s\t' % (endc, endc))
	for i,v in enumerate(sample['envelop']):
		if (i==0):
			outf.write(' %4d' % v)
		elif (i%10 == 0):
			outf.write(',%s\t %4d'  % (endc, v))
		else:
			outf.write(', %4d'  % v)
	outf.write(' ]%s}' % endc)
	
# Save array of JSON
def save_samples (fname, isJS=False):
	if (not isJS) and (not fname.endswith('.json')):
		fname = fname + '.json'
	with open(fname, 'w') as f:
		if isJS:
			f.write("var json_string = '")
		f.write("[ ")
		first = True
		for s in samples:
			if not first:
				if isJS:
					f.write(", \\\n")
				else:
					f.write(", \n")
			else:
				first = False
			sample_2_json(s, f, isJS)
		f.write(']')
		if isJS:
			f.write("';\n")
	print "%s saved." % fname

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

currsample = {}
rx_config = {}
rs_status = {}

def handle_response (msg):
	global currsample
	global rx_config
	global rx_status
	
	# check what type of message
	if msg[0:6] == 'CONFIG':
		# configuration data
		rx_config = resp2dict(msg, isInt=True)
		print "Current config: %s" % repr(rx_config)
	elif msg[0:6] == 'STATUS':
		rx_status = resp2dict(msg, isInt=False)
		print "HW Status: %s" % repr(rx_status)
	elif msg[0:5] == 'HW-DBG':
		print "HW-DBG: %s" % msg
	elif msg[0:6] == 'SAMPLE':
		# initialize sample 
		currsample = {}
		currsample['meta'] = resp2dict(msg, isInt=True)
		currsample['timing'] = []
		currsample['envelop'] = []
		currsample['last-data'] = -1
	elif msg[0:5] == 'DATA-':
		# data segment -- extract the segment index
		idx = int(msg[5:7])
		if (idx != currsample['last-data'] + 1):
			print "*** Warning: received out-of-sequence data %d" % idx
		currsample['last-data'] = idx
		# extract binary data
		data = msg[8:]
		i=0
		while (i < len(data) - 3):
			# first 2 bytes is the timing, next 2 bytes is the envelop
			t = (ord(data[i]) << 8) + ord(data[i+1])
			v = (ord(data[i+2]) << 8) + ord(data[i+3])
			# correct overflow in timing since we only allow 16bits in timing
			while (len(currsample['timing']) > 1) and (t < currsample['timing'][-1]):
				t += 0x10000
			currsample['timing'].append(t)
			currsample['envelop'].append(v)
			i += 4
		if (i != len(data)):
			print "*** Warning: received byte stream is not 32-bit aligned."
		# check if we have received last DATA-segment
		if currsample['meta'].has_key('NUM'):
			if len(currsample['timing']) >= currsample['meta']['NUM']:
				samples.append(currsample)
				sample_2_json(currsample, sys.stdout)
				sys.stdout.write('\n');

# Function to keep sending configuration until the received configuration is consistent
def send_config(sock):
	global rx_config
	
	# prepare config
	setcfg = 'SETCFG'
	for k,v in rpi_config.iteritems():
		setcfg += ' %s=%d' % (k, v)
	# send and make sure it is really registered
	while True:
		sock.sendto(setcfg + '\0', server_address)
		sock.sendto('GETCFG\0', server_address)
		rx_config = {}
		# select
		rfds, wfds, efds = select.select([sock], [], [], 0.2)
		if sock in rfds:
			msg, server = sock.recvfrom(4096)
			handle_response(msg)
			# check config is consistent
			for k,v in rpi_config.iteritems():
				if not rx_config.has_key(k):
					break
				if rx_config[k] != v:
					break
			else:
				# no break, consistent!
				break # break out of while loop
	print "*** Configuration set in RPi: %s" % setcfg[7:]
	
# Function to keep sending the specified message until:
#	if nocomms=True: no more comms
#	if nocomms=False: received something
def send_until (sock, smsg, nocomms=True):
	while True:
		sock.sendto(smsg, server_address)
		# select
		rfds, wfds, efds = select.select([sock], [], [], 0.2)
		if sock in rfds:
			rmsg, server = sock.recvfrom(4096)
			handle_response(rmsg)
			if not nocomms:
				return
		else:
			if nocomms:
				break
				
# Function will keep sending GETSTATUS until receives a response
def determine_server(sock):
	global server_address
	
	smsg = 'GETSTATUS '
	if hasattr(socket, 'SO_BROADCAST'):
		sock.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
	while True:
		sock.sendto(smsg + '\0', ('<broadcast>', server_address[1]))
		# select
		rfds, wfds, efds = select.select([sock], [], [], 0.2)
		if sock in rfds:
			msg, server = sock.recvfrom(4096)
			if msg[0:6] != 'STATUS':
				continue
			handle_response(msg)
			server_address = (server[0], server_address[1])
			print '"R-Pi Address: ', server_address
			print '"Client Address: ', server_address
			break

if __name__ == "__main__":
	
	# initialize paramaters
	createJS = 'samples.js'
	savefname = ''
	oneShot = False
	dtPrefix = False

	# handle input argument
	if len(sys.argv) > 1:
		for arg in sys.argv[1:]:
			if arg == '-1':
				oneShot = True
				print "*** Program will exit after one ranging session"
			elif arg == '-d':
				dtPrefix = True
				print "*** Saved filename will contain 'YYYYMMDD-HHMMSS' as prefix"
			elif arg[:4] == 'out=':
				savefname = arg[4:]
				print "*** Saved filename will be %s" % savefname
			elif arg[:3] == 'js=':
				createJS = arg[3:]
				print "*** Will create %s for javascript display when saving"
			elif arg[:4] == 'rpi=':
				server_address = (arg[4:], server_address[1])	
			elif arg[:5] == 'port=':
				server_address = (server_address[0], int(arg[5:]))
			elif arg == 'help':
				usage()
				
	# Create a UDP socket
	sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

	# if server is address is unknown
	if server_address[0] == '':
		determine_server(sock)
	else:
		sock.sendto('GETSTATUS\0', server_address)

	# Send configuration parameter
	send_config(sock)

	# main loop
	isRanging = False
	while True:
		# select
		rfds, wfds, efds = select.select([sock, sys.stdin], [], [], 3.0)
		
		if sock in rfds:
			# handle received message
			msg, server = sock.recvfrom(4096)
			# print "*** received %s response: %s" % (repr(server), repr(msg))
			handle_response(msg)
			
		if sys.stdin in rfds:
			# handle user input
			cmd = sys.stdin.readline().strip()		
			if cmd == '':
				cmd = '\n'

			if cmd[0] == '\x1b':
				# exit viewer but not raspberry pi
				if isRanging:
					send_until(sock, 'STOP\0', nocomms=True)
				print "Exiting (Pi still running)"
				break

			elif cmd[0] in 'Cc':
				# resend Configuration setting
				send_config(sock)

			elif cmd[0] in 'Gg':
				# resend Get Status
				sock.sendto('GETSTATUS\0', server_address)

			elif cmd[0] in 'Qq':
				# get raspberry pi to quit as well
				print "\nThis will cause raspberry pi to quit as well.  Are you sure?"
				ans = sys.stdin.readline().strip()
				if ans[0] in 'Yy':
					print "*** sending msg: QUIT"
					send_until(sock, 'QUIT\0', nocomms=True)
					print "Quiting (Pi will stop as well)"
				else:
					print "*** sending msg: STOP"
					send_until(sock, 'STOP\0', nocomms=True)
					print "Exiting (Pi still running)"
				break

			elif cmd[0] in 'Ss':
				# Save
				filename = ''
				if ' ' in cmd:
					[ignore, filename] = cmd.split(' ', 1)
				if filename == '':
					filename = savefname
				if dtPrefix:
					filename = append_dt(filename)
				if filename != '':
					save_samples(filename, isJS=False)
				if createJS != '':
					save_samples(createJS, isJS=True)
				# clear stored samples	
				samples = []
				
			elif cmd[0] in 'H':
				if hwdbg:
					print "Exiting HW-DEBUG mode ..."
					send_unti(sock, 'HWDBG-OFF\0', nocomms=False)
					hwdbg = False
				elif isRanging:
					print "Refusing to enter HW-DBG Mode while ranging."
				else:
					print "Enterting HW-DEBUG mode ..."
					send_unti(sock, 'HWDBG-ON\0', nocomms=False)
					hwdbg = True
			else:
				# any other keys is assumed to be toggling the ranging
				if isRanging:
					print "*** stop ranging"
					send_until(sock, 'STOP\0', nocomms=True)
					isRanging = False
					if oneShot:
						break
				elif hwdbg:
					# we are in hardware debug mode
					# send whatever user type to R-Pi
					sock.sendto('HWDBG-SET %s\0', cmd)
				else:
					print "*** start ranging"
					send_until(sock, 'START\0', nocomms=False)
					isRanging = True

	sock.close()
	if len(samples) > 0:
		if dtPrefix:
			savefname = append_dt(savefname)
		if savefname != '':
			save_samples(savefname, isJS=False)
		save_samples(createJS, isJS=True)
	else:
		print "No samples captured."

