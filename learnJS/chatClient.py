#!/usr/bin/python

# the chatClient program to execute and create a socket to connect
# to the server with a client name go along with the address

import socket
import traceback
import sys
import argparse
import select

port = 12345
ip = socket.gethostname()

# connect to server function
def connectServer():
	s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)

	try:
		s.connect((ip, port))
	except:
		print "Failed to connect to server %s" %ip
		traceback.print_exc(file=sys.stdout)
		exit(1)

	return s

# get the argument parse
# latter, right now, only the name of client is specify
def getCommandLineInput():
	parser = argparse.ArgumentParser()
	parser.add_argument('-n', dest='username', required=True, help="input userName")

	return parser.parse_args()

if __name__ == '__main__':
	args = getCommandLineInput()

	userName = "unknown"
	try:
		userName = args.username
	except:
		print "Failed to get the user Name"
		exit(1)

	sock = connectServer()

	# main loop
	try:
		# main program
		print "Start connection with name: ", userName
		sock.send("connect:username=" + userName)

		# main loop using the asynchronous connection
		# while True:

	except KeyboardInterrupt:
		print "Program end by user keyboard interuppt"
	except:
		traceback.print_exc(file=sys.stdout)
	finally:
		sock.close()

	exit(0)
