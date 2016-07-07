#!/usr/bin/env python2

import os
import sys
import time
import select
import socket
import thread
import BaseHTTPServer
import CGIHTTPServer
import cgitb
import platform

# CommsHub Class
#	CommsHub (config, sock_handlers, stdin_handlers)
#		config is the set of configuration parameters.  Required are 'http-port' and 'sock-port'
#		sock_handlers specify the list of functions for socket command handlers
#		stdin_handlers specify the list of functions for stdin command handlers
class CommsHub (object):
	def __init__ (self, config, sock_handlers={}, stdin_handlers={}):
		self.config = config
		self.sock_handlers = sock_handlers
		self.stdin_handlers = stdin_handlers
		self.stop = False
		self.httpd_exit = False
		self.comms_exit = False
		
		# create listening sockets
		self.svr = socket.socket (socket.AF_INET, socket.SOCK_STREAM)
		self.svr.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
		self.svr.bind (('', self.config['socket-port']))
		self.svr.listen(5)
		self.debug("listening for CGI requests on port " + str(self.config['socket-port']))
		self.socks = [ self.svr ]
		self.noautoclose = []

		# create httpd server
		cgitb.enable()
		server = BaseHTTPServer.HTTPServer
		handler = CGIHTTPServer.CGIHTTPRequestHandler
		server_address = ("", self.config['http-port'])
		handler.cgi_directories.append("/cgi")
		self.httpd = server(server_address, handler)
		self.debug("listening for HTTP requests on port " + str(self.config['http-port']))
		
	def run (self):
		# Start the threads
		thread.start_new_thread (comms_thread, (self,))
		thread.start_new_thread (httpd_thread, (self,))

		# Wait for key press to exit
		self.debug("MAIN: waiting for commandline")
		while not self.stop:
			if 'Linux' in platform.system():
				select.select ([sys.stdin], [], [])
				self.handle_stdin()
			else:
				import msvcrt
				if msvcrt.kbhit():
					a = msvcrt.getch()
					self.handle_stdin(a)

		# Wait for both threads to end
		self.debug("MAIN: waiting for threads")
		while (not self.httpd_exit) or (not self.comms_exit):
			if 'Linux' in platform.system():
				select.select ([],[],[],1.0)
			else:
				time.sleep(1)

	def comms_loop (self):
		while not self.stop:
			ifs, ofs, efs = select.select (self.socks, [], [], 1.0)
			if self.svr in ifs:
				(csock, caddr) = self.svr.accept()
				self.debug("Accepted connection with " + repr(caddr))
				self.socks.append(csock)
			elif len(ifs) > 0:
				self.handle_msg(ifs[0])
		if self.sock_handlers.has_key("__exit__"):
			self.sock_handlers["__exit__"]()
		for s in self.socks:
			s.close()
		self.socks = []
	
	# handle stdin command
	def handle_stdin (self,ch=''):
		line = ch + sys.stdin.readline()
		if line[0:4] == "exit":
			self.debug("MAIN: EXIT received")
			self.stop = True
			self.httpd.shutdown()
			return
		handled = False
		for h in self.stdin_handlers:
			if line[0:len(h)] == h:
				self.debug ("calling handler: " + h)
				self.stdin_handlers[h] (line)
				handled = True
				break
		if not handled:
			self.debug("MAIN: unknown commandline command: " + line)
	
	# handle incoming message
	def handle_msg (self, csock):
		cmd = sock_read (csock)
		self.debug ("REQ = " + cmd)
		if len(cmd) < 1:
			return
		handled = False
		for h in self.sock_handlers:
			if cmd[0:len(h)] == h:
				self.debug ("calling handler: " + h)
				res = self.sock_handlers[h] (csock, cmd)
				handled = True
				if (res != None) and (len(res) > 0):
					self.debug ("RES = " + str(res) + '\n')
					sock_write (csock, str(res))
					break
				else:
					return
		if not handled:
			sock_write (csock, "ERROR='" + cmd + "'")
			self.debug("Unhandled-REQ = '" + cmd + "'")
		if not csock in self.noautoclose:
			self.close(csock)

	def close (self, s):
		if s in self.noautoclose:
			idx = self.noautoclose.index(s)
			self.noautoclose.pop(idx)
		if s in self.socks:
			idx = self.socks.index(s)
			s.close()
			del self.socks[idx]
		else:
			s.close()
		
	def debug (self, s):
		print (time.strftime("%H:%M:%S") + " CommsHub -- "+s)

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

def handle_res (res):
	s = res.split(' ', 2)
	if s[0] != 'OK':
		return ''
	if len(s) < 3:
		return ''
	return s[2]

def httpd_thread (comms):
	comms.debug("HTTP: Entering main loop")
	comms.httpd.serve_forever()
	comms.httpd_exit = True
	comms.debug("HTTP: Exiting thread")

def comms_thread (comms):
	comms.debug("COMM: Entering main loop")
	comms.comms_loop()
	comms.comms_exit = True
	comms.debug("COMM: Exiting thread")
	
if __name__ == "__main__":
	config = { 'socket-port': 4327, 'http-port': 4321 }
	sock_handlers = { }
	stdin_handlers = { }
	chub = CommsHub (config, sock_handlers, stdin_handlers)
	chub.run()

