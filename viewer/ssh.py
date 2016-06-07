import sys, os, paramiko, os.path

# -------------------------------------
# raw string conversion dictionary
# -------------------------------------
escape_dict={
	'\a':r'\a',
	'\b':r'\b',
	'\c':r'\c',
	'\f':r'\f',
	'\n':r'\n',
	'\r':r'\r',
	'\t':r'\t',
	'\v':r'\v',
	'\'':r'\'',
	'\"':r'\"',
	'\0':r'\000',
	'\1':r'\001',
	'\2':r'\002',
	'\3':r'\003',
	'\4':r'\004',
	'\5':r'\005',
	'\6':r'\006',
}

# class to export emotion picture from remote via SSH
class SSH():
	# class initialise
	#	%svr_addr	- remote SSH server address
	#	%usr_name	- username for remote
	#	%passwd		- password for user at remote
	def __init__(self, svr_addr, usr_name, passwd):
		self.addr = svr_addr
		self.usr = usr_name
		self.passwd = passwd

		self.ssh = paramiko.SSHClient()
		self.ssh.set_missing_host_key_policy(paramiko.AutoAddPolicy())
		self.ssh.connect(self.addr, username=self.usr, password=self.passwd)

		self.ftp = self.ssh.open_sftp()

	# obtain file list in given path
	#	%path	- path name for picture directory at remote
	# return
	#	%_fileList	- list of files in remote directory
	def get_list(self, path):
		self.path = path
		_fileList = []
		_cmd = 'ls ' + path
		stdin, stdout, stderr = self.ssh.exec_command(_cmd)
		for line in stdout:
			_fileList.append(line.strip('\n'))
		return _fileList

	# convert path to Linux paramiko ssh format
	#	%line	- Windows path name to convert
	# return
	#	%_scr	- path name in paramiko ssh format
	def convert_path(self, directory, subjName=None, line=None):
		_buf = ''
		for char in line:
			try:
				_buf += escape_dict[char]
			except KeyError:
				_buf += char
		_args = _buf.split('\\')

		_scr = '/cygdrive/'
		for params in _args:
			if ':' in params:
				params = params[:-1].lower()
			if 'Users' in params:
				params = params.lower()
			_scr += params + '/'

		_dir = os.path.join(os.getcwd(), directory)
		if not os.path.exists(_dir):
			os.makedirs(_dir)
		if not subjName is None:
			_fileName = subjName + '.csv'
			_des = os.path.join(_dir, _fileName)
		else:
			_des = os.path.join(_dir, _args[len(_args) - 1])

		return _scr[:-1], _des

	def convert_path2(self, local, remote):
		_scr = remote

		_dir = os.path.join(os.getcwd(), local)
		if not os.path.exists(_dir):
			os.makedirs(_dir)
		_des = _dir

		return _scr, _des

	def convert_path3(self, local, remote):
		_scr_dir = os.path.dirname(remote)
		_scr_file = os.path.basename(remote)
		_dir = os.path.join(os.getcwd(), local)
		if not os.path.exists(_dir):
			os.makedirs(_dir)
		_des = os.path.join(_dir, _scr_file)

		return remote, _des

	# obtain picture from remote via SSH
	#	%filename	- picture name
	def ssh_get(self, src, des):
		print 'Getting CSV file: '
		print 'Source: ', src
		print 'Destination: ', des
		self.ftp.get(src, des)

	# close SSH connection
	def ssh_close(self):
		self.ftp.close()
		self.ssh.close()
