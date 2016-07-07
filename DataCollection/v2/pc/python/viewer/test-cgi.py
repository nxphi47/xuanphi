import sys
import time
import socket
import select
import datetime
import threading

server = ("localhost", 5557)

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
	while True:
		_cmd = input('Insert Command [1 = start | 2 = stop | 3 = quit]')
		if _cmd == 1:
			print 'CGI sent start'
			sock = socket.socket (socket.AF_INET, socket.SOCK_STREAM)
			sock.connect (server)
			sock_write (sock, "CGI-START")
			sock.close()
		if _cmd == 2:
			print 'CGI sent stop'
			sock = socket.socket (socket.AF_INET, socket.SOCK_STREAM)
			sock.connect (server)
			sock_write (sock, "CGI-STOP")
			msg = sock_read (sock)
			print 'CSV file is: ' + msg
			sock.close()
		if _cmd == 3:
			print 'CGI sent quit'
			sock = socket.socket (socket.AF_INET, socket.SOCK_STREAM)
			sock.connect (server)
			sock_write (sock, "__exit__")
			break
