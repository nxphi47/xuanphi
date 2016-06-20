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
