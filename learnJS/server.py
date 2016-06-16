import socket
import sys
import time
import threading
import thread
import traceback

port = 12345
ip = socket.gethostname()
multiple_client = 5


# create socket and bind it to local machine put in listenMode
def makeSocket():
	s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
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


if __name__ == '__main__':
	# main program in try block to deal with keyboard interupt to close socket
	# create the socket and bind and listen mode
	sock = makeSocket()
	print "Socket created: ", ip, " at port: ", port

	try:
		openWhole = [True]

		while openWhole[0]:
			client, address = sock.accept()
			thread.start_new_thread(handleClient, (client, address, openWhole))
			print "new Thread start!"

		sock.close()
		print "Server stopped"

	except KeyboardInterrupt:
		print "keyboard shutdown !"
	except Exception:
		traceback.print_exc(file=sys.stdout)
	finally:
		sock.close()
	exit(0)
