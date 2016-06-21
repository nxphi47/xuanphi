#!/usr/bin/python

# the chatClient program to execute and create a socket to connect
# to the server with a client name go along with the address

import socket
import traceback
import sys
import argparse
import select
import server

portChat = 5552
addressChat = ("", portChat)

# Chat application --------------DOCUMENTATION---------
"""
Chat class object:
	when the main server receive coded message
	"Chat//{'username':'xuanphi','message':'hello world',
	 'target':'maivy'}"
	code: mes = "" if it is just an update, response = "" if nothing it sent

	main server will pass the socket to and message to the Chat object to handle it
	@@ Chat object:
	the chat app will response to the socket with the queue message to that socket, or response
	response nothing, the received message will be put to the dict of queue mess as the target as key
	dict = {
		'xuanphi': ['maivy: hello world', 'maivy: chao phi']
		'maivy': ['xuanphi: chao phi', 'xuanphi: chao vy']
	}
	socket = {
		'xuanphi': socket1
		'maivy': socket2
	}
	The message will wait for the request on update of the socket to execute

	the class will run on another thread
"""

# --------------------------------------

class Chat(object):
	def __init__(self, address, port):
		self.server = server.makeSocket(port, address)
		self.sockets = {}
		self.users = {}
		self.sockInput = [self.server]
		self.sockOutput = []
		print "Socket created for chat App: ", (address, port)

	# Name of user and the queue of message
	def updateUsers(self, name, mess):
		if self.users.has_key(name):
			self.users[name].append(mess)
		else:
			self.users[name] = [mess]

	# get the message for sending
	def getMessage(self, name):
		_mes = ""
		if self.users.has_key(name):
			if len(self.users[name]) == 0:
				return _mes
			else:
				while len(self.users[name]) > 0:
					_eleMes = self.users[name].pop(0)
					if _eleMes != '':
						_mes += _eleMes + "\n"
		else:
			print "GetMessage: %s , no record for him" % name
		if _mes == "":
			_mes = "\n"
		return _mes

	# process the message
	def processInput(self, codeMess, sock):
		try:
			_code = eval(codeMess)
		except:
			print "Code mess invalid: ", codeMess
			return

		if not (_code.has_key('target') and _code.has_key('username') and _code.has_key('message')):
			print "Code received not have valid attribute: ", _code
			return
		if _code['message'] != "":
			_input = _code['username'] + ": " + _code['message']
		else:
			_input = ""
		self.updateUsers(_code['target'], _input)
		self.updateUsers(_code['username'], "")
		self.updateSockets(sock, _code['username'])

	# add the socket
	def updateSockets(self, sock, name):
		# if the socket with that name exist, there must be another client with the same name
		if self.sockets.has_key(name):
			print "Client already exist: ", name
			"""
				do somgthing on this
			"""
			return
		else:
			self.sockets[name] = sock

	# process the output, with the socket
	def processOutput(self, sock):
		try:
			username = self.sockets.keys()[self.sockets.values().index(sock)]
		except:
			print "ProcessOuput: failed to get the username from self.sockets"
			sock.send("Server failed to get this username when response")
			sock.close()
			return
		# send the output back to socket
		_mes = self.getMessage(username)
		sock.send(_mes)

		# clear everthing
		self.sockOutput.remove(sock)
		del self.sockets[username]
		self.users[username] = []

	# run, to be run in another thread
	def run(self):
		print "Starting chat app :ahihi"
		try:
			openServer = [True]

			while self.server in self.sockInput:
				readable, writable, exceptional = select.select(self.sockInput, self.sockOutput, self.sockInput)

				# handle readable
				for sock in readable:
					# if the server get connection
					if sock is self.server:
						client, address = sock.accept()
						# print "Chat: new connection: ", address
						client.setblocking(0)
						self.sockInput.append(client)

					# may want to do somthing here on connection
					else:
						# it may be the client sent data
						data = sock.recv(4096)
						if data:
							# readable data from the socket
							if "Chat//" in data:
								# valid
								self.processInput(data[6:], sock)
								if sock not in self.sockOutput:
									self.sockOutput.append(sock)
							else:
								# invalid codeData, reject the request
								print "Chat: Error: invalid code data: ", data
								sock.close()
						else:
							# connection lost
							# stop listening
							if sock in self.sockOutput:
								self.sockOutput.remove(sock)
							self.sockInput.remove(sock)
							sock.close()

							# del the information in socket and users
							try:
								del self.sockets[self.sockets.values().index(sock)]
							except:
								pass

				for sock in writable:
					self.processOutput(sock)

				# handle exception
				for sock in exceptional:
					print "Handle exceptional"
					self.sockInput.remove(sock)
					if sock in self.sockOutput:
						self.sockOutput.remove(sock)
					sock.close()

			self.server.close()
			print "Chat application stop!"
		except KeyboardInterrupt:
			print "Chat app stop: key board interrupt"
		except Exception:
			traceback.print_exc(file=sys.stdout)
		finally:
			self.server.close()
