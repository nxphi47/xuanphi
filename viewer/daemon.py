import os
import commshub
import ssh
import json
import time

SVMFILE = 'svm-20160510.svm'
TIMING = '17,33,50,100,48'
CYCLE = '100'
FEATURES = ['peakValue', 'totalArea', 'peakTime', 'refTimeWidth-grad', '+gradRatio', 'peakWidthDiff']

# -----------------------------
#   Global Configuration
# -----------------------------
config = {
	'http-port': 5551,      # socket number to communicate with CGI
	'socket-port': 5557,    # socket number to communicate with code perform control
	'svm-dir': 'svm',        # directory to store svm file
	'json-dir': 'json',      # directory to store json file received via SSH
	'use-ssh': True,       # False if using preferred Arch, True is alternative (ssh to obtain json)
	'ssh-addr': '10.80.42.164',   # IP address for RasPi which connected with sensor
	'ssh-user': 'pi',       # username for RasPi
	'ssh-passwd': 'n3tw0rk',     # password for RasPi
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
	'dist': 'NA'
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
		sock_handlers['RESULT'] = self.handle_outcome
		sock_handlers['STATUS'] = self.handle_outcome
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
				sock_write(self.RasPi['sock'], "START-REAL:" + _params)
				return "stateMachine.ajax_response('START-REAL', '');"

	def handle_start_ok(self, csock, cmd):
		print '*** START-OK received - %s***' % cmd
		cmd_split = cmd.split(':')
		for x in cmd_split:
			if 'JSON' in x:
				self.json_file = x.split('=')[1]
		print 'JSON File: ' + self.json_file
	
	def handle_update(self, csock, cmd):
		print '*** UPDATE received ***'
		result = []
		sock_write(self.RasPi['sock'], 'UPDATE:')
		('["curb", 10, "distance", 3.0, "fv", 0.3]');
		
		ajaxList = []
		for key, value in dict.iteritems():
			ajaxList.append(key)
			ajaxList.append(value)			
		
		return "stateMachine.ajax_response('UPDATE', '%s');" % json.dumps(ajaxList)
	
	def handle_outcome(self, csock, cmd):
		global dict
		print '*** OUTCOME received ***'
		self.result = cmd
		if cmd[0:5] == 'STATUS':
			self.status = cmd.split(':')[1]
			dict['status'] = self.status
		else:
			_outcome = cmd.split(':')[1].split(';')
			for x in _outcome:
				if 'OBST' in x:
					if 'CURB' in x:
						self.height = x.split('=')[1][4:]
						self.obst = 'Curb'
					if 'None' in x:
						self.obst = 'None'
						self.height = 'NA'
						self.dist = 'NA'
					if 'Unknown' in x:
						self.obst = 'Unknown'
						self.height = 'NA'
				if 'DIST' in x:
					self.dist = x.split('=')[1]	
				
			dict['obst'] = self.obst
			dict['height'] = self.height
			dict['dist'] = self.dist		
	
	def handle_stop(self, csock, cmd):
		print '*** STOP received ***'
		if self.mode == 'start-nonreal':
			sock_write(self.RasPi['sock'], 'STOP:')
			# need to perform ssh to obtain json_file
			result = []
			if self.ssh != None:
				self.ssh_scr, self.ssh_dest = self.ssh.convert_path3(self.jsondir, self.ssh_path + self.json_file)
				self.ssh.ssh_get(self.ssh_scr, self.ssh_dest)
				result = self.perform_classification(REALTIME=False)
			return "stateMachine.ajax_response('STOP', '%s');" % json.dumps(result)
		else:
			return "stateMachine.ajax_response('STOP-REAL', '');"
		
	def perform_classification(self, REALTIME=True):
		result = []
		
		return result
	
if __name__ == "__main__":
	daemon = Daemon(config)
	daemon.run()
