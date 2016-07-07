#!/bin/env python2

# -------------------------
# process-json.py
#   Adapted from Kian Soon's json2graph.py to generate only the feature vector
# -------------------------


import sys
import json
import os
import numpy as np
import matplotlib.pyplot as plt
import ConfigParser
import argparse
from argparse import RawTextHelpFormatter
from feature import singleReflect
import utils


ID_count = 0
dist_known = []


# Convenience: config or default value
def get_cfg(cfg, k, default):
    if cfg.has_key(k):
        return cfg[k]
    else:
        return default


# converting condition string to boolean
def convert_cfg(status):
	if isinstance(status, bool):
		return status
	if 'True' in status:
		return True
	else:
		return False


# read config file
def configSectionMap(Config, section, _dict=None):
	if _dict is None:
		_dict = {}
	options = Config.options(section)
	for option in options:
		try:
			_dict[option] = Config.get(section, option)
			if _dict[option] == -1:
				print("Error: skip reading config: %s" % option)
		except:
			print("Error: exception on %s!" % option)
			_dict[option] = None
	return _dict


# actual class to process the data
class JSONProcessor(object):
	def __init__(self, config, _dict, root):
		self.config = config
		self.cfg = _dict
		self.root = root
		self.settings = {}
		self.settings['temperature'] = float(get_cfg(self.cfg, 'temperature', '22'))
		self.settings['threshold'] = float(get_cfg(self.cfg, 'threshold', '5'))
		self.settings['window'] = float(get_cfg(self.cfg, 'windows', '100'))
		self.settings['averaging'] = int(get_cfg(self.cfg, 'averaging', '3'))
		self.settings['outputDetail'] = convert_cfg(get_cfg(self.cfg, 'output_detail', 'True'))
		self.settings['outputWave'] = convert_cfg(get_cfg(self.cfg, 'output_wave', 'True'))
		self.settings['actualDist'] = float(get_cfg(self.cfg, 'actual_dist', '3'))
		self.settings['actualHeight'] = float(get_cfg(self.cfg, 'actual_height', '3'))
		self.settings['features'] = ['timeWidth', 'peakValue', 'totalArea', 'peakTime']
		self.settings['zero-gradient'] = int(get_cfg(self.cfg, 'zerograd', '3'))
		self.settings['pulse-end-detect-method'] = int(get_cfg(self.cfg, 'endpulse', '1'))
		self.settings['valid-id-file'] = get_cfg(self.cfg, 'validid', 'validid.csv')
		self.settings['output_graph'] = convert_cfg(get_cfg(self.cfg, 'output_graph', 'False'))
		self.settings['distance_marker'] = convert_cfg(get_cfg(self.cfg, 'distance_marker', 'False'))
		self.settings['ignore_reflected'] = convert_cfg(get_cfg(self.cfg, 'ignore_reflected', 'False'))
		self.settings['test'] = convert_cfg(get_cfg(self.cfg, 'test', 'False'))

		self.validid = {}
		# if len(self.settings['valid-id-file']) > 0:
		#     self.load_validid()
		self.data_id = 0

	# return the important settings metadata
	def get_metadata(self, info={}):
		_METADATA = ['temperature', 'threshold', 'window', 'averaging', 'actualDist',
                     'actualHeight', 'zero-gradient', 'pulse-end-detect-method', "RPi",
                     'valid-id-file']
		for k in self.settings:
			if k in _METADATA:
				info[k] = self.settings[k]
		return {'META': info}

	def load_AIS_file(self, filename):
		ret = []
		self.inf = open(os.path.join(self.root, filename), "r")
		print 'Processing: ', filename
		self.filename = filename
		with open(self.filename) as dfile:
			data = json.load(dfile)
		
		for obj in data:
			data = self.process_AIS_object(obj)
			if data:
				data['filename'] = filename
				self.data_id += 1
				data['DataID'] = self.data_id
				ret.append(data)
				print "Sample - OK, assigned ID: %d" % data['DataID']
				if self.settings['output_graph']:
					self.plot_graph(data)
		return ret

	# load object from a data
	def load_AIS_data(self, data):
		ret = []
		print 'Processing data'
		for d in data:
			#s = json.loads(d)
			#s = eval(d)
			s = d
			if s == '':
				break
			data = self.process_AIS_object(s)
			if data:
				data['filename'] = 'realtime'
				self.data_id += 1
				data['DataID'] = self.data_id
				ret.append(data)
		return ret

	def get_AIS_json_object(self, sbegin):
		while not ('}]' in sbegin):
			s = self.inf.readline()
			if s == '':
				break
			sbegin += ' ' + s
			if ']\n' in s:
				break
		sbegin = sbegin.replace('\n', ' ')
		sbegin = sbegin.rstrip(', ')
		return eval(sbegin)

	# process the input JSON object
	def process_AIS_object(self, obj):
		if obj is None:
			return
		if obj.has_key("sensors"):
			# setting of rpi
			self.settings.update({"RPi": obj})
		else:
			# should be sensor data
			data = {}
			if obj.has_key('recv-sensor'):
				data['RX-Sensor'] = obj['recv-sensor']
			if obj.has_key('firing-sensor'):
				data['TX-Sensor'] = obj['firing-sensor']
			if obj.has_key('id'):
				data['Sample-ID'] = obj['id']
			# convert timing data
			t = 0.0
			data['Timing'] = obj['timing']
			data['Envelop'] = obj['envelop']
			# calculate threshold index range
			_min, _max = self.cal_range2(data)
			if (not _min == 0) and (not _max == 0):
				# process the detected single Reflect signal

				# the following part is only for testing when we know all the distance to the curb
				tolerance = 0.3
				if self.settings['test']:
					dist = utils.get_actual_dist_filename(self.filename)
					# print "dist = ", dist, "real dist = ", self.get_dist(data['Timing'][_min], data['Timing'][_max])
					if dist is not None:
						if not (dist + tolerance > self.get_dist(data['Timing'][_min], data['Timing'][_max]) > dist - tolerance):
							return None

				global ID_count
				ID_count += 1
				#print "Process ID: ", data['Sample-ID'], "start = ", _max, " IDcount ", ID_count
				sr = singleReflect(data['Envelop'], data['Timing'], _min, _max)
				sr.split_array()
				data['Working'] = sr.process_data()

				_initIndex = data["Working"]["txPulseStart"]
				_refIndex = data["Working"]["refPulseStart"]
				_initTime = data["Timing"][_initIndex]
				_refTime = data["Timing"][_refIndex]
				data['distance'] = self.get_dist(_initTime, _refTime)
				# data['Feature-Vector'] = sr.get_feature_vector(self.settings['features'])
				return data
			else:
				# print "Sample %d - no reflected signal detected" % obj['id']
				# if no refection signal found, insert everything as 0
				data = {}
				data['distance'] = -1.
				_paramsList = ['Timing', 'Envelop']
				
				for key in _paramsList:
					_list = []
					for x in xrange(20):
						_list.append(0.0)
					data[key] = _list

				data_working = {}
				_working_list = ['EnvEndList', 'oriEnvList', 'TimeEndList', 'envGradient', 'gradientList', 'refEnvList', 'refTimeList-default',
									'oriTimeList', 'refEnvList-default', 'refTimeList']
				_working_param = ['txPulseWidth', 'peakWidthDiff', '-gradRatio', 'refPulseWidth-grad', 'refPulseWidth-immed', 'minValue', 'txPulseWidth-ave',
									'txPulseWidth-immed', 'txTimeWidth-ave', 'txTimeWidth-grad', 'initIndex', 'peakTime', 'txPulseStart', 'altFloor',
									'floorValue', 'timeWidth', 'refTimeWidth-immed', 'refPulseStart', 'areaRatio', 'txPulseWidth-grad', 'totalArea', 
									'refPulseWidth-ave', 'pulseWidth', '+gradRatio', 'refTimeWidth-grad', 'peakValue', 'peakWidthDiv', 'peakValueDiv',
									'refIndex', 'txTimeWidth-immed', 'normTimeWidth', 'refTimeWidth-ave', '0gradRatio', 'txTimeWidth', 'gradChange']

				for key in _working_list:
					_list = []
					for x in xrange(20):
						_list.append(0.0)
					data_working[key] = _list

				for key in _working_param:
					data_working[key] = 0.0

				data['Working'] = data_working
				return data
				#return None

    # load object from a filename
	def load_file(self, filename):
		ret = []
		self.inf = open(os.path.join(self.root, filename), "r")
		print 'Processing: ', filename
		self.filename = filename
		while True:
			s = self.inf.readline()
			if s == '':
				break
			if not ('{' in s):
				continue
			obj = self.get_json_object(s)
			if not self.check_valid_id(obj, filename):
				continue
			data = self.process_object(obj)
			if data:
				data['filename'] = filename
				self.data_id += 1
				data['DataID'] = self.data_id
				ret.append(data)
				print "Sample %d - OK, assigned ID: %d" % (obj['id'], data['DataID'])
				if self.settings['output_graph']:
					self.plot_graph(data)
		self.inf.close()
		return ret
	
	# load object from a data
	def load_data(self, data):
		ret = []
		print 'Processing data'
		for d in data:
			s = json.loads(d)
			if s == '':
				break
			data = self.process_object(s)
			if data:
				data['filename'] = 'realtime'
				self.data_id += 1
				data['DataID'] = self.data_id
				ret.append(data)
		return ret
	
	# getting JSON object
	def get_json_object(self, sbegin):
		stmp = sbegin.replace("'", '"')
		while not ('}' in stmp):
			s = self.inf.readline()
			if s == '':
				break
			stmp += ' ' + s.replace("'", '"')
			if '}' in s:
				break
		stmp = stmp.replace('\n', ' ')
		# print stmp
		return eval(stmp)
	
	# process the input JSON object
	def process_object(self, obj):
		if obj is None:
			return
		if obj.has_key("sensors"):
			# setting of rpi
			self.settings.update({"RPi": obj})
		else:
			# should be sensor data
			data = {'Start-Time': [obj['start'][0], obj['start'][1]], 'Timing': [], 'Envelop': []}
			if obj.has_key('recv-sensor'):
				data['RX-Sensor'] = obj['recv-sensor']
			if obj.has_key('firing-sensor'):
				data['TX-Sensor'] = obj['firing-sensor']
			if obj.has_key('id'):
				data['Sample-ID'] = obj['id']
			# convert timing data
			t = 0.0
			for d in obj['samples']:
				td = d[0] / 1000.0
				t += td
				data['Timing'].append(t)
				data['Envelop'].append(float(d[1]))
			# calculate threshold index range
			_min, _max = self.cal_range2(data)
			if (not _min == 0) and (not _max == 0):
				# process the detected single Reflect signal

				# the following part is only for testing when we know all the distance to the curb
				tolerance = 0.3
				if self.settings['test']:
					dist = utils.get_actual_dist_filename(self.filename)
					# print "dist = ", dist, "real dist = ", self.get_dist(data['Timing'][_min], data['Timing'][_max])
					if dist is not None:
						if not (dist + tolerance > self.get_dist(data['Timing'][_min], data['Timing'][_max]) > dist - tolerance):
							return None

				global ID_count
				ID_count += 1
				#print "Process ID: ", data['Sample-ID'], "start = ", _max, " IDcount ", ID_count
				sr = singleReflect(data['Envelop'], data['Timing'], _min, _max)
				sr.split_array()
				data['Working'] = sr.process_data()

				_initIndex = data["Working"]["txPulseStart"]
				_refIndex = data["Working"]["refPulseStart"]
				_initTime = data["Timing"][_initIndex]
				_refTime = data["Timing"][_refIndex]
				data['distance'] = self.get_dist(_initTime, _refTime)
				# data['Feature-Vector'] = sr.get_feature_vector(self.settings['features'])
				return data
			else:
				# print "Sample %d - no reflected signal detected" % obj['id']
				return None

	# calculate distance
	def get_dist(self, xmin, xmax):
		_sound_speed = 331.4 + .61 * (self.settings['temperature'])
		_travel_time = (xmax - xmin) / 1000.0
		return _sound_speed * _travel_time / 2

	# threshold should vary based on distance
	# need to explore more on this
	def get_threshold(self, data, emax, ecur):
		_start = data['Timing'][emax]
		_to = data['Timing'][ecur]
		if _start == _to:
			return 0.
		_dist = self.get_dist(_start, _to)
		_thres = (1. / _dist) * 10
		return _thres

	# calculate index range for start and stop peak
	def cal_range(self, data):
		_first_index = 0
		_second_index = 0
		for i, x in enumerate(data['Envelop']):
			_ave = 0
			for y in xrange(self.settings['averaging']):
				_ave += data['Envelop'][i - (y + 1)]
			_value = x - (_ave / self.settings['averaging'])
			if _first_index == 0:
				if _value >= 5:
					_first_index = i
			elif _value >= self.get_threshold(data, _first_index, i):
				# sometime value will change from 52 -> 54 and cause trigger of threshold
				# this is to make sure if threshold trigger, envelop should more than 55
				if data['Envelop'][i] <= 55:
					continue
				if (i - _first_index) > self.settings['window']:
					_second_index = i
					break
		# handling error when change for _max too close to end
		if (len(data['Envelop']) - _second_index) < 10:
			_second_index = 0
		return _first_index, _second_index

	# version 2 to calculate and find the valid signal base on properties
	# 1/The right side must be silent with saturation (adjust by % of noise tolerance to the peak, length, and offset)
	# 2/The highest peak in the vicinity - find the peak of the signal
	# 3/The left side should be silent enough but also be filled with noise
	# **** version 3 to be developed to decided the correct signal with there are multiple valid signal
	# **** change for AIS sensor
	def cal_range2(self, data):
		start_index = 0
		start_index_end = 0
		end_index = 0
		end_index_end = 0
		tolerance = 75
		emitting_envelop_min_diff = 1500
		saturation = 0

		initial_test_range = self.settings['averaging'] * 12
		for x in xrange(initial_test_range):
			saturation += data['Envelop'][x]
		saturation /= initial_test_range
		tolerance_reflected = 30

		# print "\nplotting %s" % self.graphTitle
		# check if the begining is flat, if not, it fluctuate so much to determined the reflected signal
		if not self.check_silent(data, 0, initial_test_range, tolerance, saturation, 0, True):
			# print "Fluctuate too much to measure reflected signal"
			return 0, 0

		# finding the range of emitting signal
		for i, data['Envelop'][i] in enumerate(data['Envelop']):
			_ave_advance = 0
			_ave_behind = 0
			for y in xrange(self.settings['averaging']):
				_ave_advance += data['Envelop'][i - (y + 1)]
				if (i + y + 1) >= len(data['Envelop']):
					_ave_behind = data['Envelop'][i]
				else:
					_ave_behind += data['Envelop'][i + (y + 1)]

			# _diff = (_ave_behind / self.settings['averaging']) - (_ave_advance / self.settings['averaging'])
			if i < len(data['Envelop']) - 1:
				_diff = data['Envelop'][i + 1] - data['Envelop'][i]

			if start_index == 0:
				if _diff >= emitting_envelop_min_diff:
					start_index = i
			elif start_index_end == 0:
				# looking for the end of the emitting signal
				# end when the subsequent is silent for half the tolerance reflected / 2
				if self.check_silent(data, i, tolerance_reflected + 10, tolerance + 20, saturation, 0, True):
					start_index_end = i
					break

		# handling fault signal when sensor fault
		lowerTole = 900
		for i in xrange(len(data['Envelop']) - 1, start_index, -1):
			x = data['Envelop'][i]
			if x < saturation - lowerTole or x > saturation + emitting_envelop_min_diff + 3000:
				print "Fault: ID: ", "at ", i, " and val = ", x
				return 0, 0

		# print "starting: %d %d" % (start_index, start_index_end)
		# finding the range, peak, of reflected signal
		# This signal is followed by a serials of silence. The pattern before it usually silent
		# if there is noise very close the the signal, they cannot be larger than the relected signal
		# finding the feature vector of signal
		if start_index == 0:
			return 0, 0
		end_index, end_index_end = self.get_reflected_signal(data, start_index_end, tolerance, saturation)
		# print "---min = %d %d, max = %d %d, data size = %d" % (
		# start_index, start_index_end, end_index, end_index_end, len(data['Envelop']))

		print "finally: start = %d %d , end = %d %d" % (start_index, start_index_end, end_index, end_index_end)
		return start_index, end_index

	# Get the reflected signal based the set of potential signal
	def get_reflected_signal(self, data, emit_end, tolerance, saturation):
		# features of signal
		signals = []
		start = 0
		end = 0
		peak = 0, 0  # index and value
		noise_percent = 0.15
		noise_envelop_tolerance = 0.3
		tolerance_range_right = 80
		tolerance_range_left = int(tolerance_range_right / 1.5)
		tolerance_range_signal = 23  # length of the signal itself, which should not exceed this value
		offset = 12
		i = len(data['Envelop']) - 2
		found = False

		while i > emit_end + 10:
			# searching for for the slope increase
			# decrease tolerance to get more non-steep slope
			if self.check_slope(data, i - 2, i, tolerance - 2) == -1:
				# it is decreasing get the end of signal
				end = i
				i -= 5

				# searching for peak
				# print "starting Loop, i = ", i
				peak = 0, 0
				while np.abs(data['Envelop'][i] - saturation) > tolerance and i > emit_end:
					if data['Envelop'][i] > peak[1]:
						peak = i, data['Envelop'][i]
					i -= 1
				# if peak not be found
				if peak[1] == 0:
					i -= 1
					# print "Cannot find peak, i = ", i
					continue
				# starting index
				start = i
				# tolerance is based on the peak of of the signal and the percentage of noise allowed
				tole_silent = int((peak[1] - saturation) * noise_percent) + 1

				# need long stability afterwards and offset = 12 to avoid mistaken of the first decreasing index
				# right_silent is important and required for reflected signal
				right_silent = self.check_silent(data, end, tolerance_range_right, tole_silent, saturation, offset,
												 True)

				# left_silent is less important and may be used if there is multiple reflected signal
				# left_silent required less range and higher tole_silent since they encouter noise
				# If left_silent is False, consider, find the peak in the left side and compared with peak
				# if peak is larger, consider as reflected.
				left_silent = self.check_silent(data, start, tolerance_range_left, tole_silent * 1.5, saturation,
												False)
				if not left_silent:
					max_noise = start, data['Envelop'][start]
					while i > start - tolerance_range_left:
						if data['Envelop'][i] > max_noise[1]:
							max_noise = i, data['Envelop'][i]
						i -= 1
					if max_noise[1] - saturation > (peak[1] - saturation) * noise_envelop_tolerance:
						# when the noise larger than the peak, not regard as signal
						# print "Noise so large at %d %d, peak = %d %d" %(max_noise[0], max_noise[1], peak[0], peak[1])

						continue
				# print "Set: end %d start %d peak %d silent %d" % (end, start, peak[0], right_silent)

				# if there is no right_silent, it is not the signal
				if not right_silent:
					start = 0
					end = 0
					peak = 0, 0
					i -= 1
					continue
				found = True
				item = {'start': start, 'end': end, 'peak': peak}
				signals.insert(0, item)

				print "Found signal ", start, peak, end
			else:
				# if i % 100 == 0:
				#    print "Pass ", i
				i -= 1

		# if found evaluate all the reflected signals, from the set signals
		peakOverwhelm = 0.6  # if the first signal's peak smaller than 0.6 the second, consider the second
		maxIndex = 0
		if found:
			maxPeak = signals[0]['peak'][1]
			compareItem = None
			for item in signals:
				if item['peak'][1] > maxPeak:
					maxPeak = item['peak'][1]
					compareItem = item
			if compareItem is None or signals[0]['peak'][1] > peakOverwhelm * maxPeak:
				end = signals[0]['end']
				start = signals[0]['start']
			else:
				end = compareItem['end']
				start = compareItem['start']

			if (end - start) < tolerance_range_signal:
				return start, end
			else:
				return 0, 0
		else:
			return 0, 0

	# if define the monotonicity,
	def check_slope(self, data, start, end, tolerance):
		# check if it is monotonous
		mono = True
		max_val = start, data['Envelop'][start]
		min_val = start, data['Envelop'][start]

		for x in xrange(start, end + 1, 1):
			if data['Envelop'][x] > max_val[1]:
				max_val = x, data['Envelop'][x]
			if data['Envelop'][x] < min_val[1]:
				min_val = x, data['Envelop'][x]
		if start < max_val[0] < end:
			mono = False
		if start < min_val[0] < end:
			mono = False

		if not mono:
			return 0
		if data['Envelop'][end] - data['Envelop'][start] > tolerance:
			return 1
		elif data['Envelop'][end] - data['Envelop'][start] < -1 * tolerance:
			return -1
		return 0

	def check_silent(self, data, i, tolerance_length=30, tolerance=5, saturation=51, offset=0, forward=True):
		start = 0
		end = 0
		if forward:
			start = i + offset
			end = i + tolerance_length
			if end >= len(data['Envelop']):
				end = len(data['Envelop']) - 1
		else:
			start = i - tolerance_length
			if start < 0:
				start = 0
			end = i - offset
		silent = True
		for j in xrange(start, end, 1):
			if np.abs(data['Envelop'][j] - saturation) > tolerance:
				silent = False
				return silent

		return silent

	def check_peak(self, data, i, check_range=10, tolerance_length=5, tolerance=5):
		right_slope = self.check_slope(data, i + tolerance_length, i + tolerance_length + check_range, tolerance)
		left_slop = self.check_slope(data, i - tolerance_length - check_range, i - tolerance_length, tolerance)
		if right_slope == -1 and left_slop == 1:
			# peak
			return 1
		elif right_slope == 1 and left_slop == -1:
			# trough
			return -1
		else:
			# nothing
			return 0

	# Load the valid ID file
	def load_validid(self, path=''):
		if len(self.settings['valid-id-file']) < 0:
			return
		if path == '':
			path = self.root
		_fname = os.path.join(path, self.settings['valid-id-file'])
		if os.path.isfile(_fname):
			with open(_fname, 'r') as _f:
				while True:
					_s = _f.readline()
					if _s == '':
						break
					if len(_s) < 3:
						continue
					_s = _s.strip()
					if _s[0] == '#':
						continue  # comment
					_tmp = _s.split(',')
					self.validid[_tmp[0]] = (int(_tmp[1]), int(_tmp[2]))

	# Check if ID is valid
	def check_valid_id(self, obj, filename):
		if not self.validid.has_key(filename):
			# no valid id for this filename, assumed all valid
			return True
		if not obj.has_key('id'):
			return True
		#print "CurrentID: %d, check within %d - %d" % (obj['id'], self.validid[filename][0], self.validid[filename][1])
		if obj['id'] < self.validid[filename][0] + 1:
			print "Sample %d < %d  -- ignored." % (obj['id'], self.validid[filename][0])
			return False
		if obj['id'] > self.validid[filename][1] - 1:
			print "Sample %d > %d  -- ignored." % (obj['id'], self.validid[filename][1])
			return False
		# check if the remaining ID has reflected signal or not, then ignore
		return True

	# plot graph for valid id
	def plot_graph(self, data):
		fig, ax1 = plt.subplots()

		_color = 'blue'
		_width = 2.0
		ax1.plot(data["Timing"], data["Envelop"], color=_color, linewidth=_width)
		ax1.set_ylim(0, 200)
		ax1.set_xlabel('Time (ms)')
		ax1.set_ylabel('Envelop', color=_color)

		if self.settings['distance_marker']:
			_initIndex = data["Working"]["txPulseStart"]
			_refIndex = data["Working"]["refPulseStart"]
			_initTime = data["Timing"][_initIndex]
			_refTime = data["Timing"][_refIndex]
			_dist = str('%.3f' % self.get_dist(_initTime, _refTime)) + ' m'

			_color = 'red'
			_width = 1.0
			plt.axvline(x=_initTime, color=_color, linewidth=_width, linestyle='dashed')
			plt.axvline(x=_refTime, color=_color, linewidth=_width, linestyle='dashed')
			ax1.text((_refTime + _initTime) / 2, 12, _dist, ha='center', color=_color)
			ax1.annotate('', (_initTime, 10), (_refTime, 10), arrowprops=dict(arrowstyle='<->', color=_color, linewidth=_width, linestyle='dashed'))

		_title = 'ID-' + str(data["DataID"]) + '_' + data["filename"][:-5]
		plt.title(_title)
		plt.savefig(os.path.join(self.root, _title.replace('.', ',')))
		plt.close('all')


# prepare argparse to read command from user input
# all parameters can be changed without modified config file
def read_parser():
	parser = argparse.ArgumentParser(description='Convert data to json and plot graph',
									 formatter_class=RawTextHelpFormatter)

	# adding argument (SectionOne)
	parser.add_argument('-dir', dest="inputdir", required=True, help="Directory contain json file(s)")
	parser.add_argument('-c', dest='config', default=os.path.join(os.getcwd(), 'config.ini'),
						help="Getting config file")
	parser.add_argument('-d', dest='actualdist', default="3", type=float,
						help="Setting for actual horizontal distance (use for wave output). \n\t[default: %(default)s]")
	parser.add_argument('-g', dest='actualheight', default="3", type=float,
						help="Setting for actual height of curb/pole (use for wave output). \n\t[default: %(default)s]")
	parser.add_argument('-t', dest='temperature', default="22", type=float,
						help="Setting temperature information. \n\t[default: %(default)s]")
	parser.add_argument('-w', dest='winsize', default="100", type=int,
						help="Windows size to ignore envelop exist threshold. \n\t[default: %(default)s]")
	parser.add_argument('-e', dest='threshold', default="5", type=float,
						help="Setting envelop threshold to trigger soundwave detection. \n\t[default: %(default)s]")
	parser.add_argument('-a', dest='averaging', default="3", type=int,
						help="Number of previous sampling use for threshold calculation. \n\t[default: %(default)s]")

	# adding argument (SectionTwo)
	parser.add_argument('-od', default=True, action="store_false",
						help="Output Detail, Output info to *-det.txt. \n\t[default: %(default)s]")
	parser.add_argument('-ow', default=True, action="store_false",
						help="Output Wave, Output only wave with signal (special for NUS). \n\t[default: %(default)s]")
	parser.add_argument('-og', default=True, action="store_false",
						help="Output Graph, Output all graph. \n\t[default: %(default)s]")
	parser.add_argument('-sg', default=False, action="store_true",
						help="Show Graph, Show graph when run. \n\t[default: %(default)s]")
	parser.add_argument('-sa', default=False, action="store_true",
						help="Secondary Axis, Plot secondary axis (in this case delta-time). \n\t[default: %(default)s]")
	parser.add_argument('-dm', default=True, action="store_false",
						help="Distance Marker, Show distance calculation marker during plot. \n\t[default: %(default)s]")
	parser.add_argument('-cp', default=True, action="store_false",
						help="Combine signal from all cycle into single graph. \n\t[default: %(default)s]")
	parser.add_argument('-cw', default=True, action="store_false",
						help="Combine output reflected envelop to a file. \n\t[default: %(default)s]")
	parser.add_argument('-ig', default=False, action="store_true",
						help="ignore plotting the graph of no reflected signal. \n\t[default: %(default)s]")

	# output filename
	parser.add_argument('-out', dest='outfilename', default=os.path.join(os.getcwd(), 'processed.data'),
						help="Output Filename")
	# end pulse method
	parser.add_argument('-ep', dest='endpulse', default=1, type=int,
						help="End Pulse method: 0=averaging, 1=immediate, 2=gradient")
	# valid sample id
	parser.add_argument('-vid', dest='validid', default="validid.csv",
						help="CSV file containing valid ID range (inclusive) for each raw json file")
	parser.add_argument('-test', dest='testing', default=False, action='store_true',
						help="testing when we know the distance")

	return parser.parse_args()


# since there have two ways to configure, if user insert command, need to overwrite parameter
# read from config file to user setting
def update_value(cmd, args, _dict):
	if '-d' in cmd:
		_dict['actual_dist'] = args.actualdist
	if '-g' in cmd:
		_dict['actual_height'] = args.actualheight
	if '-t' in cmd:
		_dict['temperature'] = args.temperature
	if '-w' in cmd:
		_dict['windows'] = args.winsize
	if '-e' in cmd:
		_dict['threshold'] = args.threshold
	if '-a' in cmd:
		_dict['averaging'] = args.averaging
	if '-od' in cmd:
		_dict['output_detail'] = args.od
	if '-ow' in cmd:
		_dict['output_wave'] = args.ow
	if '-og' in cmd:
		_dict['output_graph'] = args.og
	if '-sg' in cmd:
		_dict['show_graph'] = args.sg
	if '-sa' in cmd:
		_dict['secondary_axis'] = args.sa
	if '-dm' in cmd:
		_dict['distance_marker'] = args.dm
	if '-cp' in cmd:
		_dict['combine_plot'] = args.cp
	if '-cw' in cmd:
		_dict['combine_wave'] = args.cw
	if '-ep' in cmd:
		_dict['endpulse'] = args.endpulse
	if '-out' in cmd:
		_dict['outfilename'] = args.outfilename
	if '-vid' in cmd:
		_dict['validid'] = args.validid
	if 'ig' in cmd:
		_dict['ignore_reflected'] = args.ig
	if '-test' in cmd:
		_dict['test'] = args.testing
	return _dict


if __name__ == "__main__":
    args = read_parser()

    Config = ConfigParser.ConfigParser()
    Config.read(os.path.join(os.getcwd(), args.config))

    _dict = {}
    for section in Config.sections():
        if 'Section' in section:
            _dict = configSectionMap(Config, section, _dict)

    _dict = update_value(sys.argv[1:], args, _dict)
    _pathname = os.path.abspath(args.inputdir)


    # main operation
    if os.path.isfile(_pathname):
        _root = os.path.dirname(_pathname)
        _jsonf = os.path.basename(_pathname)
        _json = JSONProcessor(Config, _dict, _root)
        if _json is None:
            print "Error: Failed to initialize JSON Class!"
            exit(1)
        pdata = _json.load_file(_jsonf)
        with open(args.outfilename, 'w') as outf:
            for d in pdata:
                outf.write(json.dumps(d) + '\n')
    else:
        _json = JSONProcessor(Config, _dict, _pathname)
        if _json is None:
            print "Error: Failed to initialize JSON Class!"
            exit(1)
        _fnames = []
        with open(args.outfilename, 'w') as outf:
            for root, dirs, files in os.walk(_pathname):
                _json.load_validid(path=root)
                for jsonf in files:
                    if jsonf.endswith(".json"):
                        pdata = _json.load_file(jsonf)
                        for d in pdata:
                            outf.write(json.dumps(d) + '\n')
                        _fnames.append(jsonf)
            outf.write(json.dumps(_json.get_metadata({'raw-json-files': _fnames})) + '\n')
