#!/usr/bin/python

import cgi, cgitb
import os
import socket
import sys
import select

portHTTP = 5555
addressHTTP = ("", portHTTP)

portServerSocket = 12345
ipServer = socket.gethostname()

# CGI to execute the chatCLient.py to connect to the server
# and get the port and address of the chatClient.py program

