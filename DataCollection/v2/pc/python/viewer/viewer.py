import os
import ssh

if __name__ == "__main__":
	ssh_addr = '10.80.43.50'
	ssh_user = 'pi'
	ssh_passwd = 'raspberry'
	#ssh_path = '/home/pi/src/SonarSensing/DataCollection/v2/rpi/temp.json'
	ssh_path = '/home/pi/Desktop/sonar/SourceCodes/DataCollection/v4-AIS/temp.json'
	
	_ssh = ssh.SSH(ssh_addr, ssh_user, ssh_passwd)
	
	if _ssh is None:
		print 'No SSH module'
		exit(1)
	
	scr, dest = _ssh.convert_path3('', ssh_path, FILE='temp.js')
	_ssh.ssh_get(scr, dest)
	
	str = ''
	with open(dest, 'r') as f:
		_exist = f.read()
		_exist = _exist.replace("\n", "")
		str = "var testObjStr = '" + _exist[:-1] + "';"
	
	_f = open(dest, 'w')
	_f.write(str)
	_f.close()
