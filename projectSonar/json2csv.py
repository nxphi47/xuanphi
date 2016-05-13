#!/bin/env python2

import sys
import json

def get_json_object(f, sbegin):
	stmp = sbegin.replace("'", '"')
	while not ('}' in stmp):
		s = f.readline()
		if s == '':
			break
		stmp += ' ' + s.replace("'", '"')
		if '}' in s:
			break
	stmp = stmp.replace('\n',' ')
	print stmp
	return eval(stmp)
	return json.loads(stmp)
	
def out_csv(f, obj):
	if obj == None:
		return
	if obj.has_key("sensors"):
		# setting of rpi
		# output as comment
		f.write("Sensors,%s\n" % (",".join('%d'%x for x in obj['sensors'])))
		f.write("Timings,%s\n" % (",".join('%d'%obj[x] for x in ['T-ranging','T-sampling','T-next','T-cycle','T-sample'])))
	else:
		# should be sensor data
		f.write("Start,%d,%d\n" % (obj['start'][0],obj['start'][1]))
		f.write(",delta-time(ms),time(ms),Envelop\n")
		t = 0.0
		for d in obj['samples']:
			td = d[0] / 1000.0
			t += td
			f.write(",%.3f,%.3f,%d\n" % (td, t, d[1]))
	
if len(sys.argv) < 3:
	print "Usage: %s <input> <output>\n" % sys.argv[0]
	exit(1)
	
inf = open(sys.argv[1], "r")
outf = open(sys.argv[2], "w")
while True:
	s = inf.readline()
	if s == '':
		break
	if not ('{' in s):
		continue
	out_csv(outf, get_json_object(inf, s))
inf.close()
outf.close()	
		
