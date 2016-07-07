#!/bin/env python2
import sys
import os
import json

def process_json(infn, outf, first=False):
	with open(infn, 'r') as inf:
		samples = json.load(inf)		
	s = json.dumps(samples)
	if not first:
		outf.write(",\n")
	outf.write("{ objstr: '"+s+"',\n")
	outf.write("  fname: '%s'\n}" % infn)

if __name__ == "__main__":
	if (len(sys.argv) <= 1) or ('help' in sys.argv):
		print "Usage: %s <json-file>" % sys.argv[0]
		print "Usage: %s <json-directory>" % sys.argv[0]
		print "Usage: %s <json-file 1> <json-file 2> ..." % sys.argv[0]
		print
		print "The output will be 'samples.js' that can be viewed using 'basic.html'"
		exit()

	first = True
	with open("samples.js", 'w') as outf:
		outf.write("var json_samples = [\n");
		for a in sys.argv[1:]:
			if os.path.isfile(a):
				process_json(a, outf, first)
				first = False
				continue
			if os.path.isdir(a):
				for root, dirs, files in os.walk(a):
					for f in files:
						if f.endswith('.json'):
							process_json(os.path.join(root, f), outf, first)
							first = False
		if not first:
			outf.write('];\n');
		else:
			print "No json file loaded!!!"
		outf.close()

