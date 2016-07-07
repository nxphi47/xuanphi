import os
import commshub
import json
import time
import random
import threading
import platform
import subprocess
import socket
import select

if 'Linux' in platform.system():
	if not os.path.isdir('code'):
		subprocess.call(["ln -s .. code"], shell=True)

	from code.processJson import JSONProcessor
	from code.demolib import feature_extract
	from code.demolib import svm_classify
else:
	subprocess.call(["move ..\*.* .\ "], shell=True)

	from processJson import JSONProcessor
	from demolib import feature_extract
	from demolib import svm_classify

SMOOTH = 3
DEBUG = 0		# 0 if actual run, 1 if debug mode (sending dummy data)
RASPI = False	# True if running test-sensor.py at client side
AIS = True		# true if using AIS sensor, False if using PRDCSG sensor

__STATUS__ = 'Burst'
__RESULT__ = 'STATUS=Burst'
__OBST__ = 'None'
__HEIGHT__	= 'NA'
__DIST__ = 'NA'

sample = {
	'meta': {},
	'timing': [],
	'envelop': [],
	'last-data': -1
}

# -----------------------------
#   Global Configuration
# -----------------------------
config = {
	'http-port': 5551,      # socket number to communicate with CGI
	'socket-port': 5557,    # socket number to communicate with code perform control
	'udp-port': 4270,		# socket number for UDP communication
	'svm-dir': 'svm',        # directory to store svm file
	'json-dir': 'json',      # directory to store json file
	#'raspi-addr': '10.80.43.127',   # IP address for RasPi which connected with sensor
	'raspi-addr': '',		# if using broadcast to obtain RasPi address
}

setting = {
	"SVM": 'train.svm',
	"FV": ["peakValue", "totalArea", "peakTime", "refTimeWidth-immed", "areaRatio"],
	"T-WAIT": "15000", 
	"SETUP": "0", 
	"T-SAMPLE": "50", 
	"T-BURST": "60", 
	"N-SAMPLES": "800",
	"VSPEED": "10"
}

dict = {
	'status': 'Burst',
	'obst': 'None',
	'height': 'NA',
	'dist': 'NA',
	'det': []
}


# Convenience: config or default value
def get_cfg(cfg, k, default):
	if cfg.has_key(k):
		if not cfg[k] is None:
			return cfg[k]
		else:
			return default

# Convenience: routines socket caller
def sock_read(s):
	msg = ''
	while True:
		c = s.recv(1)
		if c == '':
			break
		if c == '\n':
			break
		msg = msg + c
	return msg

def sock_write(s, msg):
	l = 0
	while l < len(msg):
		r = s.send(msg[l:])
		if r == 0:
			return -1
		l = l + r
	s.send('\n')
	return 0

def udp_sock(msg):
	sock = socket.socket (socket.AF_INET, socket.SOCK_STREAM)
	sock.connect (("localhost", 5558))
	sock_write (sock, msg)

# -----------------------------
#  Main Class to run demo daemon
# -----------------------------
class Daemon (object):
	def __init__(self, cfg):
		self.mode = 'init'
		self.cfg = cfg
		self.RasPi = None
		sock_handlers = {}
		sock_handlers['REGISTER'] = self.handle_register
		sock_handlers['CGI:START'] = self.handle_start
		sock_handlers['CGI:STOP'] = self.handle_stop
		sock_handlers['CGI:UPDATE'] = self.handle_update
		sock_handlers['CGI:GETCFG'] = self.handle_getcfg
		sock_handlers['CGI:SETCFG'] = self.handle_configure
		sock_handlers['START-OK'] = self.handle_start_ok
		sock_handlers['MATRIX'] = self.handle_matrix
		sock_handlers['UDP'] = self.handle_udp_msg
		stdin_handlers = { }
		
		self.loadSVM = None
		self.udp_port = get_cfg(self.cfg, 'udp-port', 4270)
		self.raspi_addr = get_cfg(self.cfg, 'raspi-addr', '')
		if len(self.raspi_addr) < 1:
			self.determine_server()
			config['raspi-addr'] = self.raspi_addr
		self.chub = commshub.CommsHub(self.cfg, sock_handlers, stdin_handlers)
		
		# prepare output directory
		self.jsondir = get_cfg(self.cfg, 'json-dir', 'json')
		if not os.path.isdir(self.jsondir):
			os.makedirs(self.jsondir)
		self.svmdir = get_cfg(self.cfg, 'svm-dir', 'svm')
		if not os.path.isdir(self.svmdir):
			os.makedirs(self.svmdir)
		
		self.config_params = 'SETUP=' + setting["SETUP"] + ' T-BURST=' + setting["T-BURST"] + ' T-SAMPLE=' + setting["T-SAMPLE"] + ' N-SAMPLES=' + setting["N-SAMPLES"] + ' T-WAIT=' + setting["T-WAIT"] + ' '
		self.status = __STATUS__
		self.result = __RESULT__
		self.obst = __OBST__
		self.height = __HEIGHT__
		self.dist = __DIST__
		self.matrix = []
		self.init_cfg()

	def run(self):
		self.chub.run()

	# determine RasPi address
	# Function will keep sending GETSTATUS until receives a response
	def determine_server(self):
		sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
		smsg = 'GETSTATUS '
		if hasattr(socket, 'SO_BROADCAST'):
			sock.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
		while True:
			sock.sendto(smsg + '\0', ('<broadcast>', self.udp_port))
			# select
			rfds, wfds, efds = select.select([sock], [], [], 0.2)
			if sock in rfds:
				msg, server = sock.recvfrom(4096)
				if msg[0:6] != 'STATUS':
					continue
				self.handle_udp_msg(sock, msg)
				self.raspi_addr = server[0]
				print '"Client Address: ', self.raspi_addr
				break
		sock.close()

	def init_cfg(self):
		sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
		sock.connect ((self.raspi_addr, self.udp_port))
		sock_write (sock, 'SETCFG ' + self.config_params)

	# handle reset msg, clear array, memory and empty directory
	def handle_reset(self, csock, cmd):
		print '*** RESET received ***'
		if not self.loadSVM is None:
			self.loadSVM = None
		return "stateMachine.ajax_response('RESET', '');"

	# handle register msg from raspberryPi
	def handle_register(self, csock, cmd):
		self.mode = 'idle'
		print '*** REGISTER received ***'
		if RASPI:
			self.RasPi = {'sock': csock}
		self.chub.noautoclose.append(csock) # tell chub not to auto-close this socket

	def handle_getcfg(self, csock, cmd):
		self.chub.udp_send('GETCFG')
		time.sleep(0.2)
		
		return "stateMachine.ajax_response('CONFIG', '%s')" % json.dumps(self.config_params)
		
	# handle configuration
	def handle_configure(self, csock, cmd):
		print '*** CONFIGURE received - %s***' % cmd
		
		_cfg = 'SETCFG '
		_params = eval(cmd[11:])
		for key, value in _params.iteritems():
			setting[key] = value
			if key != 'SVM' and key != 'FV':
				_cfg = _cfg + key + '=' + value + ' '

		self.chub.udp_send(_cfg)
		
		return "stateMachine.ajax_response('CONFIGURE-OK', '')"

	# sending start command to raspberryPi to start ranging
	def handle_start(self, csock, cmd):
		print '*** START received - %s***' % cmd

		if 'NONREAL' in cmd:
			self.mode = 'start-nonreal'
			if RASPI:
				sock_write(self.RasPi['sock'], "START-NONREAL:" + _params)
			else:
				if 'Linux' in platform.system():
					_port = get_cfg(self.cfg, 'udp-port', 4270)
					self.output = 'demo.json'
					self.json_file = os.path.join(os.getcwd(), self.jsondir) + '/' + self.output
										
					def nonreal_cmd():
						print 'send non real time command'
						subprocess.call(["python viewer-nonreal.py out=" + self.json_file + " rpi=" + self.raspi_addr + " port=" + str(self.udp_port) ], shell=True)
											
					_nonreal = threading.Thread(target=nonreal_cmd)
					_nonreal.start()
					
					time.sleep(0.1)
					# Connect to server first, and register ourselves as SENSORS
					udp_sock("START")
					
					return "stateMachine.ajax_response('START-NONREAL', '');"
				else:
					return "stateMachine.ajax_response('START-ERROR', '');"
		else:
			self.mode = 'start-real'
			self.real_json = JSONProcessor({}, {}, os.path.join(os.getcwd(), self.jsondir))
			if self.real_json is None:
				return "stateMachine.ajax_response('ERROR', '');"
			if RASPI:
				sock_write(self.RasPi['sock'], "START-REAL:" + _params)
			else:
				self.chub.udp_send("START")
			return "stateMachine.ajax_response('START-REAL', '');"

	# handle msg when raspPi start recording 
	def handle_start_ok(self, csock, cmd):
		print '*** START-OK received - %s***' % cmd
		cmd_split = cmd.split(':')
		for x in cmd_split:
			if 'JSON' in x:
				self.json_file = x.split('=')[1]
		print 'JSON File: ' + self.json_file
	
	# handle real time information update
	def handle_update(self, csock, cmd):
		print '*** UPDATE received ***'
		result = []
		if RASPI:
			sock_write(self.RasPi['sock'], 'UPDATE:')
		
		if DEBUG == 0:
			if len(self.matrix) == SMOOTH:
				self.perform_classification(REALTIME=True)
		
		ajaxList = []
		for key, value in dict.iteritems():
			ajaxList.append(key)
			if DEBUG == 0:
				ajaxList.append(value)
			elif DEBUG == 1:
				if 'obst' in key:
					k = random.randint(0, 2)
					if k == 0:
						ajaxList.append('Curb')
					elif k == 1:
						ajaxList.append('Curb')
					else:
						ajaxList.append('Unknown')
				elif 'status' in key:
					k = random.randint(0,1)
					if k == 0:
						ajaxList.append('Burst')
					else:
						ajaxList.append('Measuring')
				elif 'dist' in key:
					k = random.uniform(2, 4)
					ajaxList.append(str('%.2f' % k))
				else:
					k = random.randint(1,20)
					ajaxList.append(str(k))			
		
		return "stateMachine.ajax_response('UPDATE', '%s');" % json.dumps(ajaxList)

# split the received message into a dictionary of name-value pair
	# If value should be integer, set <isInt> to True
	def resp2dict (self, msg, isInt=False):
		# initialize return dictionary
		d = {}
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
	
	def handle_udp_msg (self, csock, msg):
		# remove trailing NULL byte
		if msg[-1] == '\x00':
			msg = msg[:-1]
		# check what type of message
		if msg[0:6] == 'CONFIG':
			# configuration data
			self.config_params = self.resp2dict(msg)		
			print "Current config: %s" % repr(self.config_params)
		elif msg[0:6] == 'STATUS':
			rx_status = self.resp2dict(msg, isInt=False)
			print "HW Status: %s" % repr(rx_status)
		elif msg[0:6] == 'SAMPLE':
			# initialize sample 
			sample['meta'] = self.resp2dict(msg, isInt=True)
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
				sample['timing'].append(float("%6.3f" % (t/1000.)))
				sample['envelop'].append(v)
				i += 4
			if (i != len(data)):
				print "*** Warning: received byte stream is not 32-bit aligned."
			# check if we have received last DATA-segment
			if sample['meta'].has_key('NUM'):
				# convert timing format
				if len(sample['timing']) >= sample['meta']['NUM']:
					# yes, now we just print it out
					print "calling handler: MATRIX"
					res = self.handle_matrix(None, sample)
		return
	
	# handle real time matrix received from raspi
	def handle_matrix(self, csock, cmd):
		print '*** Matrix received ***'
		
		self.matrix.append(cmd)
		
		if len(self.matrix) > SMOOTH:
			self.matrix.pop(0)
	
	# handle stop operation
	def handle_stop(self, csock, cmd):
		print '*** STOP received ***'
		if self.mode == 'start-nonreal':
			if RASPI:
				sock_write(self.RasPi['sock'], 'STOP:')
				
			ajaxList = []
			print 'send stop'
			udp_sock("STOP")
			print 'send save'
			udp_sock("SAVE")
			self.perform_classification(REALTIME=False)
			for key, value in dict.iteritems():
				ajaxList.append(key)
				ajaxList.append(value)
			return "stateMachine.ajax_response('STOP', '%s');" % json.dumps(ajaxList)
		else:
			# stop real
			if RASPI:
				sock_write(self.RasPi['sock'], 'STOP-REAL:')
			else:
				self.chub.udp_send("STOP")
			return "stateMachine.ajax_response('STOP-REAL', '');"
	
	# defined classification result and update dictionary	
	def perform_classification(self, MATRIX=None, REALTIME=True):
		if not REALTIME:
			time.sleep(6)
			print 'Processing JSON: ' + self.json_file
			# extract all feature
			_json = JSONProcessor({}, {}, os.path.dirname(self.json_file))
			if AIS:
				data = _json.load_AIS_file(self.json_file)
			else:
				data = _json.load_file(self.json_file)
			pdata = []
			for p in data:
				pdata.append(json.dumps(p))
			
			# execute feature
			_ext_fv = feature_extract(pdata, setting['FV'], SMOOTH)
			_fv, _dist = _ext_fv.run(REALTIME=False)

			# perform svm
			if len(_fv) > 0 and len(_dist) > 0:
				_svm = svm_classify(os.path.join(self.svmdir, setting['SVM']), _fv, _dist)
				self.obst, self.height, self.details = _svm.run()
				self.dist = float("{0:.2f}".format(sum(_dist) / float(len(_dist))))
				dict['det'] = self.details
		else:
			if AIS:
				data = self.real_json.load_AIS_data(self.matrix)
			else:
				data = self.real_json.load_data(self.matrix)
			pdata = []
			for p in data:
				pdata.append(p)
			
			# execute feature
			_ext_fv = feature_extract(pdata, setting['FV'], SMOOTH)
			_fv, _dist = _ext_fv.run(REALTIME=True)
			
			#print '***********************'
			#print _fv
			#print _dist	
			#print '************************'
			
			# perform svm
			if len(_fv) > 0 and len(_dist) > 0:
				_svm = svm_classify(os.path.join(self.svmdir, setting['SVM']), _fv, _dist)
				self.obst, self.height, self.details = _svm.run()
				self.dist = float("{0:.2f}".format(sum(_dist) / float(len(_dist))))
				dict['det'] = self.details
			else:
				self.status = __STATUS__
				self.result = __RESULT__
				self.obst = __OBST__
				self.height = __HEIGHT__
				self.dist = __DIST__

		if self.dist < 0:
			self.status = __STATUS__
			self.result = __RESULT__
			self.obst = __OBST__
			self.height = 0
			self.dist = __DIST__

		dict['obst'] = self.obst
		dict['height'] = self.height
		dict['dist'] = self.dist	
	
if __name__ == "__main__":
	daemon = Daemon(config)
	daemon.run()
