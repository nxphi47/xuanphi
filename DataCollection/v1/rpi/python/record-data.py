#!/bin/env python2

import time
from comms import CommsClient

comms = CommsClient(("127.0.0.1",2929))

print "connecting to server ..."
comms.connect()

print "Sending INIT ..."
msg = comms.send_and_recv(('INIT',))
msg.debug("Received INIT resposne: ")

print "Sending SET-TIMING ..."
msg = comms.send_and_recv(('SET-TIMING',15,100,100,1000))
msg.debug("Received SET-TIMING resposne: ")

print "Sending SET-STATE ..."
msg = comms.send_and_recv(('SET-STATE',[1,0,0,0]))
msg.debug("Received SET-STATE resposne: ")

print "Sending READ 1 ..."
msg = comms.send_and_recv(('READ',))
msg.debug("Received SET-STATE resposne: ")
msg.debug("Received msg: ")
data1 = msg.data[:]

time.sleep(1)

print "Sending READ 2 ..."
msg = comms.send_and_recv(('READ',))
msg.debug("Received SET-STATE resposne: ")
msg.debug("Received msg: ")
data2 = msg.data[:]

print "Sending SET-STATE ..."
msg = comms.send_and_recv(('SET-STATE',[0,0,0,0]))
msg.debug("Received SET-STATE resposne: ")

print "Sending FINI ..."
msg = comms.send_and_recv(('FINI',))
msg.debug("Received FINI resposne: ")

fname = time.strftime("%Y%m%d-%H%M%S") + ".data" 
with open(fname, 'w') as f:
	f.write(" ".join("%d" % x for x in data1) + "\n")
	f.write(" ".join("%d" % x for x in data2) + "\n")
	
comms.close()

print "File '%s' saved" % fname
