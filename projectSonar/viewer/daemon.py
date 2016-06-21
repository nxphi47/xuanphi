import os
import commshub
import ssh
import json
import time
import random

if not os.path.isdir('code'):
	import subprocess
	subprocess.call(["ln -s .. code"], shell=True)

from code.processJson import JSONProcessor
from code.demolib import feature_extract
from code.demolib import svm_classify

SVMFILE = 'train.svm'
TIMING = '17,33,50,100,48'
CYCLE = '100'
FEATURES = ["peakValue", "totalArea", "peakTime", "refTimeWidth-immed", "areaRatio"]

SMOOTH = 3
DEBUG = 1

# -----------------------------
#   Global Configuration
# -----------------------------
config = {
	'http-port': 5551,      # socket number to communicate with CGI
	'socket-port': 5557,    # socket number to communicate with code perform control
	'svm-dir': 'svm',        # directory to store svm file
	'json-dir': 'json',      # directory to store json file received via SSH
	'use-ssh': True,       # False if using preferred Arch, True is alternative (ssh to obtain json)
	'ssh-addr': '10.80.43.127',   # IP address for RasPi which connected with sensor
	'ssh-user': 'pi',       # username for RasPi
	'ssh-passwd': '11',     # password for RasPi
	'ssh-path': '/home/pi/src/SonarSensing/DataCollection/v2/rpi/',         # pathname to json file directory
	'cfg-svmfile': SVMFILE,
	'cfg-timing': TIMING,
	'cfg-cycle': CYCLE,
	'cfg-feature': FEATURES
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
		sock_handlers['CGI:CONFIGURE'] = self.handle_configure
		sock_handlers['CGI:START'] = self.handle_start
		sock_handlers['CGI:STOP'] = self.handle_stop
		sock_handlers['CGI:UPDATE'] = self.handle_update
		sock_handlers['START-OK'] = self.handle_start_ok
		sock_handlers['MATRIX'] = self.handle_matrix
		stdin_handlers = { }
		self.chub = commshub.CommsHub(self.cfg, sock_handlers, stdin_handlers)
		self.loadSVM = None
		self.use_ssh = get_cfg(self.cfg, 'use-ssh', False)
		if self.use_ssh:
			self.ssh_addr = get_cfg(self.cfg, 'ssh-addr', '127.0.0.1')
			self.ssh_user = get_cfg(self.cfg, 'ssh-user', 'root')
			self.ssh_passwd = get_cfg(self.cfg, 'ssh-passwd', 'n3tw0rk')
			self.ssh_path = get_cfg(self.cfg, 'ssh-path', '/')
			self.ssh = ssh.SSH(self.ssh_addr, self.ssh_user, self.ssh_passwd)
		else:
			self.ssh = None

		# prepare output directory
		self.jsondir = get_cfg(self.cfg, 'json-dir', 'json')
		if not os.path.isdir(self.jsondir):
			os.makedirs(self.jsondir)
		self.svmdir = get_cfg(self.cfg, 'svm-dir', 'svm')
		if not os.path.isdir(self.svmdir):
			os.makedirs(self.svmdir)
		
		self.cfg_svmfile = os.path.join(self.svmdir, get_cfg(self.cfg, 'cfg-svmfile', SVMFILE))
		self.cfg_timing = get_cfg(self.cfg, 'cfg-timing', TIMING)
		self.cfg_cycle = get_cfg(self.cfg, 'cfg-cycle', CYCLE)
		self.cfg_fv = get_cfg(self.cfg, 'cfg-feature', FEATURES)
		
		self.status = 'Burst'
		self.result = 'STATUS=Burst'
		self.obst = 'None'
		self.height = 'NA'
		self.dist = 'NA'
		self.matrix = []

	def run(self):
		self.chub.run()

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
		self.RasPi = {'sock': csock}
		self.chub.noautoclose.append(csock) # tell chub not to auto-close this socket

	# handle configuration
	def handle_configure(self, csock, cmd):
		print '*** CONFIGURE received - %s***' % cmd
		_params = cmd.split(':')[2].split('_')
		for x in _params:
			if 'SVM' in x:
				_newFile = x.split('=')[1]
				_oldFile = os.path.basename(self.cfg_svmfile)
				self.cfg_svmfile = self.cfg_svmfile.replace(_oldFile, _newFile)
			if 'TIME' in x:
				_timing = x.split('=')[1]
				self.cfg_timing = _timing
			if 'CYCLE' in x:
				_cycle = x.split('=')[1]
				self.cfg_cycle = _cycle
			if 'FV' in x:
				_fv = x.split('=')[1]
				self.cfg_fv = _fv.split(',')
		#print self.cfg_svmfile, self.cfg_timing, self.cfg_cycle, self.cfg_fv
		return "stateMachine.ajax_response('CONFIGURE-OK', '')"

	# sending start command to raspberryPi to start ranging
	def handle_start(self, csock, cmd):
		print '*** START received - %s***' % cmd
		if self.RasPi:
			_svmFile = 'SVM=' + self.cfg_svmfile
			_timing = 'TIME=' + self.cfg_timing
			_cycle = 'CYCLE=' + self.cfg_cycle
			_fv = 'FV=%s' % ','.join(x for x in self.cfg_fv)
			_params = _svmFile + ';' + _timing + ';' + _cycle + ';' + _fv
			if 'NONREAL' in cmd:
				self.mode = 'start-nonreal'
				sock_write(self.RasPi['sock'], "START-NONREAL:" + _params)
				return "stateMachine.ajax_response('START-NONREAL', '');"
			else:
				self.mode = 'start-real'
				self.real_json = JSONProcessor({}, {}, os.path.join(os.getcwd(), self.jsondir))
				if self.real_json is None:
					return "stateMachine.ajax_response('ERROR', '');"
				sock_write(self.RasPi['sock'], "START-REAL:" + _params)
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
						ajaxList.append('None')
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
					k = random.randint(0,9)
					ajaxList.append(str(k))			
		
		return "stateMachine.ajax_response('UPDATE', '%s');" % json.dumps(ajaxList)
	
	# handle real time matrix received from raspi
	def handle_matrix(self, csock, cmd):
		print '*** Matrix received ***'
		
		self.matrix.append(cmd.split(':', 1)[1])
		
		if len(self.matrix) > SMOOTH:
			self.matrix.pop(0)
	
	# handle stop operation
	def handle_stop(self, csock, cmd):
		print '*** STOP received ***'
		if self.mode == 'start-nonreal':
			sock_write(self.RasPi['sock'], 'STOP:')
			# need to perform ssh to obtain json_file
			ajaxList = []
			if self.ssh != None:
				self.ssh_scr, self.ssh_dest = self.ssh.convert_path3(self.jsondir, self.ssh_path + self.json_file)
				self.ssh.ssh_get(self.ssh_scr, self.ssh_dest)
				self.perform_classification(REALTIME=False)
				for key, value in dict.iteritems():
					ajaxList.append(key)
					ajaxList.append(value)	
				return "stateMachine.ajax_response('STOP', '%s');" % json.dumps(ajaxList)
			else:
				return "stateMachine.ajax_response('STOP', '');"
		else:
			# stop real
			sock_write(self.RasPi['sock'], 'STOP-REAL:')
			return "stateMachine.ajax_response('STOP-REAL', '');"
	
	# defined classification result and update dictionary	
	def perform_classification(self, MATRIX=None, REALTIME=True):
		if not REALTIME:
			print 'Processing JSON: ' + self.ssh_dest
			# extract all feature
			_json = JSONProcessor({}, {}, os.path.dirname(self.ssh_dest))
			data = _json.load_file(self.ssh_dest)
			pdata = []
			for p in data:
				pdata.append(json.dumps(p))
			
			# execute feature
			_ext_fv = feature_extract(pdata, self.cfg_fv, SMOOTH)
			_fv, _dist = _ext_fv.run(REALTIME=False)

			# perform svm
			_svm = svm_classify(self.cfg_svmfile, _fv, _dist)
			self.obst, self.height, self.details = _svm.run()
			self.dist = float("{0:.2f}".format(sum(_dist) / float(len(_dist))))
			dict['det'] = self.details
		else:
			data = self.real_json.load_data(self.matrix)
			pdata = []
			for p in data:
				pdata.append(p)
			
			# execute feature
			_ext_fv = feature_extract(pdata, self.cfg_fv, SMOOTH)
			_fv, _dist = _ext_fv.run(REALTIME=True)
			
			print '***********************'
			print _fv
			print _dist	
			print '************************'
			# perform svm
			if len(_fv) > 0 and len(_dist) > 0:
				_svm = svm_classify(self.cfg_svmfile, _fv, _dist)
				self.obst, self.height, self.details = _svm.run()
				self.dist = float("{0:.2f}".format(sum(_dist) / float(len(_dist))))
				dict['det'] = self.details

		dict['obst'] = self.obst
		dict['height'] = self.height
		dict['dist'] = self.dist	
	
if __name__ == "__main__":
	daemon = Daemon(config)
	daemon.run()
