#!/usr/bin/env python2

import sys
import time
import socket
import select
import datetime
import threading
import random

server = ("localhost", 5557)

# detection thread
#	this thread will read input from sensor and store into Sensor class
class detectThread(threading.Thread):
	def __init__(self, label, PIRList, time):
		print 'init threading'
		self.label = label
		self.PIRList = PIRList
		self.startTime = time
		self.isStop = 0
		threading.Thread.__init__(self)

	def timestamp(self):
		timediff = datetime.datetime.now() - self.startTime
		s = timediff.seconds % 60
		m = timediff.seconds / 60
		h = m / 60
		m %= 60
		return '%02d:%02d:%02d.%06d' % (h, m, s, timediff.microseconds)

	def stopThread(self):
		self.isStop = 1

	def run(self):
		self.fileDir = open('./data/sensor_' + self.label + '.txt', 'a')
		while (self.isStop == 0):
			startTime = time.time()		# get current UTC time
			runPeriod = 0			# use to make sure single scan time not exceed detectPeriod
			currentCycle = 0		# current running cycle counter within single scan time
			sleepTime = 0.0			# sleep time after cycle

			# since user can defined detectPeriod and runCycle, in order to complete defined
			#   runCycle within detectPeriod, each cycle used time need to calculated
			singleCyclePeriod = float(detectPeriod) / float(runCycle)

			# single cycle loop
			while runPeriod < detectPeriod:
				# obtain end time for current cycle
				currentCycle = currentCycle + 1
				singleCycleEndTime = startTime + (currentCycle * singleCyclePeriod)

				# make sure total ran cycle not exceed runCycle
				if currentCycle > runCycle:
					currentCycle = runCycle
					break

				# update runPeriod so that it make sure process finished within detectPeriod
				runPeriod = time.time() - startTime

				# update detectedCycle for each sensor if obtain reading from pyhsical pin
				#   since circuit design will toggle IO signal, hence 0v means detected
				if not len(self.PIRList) < maxPIR:
					if not io.input(sensor1):
						self.PIRList[0].raw = 1
					else:
						self.PIRList[0].raw = 0
					if not io.input(sensor2):
						self.PIRList[1].raw = 1
					else:
						self.PIRList[1].raw = 0
					if maxPIR > 2:
						if not io.input(sensor3):
							self.PIRList[2].raw = 1
						else:
							self.PIRList[2].raw = 0
						if not io.input(sensor4):
							self.PIRList[3].raw = 1
						else:
							self.PIRList[3].raw = 0

				# output to a file
				_output = ''
				for x in xrange(maxPIR):
					_output = _output + str(self.PIRList[x].raw) + '\t'
				self.fileDir.write(str(self.timestamp()) + ' - ' + _output + '\n')

				# update sleep time to make sure runCycle within detectPeriod
				sleepTime = float(singleCycleEndTime) - float(time.time())
				if sleepTime < 0.0:
					sleepTime = 0.0

				# sleep after every cycle
				time.sleep(sleepTime)
		self.fileDir.close()

def sock_read (s):
	msg = ''
	while True:
		c = s.recv (1)
		if c == '':
			break
		if c == '\n':
			break
		msg = msg + c
	return msg

def sock_write (s, msg):
	l = 0
	while l < len(msg):
		r = s.send (msg[l:])
		if r == 0:
			return -1
		l = l + r
	s.send ('\n')
	return 0

if __name__ == "__main__":
	_sensorName = 'Test1'

	# Connect to server first, and register ourselves as SENSORS
	sock = socket.socket (socket.AF_INET, socket.SOCK_STREAM)
	sock.connect (server)
	sock_write (sock, "REGISTER=" + _sensorName)

	# Set operating mode to IDLE == Nothing to do
	mode = "IDLE"
	label = ""

	mStartTime = 0.0

	# Main Loop
	while True:
		# ---- Comms portion ----
		ifs, ofs, efs = select.select ([sock, sys.stdin], [], [], 0.01)
		if sys.stdin in ifs:
			print "Received input from STDIN.  Quiting ..."
			sock_write (sock, "UNREGISTER=" + _sensorName)
			break
		if sock in ifs:
			msg = sock_read (sock)
			args = msg.split(":")
			if len(args) > 1:
				label = args[1]
			if msg[0:5] == "START":
				print "Received START from server: " + label
				_label_split = label.split(';')
				for x in _label_split:
					if 'SVM' in x:
						_svmFile = x.split('=')[1]
					if 'TIME' in x:
						_timing = x.split('=')[1]
					if 'CYCLE' in x:
						_cycle = x.split('=')[1]
					if 'FV' in x:
						_fv = x.split('=')[1]
				print 'SVM: %s\nTime: %s\nCycle: %s\nFeature: %s\n' % (_svmFile, _timing, _cycle, _fv)
				'''
					FIXME: need to specific output filename
					 		need to add command to start record-sensor
				'''
				if 'NONREAL' in msg:
					print 'Calling non-realTime function to record-sensor'
					sock_write (sock, 'START-OK:JSON=20160317-063109-2m-curb15-move10.json')
				else:
					print 'Calling realTime function to record-senser'
				mode = "RECORD"
				# -------------------------------- <<<
				# Do What you need to do at Start
				# -------------------------------- <<<
				#mStartTime = datetime.datetime.now()
				#dThread = detectThread(label, PIRList, mStartTime)
				#dThread.daemon = True
				#dThread.setDaemon(True)
				#dThread.start()
			elif msg[0:4] == "STOP":
				print "Received STOP from server"
				mode = "IDLE"
				
				# -------------------------------- <<<
				# Do What you need to do at Stop
				# -------------------------------- <<<
				#mStartTime = 0.0
				#dThread.stopThread()
			elif msg[0:4] == "QUIT":
				print "Received QUIT from server!!!"
				mode = "QUIT"
				# -------------------------------- <<<
				# Do What you need to do before quiting
				# No need to send UNREGISTER if QUIT is from server
				# -------------------------------- <<<
				break
			elif msg[0:6] == 'UPDATE':
				print "Received UPDATE from server!!!"
				mode = "UPDATE"
				_msg = random.randint(0,1)
				if _msg == 0:
					# in status message
					_statusi = random.randint(0,1)
					_status = ''
					if _statusi == 0:
						_status = 'Burst'
					else:
						_status = 'Measurement'
					sock_write(sock, 'STATUS:' + _status + '\n')
				else:
					# in obstacle message
					_statusi = random.randint(0,2)
					_result = ''
					if _statusi == 0:
						#curb messsage
						_result = "OBST=CURB" + str(random.randint(5, 15)) + ";DIST=" + str(random.randint(2,5)) + "\n"
					elif _statusi == 1:
						#unknow message
						_result = "OBST=Unknown;DIST=" + str(random.randint(2,5)) + "\n"
					else:
						_result = "OBST=None\n"
					sock_write(sock, 'RESULT:' + _result)
			else:
				print "Received unknown message: " + msg

		# ---- Actual Sensor Operation ----
		if mode == "RECORD":
			print "Test: In recording mode."
			# Do whatever you want in RUN mode
		else:
			print "Test: In idle mode."
			# Do whatever you want in IDLE mode
		# ---- This is just example ----
		select.select ([sock, sys.stdin], [], [], 15.0)

	# Cleanup
	sock.close()
