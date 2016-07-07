import sys, json, os
import matplotlib.pyplot as plt
import ConfigParser, argparse
from argparse import RawTextHelpFormatter
import numpy as np

DATAs = []		# list of dictionary

'''
	USE to output details file 
		*.csv = file use contain characteristic for each reflected signal
		*-list.csv = file contain major characteristic for each reflected signal together with timestamp and envelop
		*-ori.csv = file contain original envelop
'''

'''
	DICT key:
			oriTimeList = 'otl' + _in		# original time array
			oriEnvList = 'oel' + _in		# original envelop array
			TimeEndList = 'tel' + _in	# time array contain only sound signal (withou initialize portion at beginning)
			EnvEndList = 'eel' + _in	# envelop array contain only sound signal (without initialize portion at beginning)
			initIndex = 'iId' + _in		# index for signal output
			refIndex = 'rId' + _in			# index for reflect signal
			
			# relfect signal characteristic
			pulseWidth = 'pw' + _in		# pulse width
			timeWidth = 'td' + _in			# time width
			areaRatio = 'ar' + _in			# area ratio areaA / areaB
			peakValue = 'pv' + _in			# peak value for reflect signal
			minValue = 'mv' + _in			# minimum value for reflect signal
			totalArea = 'ta' + _in			# total area for reflect signal
			peakTime = 'pt' + _in			# time require to reach peak value
			refEnvList = 'rel' + _in		# reflect signal envelop array (until saturation)
			refTimeList = 'rtl' + _in		# reflect signal time array (until saturation)
'''

OUTPUT_LIST = False
OUTPUT_ORI = False

def scan_input(_file):
	dirname = os.path.dirname(_file)
	with open(_file) as json_file:
		DATAs.append(json.load(json_file))

def get_value_by_key(dic, key):
	if dic.has_key(key):
		return dic[key]
	else:
		return None

def get_output_by_key(dic, key):
	if dic.has_key(key):
		_value = dic[key]
		if isinstance(_value, float):
			_buf = "%.3f" % _value
			return str(_buf)
		elif isinstance(_value, int):
			return str(_value)
	else:
		return "NA"

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
		
	_filename = os.path.abspath(sys.argv[2])
	scan_input(_filename)
	
	_fileout = open(sys.argv[1] + '.csv', 'w')
	if OUTPUT_LIST:
		_filelist = open(sys.argv[1] + '-list.csv', 'w')
	if OUTPUT_ORI:
		_fileori = open(sys.argv[1] + '-ori.csv', 'w')
	
	print "Executing reflect signal file for %s" % os.path.basename(_filename)
	
	_fileout.write("Index\tPulseLength\tTimeWidth(ms)\tPeakAmplitude\tPeakOffset(ms)\tPulseArea\tAreaRatio\n")
	for data in DATAs:
		for x in data['index']:
			_in = str(x)
			if OUTPUT_LIST:
				_filelist.write("\n\n")
			if OUTPUT_ORI:
				_fileori.write("\n\n")
			
			oriTimeList = 'otl' + _in		# original time array
			oriEnvList = 'oel' + _in		# original envelop array
			TimeEndList = 'tel' + _in	# time array contain only sound signal  (without initialize portion and timing change by offset)
			EnvEndList = 'eel' + _in	# envelop array contain only sound signal  (without initialize portion and timing change by offset)
			initIndex = 'iId' + _in		# index for signal output
			refIndex = 'rId' + _in			# index for reflect signal
			
			# relfect signal characteristic
			pulseWidth = 'pw' + _in		# pulse width
			timeWidth = 'td' + _in			# time width
			areaRatio = 'ar' + _in			# area ratio areaA / areaB
			peakValue = 'pv' + _in			# peak value for reflect signal
			minValue = 'mv' + _in			# minimum value for reflect signal
			totalArea = 'ta' + _in			# total area for reflect signal
			peakTime = 'pt' + _in			# time require to reach peak value
			refEnvList = 'rel' + _in		# reflect signal envelop array (until saturation)
			refTimeList = 'rtl' + _in		# reflect signal time array (until saturation)
			
			if get_value_by_key(data, refIndex) is None:
				continue
			
			_fileout.write("%s\t%s\t%s\t%s\t%s\t%s\t%s\n" % (_in, get_output_by_key(data, pulseWidth), get_output_by_key(data, timeWidth),
							get_output_by_key(data, peakValue), get_output_by_key(data, peakTime), get_output_by_key(data, totalArea),
							get_output_by_key(data, areaRatio)))
			
			if OUTPUT_LIST:
				output_detail(_filelist, "Index", x)
				output_detail(_filelist, "Pulse Length", get_value_by_key(data, pulseWidth))
				output_detail(_filelist, "Time Width", get_value_by_key(data, timeWidth))
				output_detail(_filelist, "Peak Amplitude", get_value_by_key(data, peakValue))
				output_detail(_filelist, "Peak Offset", get_value_by_key(data, peakTime))
				output_detail(_filelist, "Pulse Area", get_value_by_key(data, totalArea))
				output_detail(_filelist, "Area Ratio", get_value_by_key(data, areaRatio))
				output_array(_filelist, "Reflected Signal", get_value_by_key(data, refTimeList), get_value_by_key(data, refEnvList))
			
			if OUTPUT_ORI:
				output_detail(_fileori, "min key", get_value_by_key(data, initIndex))
				output_detail(_fileori, "max key", get_value_by_key(data, refIndex))
				output_array(_fileori, "Original Signal", get_value_by_key(data, oriTimeList), get_value_by_key(data, oriEnvList))
	
	_fileout.close()
	if OUTPUT_LIST:	
		_filelist.close()
	if OUTPUT_ORI:
		_fileori.close()
