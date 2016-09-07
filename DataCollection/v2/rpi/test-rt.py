#!/bin/env python2

import socket
import sys
import time


# Specify server
server_address = ('localhost', 4270)

# what message to send
msgs = [
	'SETCFG T-RANGE=200',
	'GETCFG',
]

# Create a UDP socket
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

for m in msgs:
	time.sleep(3)
	print "** sending msg: %s" % m
	sock.sendto(m+'\0', server_address)
	data, server = sock.recvfrom(4096)
	print "** received response: %s", data
	
sock.close()
	
		
