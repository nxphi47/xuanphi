import socket
import traceback
import sys
import time

port = 12345
ip = socket.gethostname()
messages = [ '1+1',
             '2*2',
             '"helloooo"'
             ]
"""
if __name__ == '__main__':
	# make socket and connect to local machine
	socks = [
		socket.socket(socket.AF_INET, socket.SOCK_STREAM),
		socket.socket(socket.AF_INET, socket.SOCK_STREAM)
		]
	socks[0].setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
	socks[1].setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)


	try:
		socks[0].connect((ip, port))
		socks[1].connect((ip, port))
	except:
		print "Failed to connect %s" % (ip)
		exit(1)

	# loop
	try:
		# main program
		print "Connection completed"

		for mes in messages:
			for s in socks:
				print "%s: send %s" %(s.getsockname(), mes)
				s.send(mes)
				time.sleep(1)
				data = s.recv(2048)
				print "%s: received %s" %(s.getsockname(), data)


		print "program finish"
	except KeyboardInterrupt:
		print "Client ended, keyboard interuppt!"
	except:
		traceback.print_exc(file=sys.stderr)
	finally:
		socks[0].close()
		socks[1].close()
	exit(0)
"""

test = [1,2,3,5,1,3,2,3,6,3]
for i in set(test):
	print i