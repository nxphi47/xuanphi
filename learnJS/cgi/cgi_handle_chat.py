#!/usr/bin/python

import cgi, cgitb
import os
import socket
import sys
import select

portHTTP = 5555
addressHTTP = ("", portHTTP)

portChat = 5552
addressChat = ""

# testing purpose
"""
if __name__ == "__main__":
	s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)

	try:
		s.connect((addressChat, portChat))
	except:
		print "Failed: unable to connect to server chat"
		exit(1)

	username = raw_input("Input username: ")
	target = raw_input("input target name: ")
	mes = raw_input("input the mes: ")

	data = {'username': username, 'message': mes, 'target': target}
"""

# this will create the message input to the socket
def makeSocketMessage(username, mess, target):
	_mes = "Chat//"
	_dict = {'username': username, 'message': mess, 'target': target}
	return _mes + str(_dict)

def handleServerChat(address, port, cmd):
	s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	# to make the socket reusable without timeout
	s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)

	try:
		s.connect((address, port))
	except:
		# failed to connect the server
		print "Failed to connect to server chat: ", (address, port)
		return "Chat//:NoConnectionError"

	s.send(cmd)
	s.settimeout(10)
	try:
		result = s.recv(4096)
	except:
		print "Failed to get response from server chat, after sending"
		return "Chat//:NoResponseError"
	finally:
		s.close()

	# may need to handle the result
	return result

# main program
form = cgi.FieldStorage()

# get data from fields
username = form.getvalue('username')
message = form.getvalue('message')
target = form.getvalue('target')

command = makeSocketMessage(username, message, target)

res = handleServerChat(addressChat, portChat, command)

# send back to html
print "Content-type:text/html\r\n\r\n"
print res