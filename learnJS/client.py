import socket
import traceback
import sys



port = 12345
ip = socket.gethostname()

if __name__ == '__main__':
	# make socket and connect to local machine
	s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)

	try:
		s.connect((ip, port))
	except:
		print "Failed to connect %s" % (ip)
		exit(1)

	# loop
	try:
		# main program
		print "Connection completed"
		while True:
			exp = raw_input("Enter expression: ")
			if str(exp).lower() == "close":
				s.close()
				break
			else:
				s.send(str(exp))
				result = s.recv(1024)
				if result:
					print "Result: ", result
				else:
					print "Disconnected !"
					break

		print "program finish"
	except KeyboardInterrupt:
		print "Client ended, keyboard interuppt!"
	except:
		traceback.print_exc(file=sys.stderr)
	finally:
		s.close()
	exit(0)
