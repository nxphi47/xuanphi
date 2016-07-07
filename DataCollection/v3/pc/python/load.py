import sys, json, os
import matplotlib.pyplot as plt
import ConfigParser, argparse
from argparse import RawTextHelpFormatter
import numpy as np

DATAs = []		# list of dictionary

'''
	DICT key:
		'ot' + x	= original signal time wave
		'oe' + x	= original signal envelop wave
		't' + x		= array of time
		'e' + x		= array of envelop
		'min' + x	= first envelop which excess threshold (sensor output signal)
		'max' + x	= second envelop which excess threshold (reflected sonar signal)
		
		# relfect signal characteristic
		'w' + x		= reflected signal pulse width
		'p' + x		= peak envelop value in reflected signal
		'l' + x		= lowest envelop value in reflected signal
		'a' + x		= total area of reflected signal
		'r' + x		= area ratio (area before peak / area after peak)
		're' + x	= reflected signal envelop
		'rt' + x	= reflected signal time
'''

def scan_input(_file):
	dirname = os.path.dirname(_file)
	with open(_file) as json_file:
		DATAs.append(json.load(json_file))

def get_value_by_key(dic, key):
	if dic.has_key(key):
		return dic[key]
	else:
		return None

def output_detail(filename, key, value):
	if value is None:
		filename.write("%s\tNA\n" % key)
	else:
		if isinstance(value, int):
			filename.write("%s\t%d\n" % (key, value))
		if isinstance(value, float):
			filename.write("%s\t%.3f\n" % (key, value))

def output_array(filename, key, time, envelop):
	filename.write("%s\n" % key)
	if time is None or envelop is None:
		filename.write("\t\tNA\tNA\n")
	else:
		for x in xrange(len(time)):
			filename.write("\t\t%.3f\t%d\n" % (time[x], envelop[x]))
	
if __name__ == "__main__":
	if len(sys.argv) < 3:
		print "Usage:	%s [File1] [outputname] " % sys.argv[0]
		exit(1)
		
	_filename = os.path.abspath(sys.argv[1])
	scan_input(_filename)
	
	_fileout = open(sys.argv[2] + '.csv', 'w')
	_fileori = open(sys.argv[2] + '-ori.csv', 'w')
	
	print "Executing reflect signal file for %s" % os.path.basename(_filename)
	
	for data in DATAs:
		for x in data['index']:
			_in = str(x)
			_fileout.write("\n\n")
			_fileori.write("\n\n")
			
			_oritKey = 'ot' + _in
			_orieKey = 'oe' + _in
			_timeKey = 't' + _in
			_envKey = 'e' + _in
			_minKey = 'min' + _in
			_maxKey = 'max' + _in	
			
			# relfect signal characteristic
			_widthKey = 'w' + _in
			_diffKey = 'td' + _in
			_ratioKey = 'r' + _in
			_peakKey = 'p' + _in
			_lowKey = 'l' + _in
			_areaKey = 'a' + _in
			_refKey = 're' + _in	
			_refTimeKey = 'rt' + _in
			
			if get_value_by_key(data, _maxKey) is None:
				continue
			
			output_detail(_fileout, "Index", x)
			output_detail(_fileout, "pulse width", get_value_by_key(data, _widthKey))
			output_detail(_fileout, "time width", get_value_by_key(data, _diffKey))
			output_detail(_fileout, "peak envelop", get_value_by_key(data, _peakKey))
			output_detail(_fileout, "total area", get_value_by_key(data, _areaKey))
			output_detail(_fileout, "Area Ratio", get_value_by_key(data, _ratioKey))
			output_array(_fileout, "Reflected Signal", get_value_by_key(data, _refTimeKey), get_value_by_key(data, _refKey))
			
			output_detail(_fileori, "min key", get_value_by_key(data, _minKey))
			output_detail(_fileori, "max key", get_value_by_key(data, _maxKey))
			output_array(_fileori, "Original Signal", get_value_by_key(data, _oritKey), get_value_by_key(data, _orieKey))
		
	_fileout.close()
	_fileori.close()
