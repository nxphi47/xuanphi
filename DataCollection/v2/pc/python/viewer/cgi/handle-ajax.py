#!/usr/bin/env python2

import time
import sys
import socket
import cgi, cgitb

comms_port = 5557
log = open ("ajax.log", "a")

def debug (s):
	log.write (time.strftime("%H:%M:%S") + " CurbDetectCGI -- " + s + '\n')

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

def send_cmd (ip, port, cmd):
	sock = socket.socket (socket.AF_INET, socket.SOCK_STREAM)
	sock.connect ((ip, int(port)))
	sock_write (sock, cmd)
	res = sock_read (sock)
	sock.close()
	return res

form = cgi.FieldStorage()
param = form.getvalue("PARAM", "")
debug ("PARAM=" + param)

res = send_cmd("127.0.0.1", comms_port, "CGI:"+param)

debug ('return response = \n' + res)

print "Content-type: text/html"
print
print res

log.close()
