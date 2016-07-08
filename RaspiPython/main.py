import time
import RPi.GPIO as io

ledPin = 18 # BCM pin, onboard 12
buttonPin = 17 # BCM, onboard 11, use pullUp

io.setmode(io.BCM)
io.setup(ledPin, io.OUTPUT)

while True:
	io.w