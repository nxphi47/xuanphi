#!/usr/bin/python

import socket
import sys
import time
import threading
import thread
import traceback
import Queue
import select
import BaseHTTPServer
import CGIHTTPServer
import cgitb

portHTTP = 5555
addressHTTP = ("", portHTTP)

portMain = 12345
ip = socket.gethostname()
multiple_client = 5

portChat = 5551
addressChat = ("", portChat)


# create socket and bind it to local machine put in listenMode
def makeSocket(port, ip):
	s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
	s.bind((ip, port))
	s.listen(multiple_client)
	return s


def handleClient(client, addr, openServer):
	# will return false if the client want to shutdown the server
	print "Received connection from ", addr

	# evaluation on request
	while openServer[0]:
		try:
			inputMess = client.recv(1024)
		except:
			print "Connection time out! or faile to receive input!"
			print "Connection with ", addr, " stopped"
			client.close()
			break
		print "Request received: ", inputMess

		if inputMess:
			if inputMess.lower() == "close":
				# client.send("Thanks for connected with us")
				client.close()
				break
			elif inputMess.lower() == "closeall":
				# client.send("Thanks for connected with us")
				client.close()
				print "Server will also cease!"
				openServer[0] = False
				break
			else:
				try:
					outputMess = eval(inputMess)
					print "Output mess: ", outputMess
					client.send(str(outputMess))
				except Exception as ex:
					print "Input exp not valid"
					client.send("Input exp not valid or error in sending!")
		else:
			print "Connection with ", addr, " stopped"
			break


# function to evaluate expression and return result to client, close/closeall
# is to end the client or the server socket
def handleMessAsync(msg, server, sock):
	if msg.lower() == "close":
		# socket want to close it self
		sock.close()
	elif msg.lower() == "closeall":
		# socket want to close the server
		return "closeall"
	else:
		try:
			result = eval(msg)
		except:
			print "Unable to evaluate client request"
			result = "Invalid request"
		finally:
			return str(result)


# generate the HTTP server to access the html file
def generateHTTPserver():
	cgitb.enable()
	httpServer = BaseHTTPServer.HTTPServer
	cgiHandler = CGIHTTPServer.CGIHTTPRequestHandler
	cgiHandler.cgi_directories.append("/cgi")
	httpd = httpServer(addressHTTP, cgiHandler)
	print "Start generate HTTP server"
	httpd.serve_forever()


# Chat application --------------DOCUMENTATION---------
"""
Chat class object:
	when the main server receive coded message
	"Chat//{'ip' = '127.0.0.1','port'=12345,'username'='xuanphi','mes'='hello world', 'target'='maivy'}"
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
"""


# --------------------------------------


# class Chat to handle chat, it will manipulate the transfer or infomation between sockets
#
class Chat(object):
	def __init__(self):
		self.server = makeSocket(portChat, "")
		self.data = {}

	def update(self, sock, mess):
		pass

	def extractMess(self, mess):
		pass

	def addSocket(self, sock, name):
		pass


if __name__ == '__main__':
	# main program in try block to deal with keyboard interupt to close socket
	# create the socket and bind and listen mode
	serverSock = makeSocket(portMain, socket.gethostname())
	print "Socket created: ", ip, " at port: ", portMain

	try:
		openWhole = [True]
		# list of socket to read
		inputs = [serverSock]
		outputs = []
		messageQueue = {}

		# start the http server in a new thread
		thread.start_new_thread(generateHTTPserver, ())

		# start the comm server for other python client program to work on
		# using asynchronous polling with select, loop until no more serverCOM
		while serverSock in inputs:
			# print "New loop"
			readable, writable, exceptional = select.select(inputs, outputs, inputs)

			# handle the readable
			for sock in readable:
				# it may be the server itself ready to get another client
				if sock is serverSock:
					client, address = sock.accept()
					print "New Connection with: ", address
					client.setblocking(0)
					inputs.append(client)

					# also want to make a queue for data to sent to it
					messageQueue[client] = Queue.Queue()
				else:
					# it may be a client has sent data
					# handle client with a
					data = sock.recv(1024)
					if data:
						# a readable data from the socket
						print "Received: [ %s ] from %s" % (data, sock.getpeername())
						messageQueue[sock].put(data)

						# if socket not in outputs channel, add to it
						if sock not in outputs:
							outputs.append(sock)
					else:
						# no receve input, connection lost
						print "Client disconnected: ", sock.getpeername()
						# stop listening
						if sock in outputs:
							outputs.remove(sock)
						inputs.remove(sock)
						sock.close()

						# remove the queue
						del messageQueue[sock]

			# handle the writable
			for sock in writable:
				try:
					nextMsg = messageQueue[sock].get_nowait()
				except Queue.Empty:
					# no message so stop checking for writabilityu
					print "no message to output"
					outputs.remove(sock)
				else:
					# handle the message
					print "Handle mess for output"
					result = handleMessAsync(nextMsg, serverSock, sock)
					if result == "closeall":
						# close the server
						inputs.remove(serverSock)
						sock.send("Close server")
					else:
						sock.send(result)

			# handle exception
			for sock in exceptional:
				print "Handling exceptional"
				inputs.remove(sock)
				if sock in outputs:
					outputs.remove(sock)
				sock.close()
				del messageQueue[sock]

		# the loop has stopped
		serverSock.close()
		print "Server stopped"

	except KeyboardInterrupt:
		print "keyboard shutdown !"
	except Exception:
		traceback.print_exc(file=sys.stdout)
	finally:
		serverSock.close()
	exit(0)
