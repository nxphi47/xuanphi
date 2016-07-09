import time
import RPi.GPIO as io

outpin = 18 # BCM pin, onboard 12
inpin = 17 # BCM, onboard 11, use pullUp

io.setmode(io.BCM)
io.setup(outpin, io.OUT)
io.setup(inpin, io.IN)

while True:
	io.output(outpin, io.HIGH)
	print "recv: ", io.input(inpin)
	time.sleep(0.5)
	io.output(outpin, io.LOW)
	print "recv: ", io.input(inpin)
	time.sleep(0.5)
