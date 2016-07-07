#!/bin/env python2

# -------------------------
# feature.py
#	Adapted from Kian Soon's json2graph.py to use feature.py instead
# -------------------------


import sys, json, os
import matplotlib.pyplot as plt
import ConfigParser, argparse
from argparse import RawTextHelpFormatter
from feature import COMBINE
import numpy as np
import time
import thread


# Convenience: config or default value
def get_cfg(cfg, k, default):
	if cfg.has_key(k) and (cfg[k] != None):
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


# Json Class
class JSON_EXE(object):
	def __init__(self, config, _dict, root):
		self.config = config
		self.cfg = _dict
		self.root = root

		self.RECV_SEN = 0
		self.recv = 0
		self.fir = 0

		self.temperature = float(get_cfg(self.cfg, 'temperature', '22'))
		self.threshold = float(get_cfg(self.cfg, 'threshold', '5'))
		self.window = float(get_cfg(self.cfg, 'windows', '100'))
		self.averaging = int(get_cfg(self.cfg, 'averaging', '3'))
		self.outputDetail = convert_cfg(get_cfg(self.cfg, 'output_detail', 'True'))
		self.outputWave = convert_cfg(get_cfg(self.cfg, 'output_wave', 'True'))
		self.actualDist = float(get_cfg(self.cfg, 'actual_dist', '3'))
		self.actualHeight = float(get_cfg(self.cfg, 'actual_height', '3'))
		self.outputGraph = convert_cfg(get_cfg(self.cfg, 'output_graph', 'True'))
		self.showGraph = convert_cfg(get_cfg(self.cfg, 'show_graph', 'True'))
		self.SecondaryAxis = convert_cfg(get_cfg(self.cfg, 'secondary_axis', 'False'))
		self.DistanceMarker = convert_cfg(get_cfg(self.cfg, 'distance_marker', 'True'))
		self.combineWave = convert_cfg(get_cfg(self.cfg, 'combine_wave', 'True'))
		self.combinePlot = convert_cfg(get_cfg(self.cfg, 'combine_plot', 'True'))
		self.ignore_reflected = convert_cfg(get_cfg(self.cfg, 'ignore_reflected', 'False'))

		self.graphTitle = ""

	# setting name and open required files
	def set_name(self, basename):
		self.csvName = basename.replace(".json", ".csv")
		self.detName = basename.replace(".json", "-det.txt")
		self.wavName = basename.replace(".json", "-wave.csv")
		self.comName = 'com-' + basename.replace(".json", "-com.csv")

		self.inf = open(os.path.join(self.root, basename), "r")
		self.csvf = open(os.path.join(self.root, self.csvName), "w")
		if self.outputDetail:
			self.detf = open(os.path.join(self.root, self.detName), "w")
		if self.outputWave:
			self.wavf = open(os.path.join(self.root, self.wavName), "w")

		self.data = []
		if self.combinePlot:
			self.combine = COMBINE(self.config, self.root, self.comName)

	# closing json file object
	def close(self):
		if self.combinePlot:
			self.combine.plot()

		self.inf.close()
		self.csvf.close()
		if self.outputDetail:
			self.detf.close()
		if self.outputWave:
			self.wavf.close()

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

	# main running module
	def execution(self, _index, obj):
		plt.close('all')
		if obj == None:
			return
		if obj.has_key("sensors"):
			# setting of rpi
			# output as comment
			self.csvf.write("Sensors,%s\n" % (",".join('%d' % x for x in obj['sensors'])))
			self.csvf.write("Timings,%s\n" % (
				",".join('%d' % obj[x] for x in ['T-ranging', 'T-sampling', 'T-next', 'T-cycle', 'T-sample'])))

			if self.outputDetail:
				self.detf.write("Sensors,%s\n" % (",".join('%d' % x for x in obj['sensors'])))
				self.detf.write("SoundSpeed,%.1f,%.1f\n" % (self.temperature, (331.4 + .61 * (self.temperature))))
				self.detf.write("Threshold,%.1f\n" % self.threshold)
				self.detf.write("Windows,%d\n" % self.window)
				self.detf.write("Averaging,%d\n" % self.averaging)
				self.detf.write("ActualDistance,%.2f\n" % self.actualDist)
				self.detf.write("ActualHeight,%.2f\n" % self.actualHeight)
				self.detf.write("Timings,%s\n" % (
					",".join('%d' % obj[x] for x in ['T-ranging', 'T-sampling', 'T-next', 'T-cycle', 'T-sample'])))

			self.graphTitle = ''
			_buf = "%s" % ("-".join('%d' % x for x in obj['sensors']))
			self.allplotTitle = self.csvName.replace('.csv', '_' + _buf) + '_all'
			self.graphTitle = self.csvName.replace('.csv', '_' + _buf) + '_' + str(_index)
		else:
			# should be sensor data
			"""
				EDITING FOR AIS SENSOR-JSON FILE
			self.csvf.write("Start,%d,%d\n" % (obj['start'][0], obj['start'][1]))
			self.csvf.write(",delta-time(ms),time(ms),Envelop\n")
			"""

			if self.outputDetail:
				self.detf.write("File," + self.graphTitle + '_' + str(_index) + '\n')
				self.detf.write("Start,%d,%d\n" % (obj['start'][0], obj['start'][1]))

			"""Set timing and envelop"""
			t = 0.0
			self.dtime = []
			self.time = []
			self.envelop = []
			"""
			for d in obj['samples']:
				td = d[0] / 1000.0
				t += td
				self.csvf.write(",%.3f,%.3f,%d\n" % (td, t, d[1]))
				self.dtime.append(td)
				self.time.append(t)
				self.envelop.append(float(d[1]))
			"""
			for i in xrange(len(obj['timing'])):
				if i == 0:
					td = obj['timing'][i]
				else:
					td = obj['timing'][i] - obj['timing'][i - 1]
				t += td
				self.dtime.append(td)
				self.time.append(t)
				self.envelop.append(float(obj['envelop'][i]))

			self.graphTitle = self.csvName.replace(".csv", "")
			_split = self.graphTitle.split('_')
			"""
			_recv_sen = obj['recv-sensor']
			_firing_sen = obj['firing-sensor']
			if self.recv != _recv_sen or self.fir != _firing_sen:
				self.recv = _recv_sen
				self.fir = _firing_sen
			if self.recv != self.RECV_SEN:
				_index = 0
				self.RECV_SEN = self.recv
			if self.fir == 0:
				_index += 1
			"""
			# self.graphTitle = _split[0] + '_SEQ-' + str(_index) + '_' + str(self.recv) + '-' + str(self.fir)
			# self.graphTitle = _split[0] + '_SEQ-' + str(obj['id']) + '_' + str(self.recv) + '-' + str(self.fir)
			self.graphTitle = _split[0] + '_SEQ-' + str(_index)
			self.mat_plot(_index)

		return _index

	# read config file by section and parameter
	def read_config(self, section):
		color = configSectionMap(self.config, section)['color']
		width = configSectionMap(self.config, section)['width']
		label = configSectionMap(self.config, section)['label']
		return color, float(width), label

	# perform graph plotting
	def mat_plot(self, index):
		# calculate threshold index range
		_min, _max = self.cal_range2()
		if _max == 0:
			# no reflected signal, put ignored in to the graph.titlve
			self.graphTitle += 'EXCLUDED'
			if self.ignore_reflected:
				pass

		fig, ax1 = plt.subplots()
		# Print for envelop axis
		_color, _width, _label = self.read_config('FirstAxisProp')
		ax1.plot(self.time, self.envelop, color=_color, linewidth=_width)
		ax1.set_ylim(0, 5000)
		ax1.set_xlabel('Time (ms)')
		ax1.set_ylabel(_label, color=_color)
		for t1 in ax1.get_yticklabels():
			t1.set_color(_color)

		# print for delta time axis
		if self.SecondaryAxis:
			_color, _width, _label = self.read_config('SecondAxisProp')
			ax2 = ax1.twinx()
			ax2.plot(self.time, self.dtime, color=_color, linewidth=_width)
			ax2.set_ylabel(_label, color=_color)
			ax2.autoscale(False)
			for t1 in ax2.get_yticklabels():
				t1.set_color(_color)

			# calculate threshold index range
			# _min, _max = self.cal_range()

		# print distance marker
		if self.DistanceMarker:
			# print self.graphTitle, _min, _max
			if not _min == 0 and not _max == 0:
				_xmin = self.time[_min]
				_xmax = self.time[_max]

				_dist = str('%.3f' % self.get_dist(_xmin, _xmax)) + ' m'
				if self.outputDetail:
					self.detf.write("Min,%.3f,%.3f,%d\n" % (self.dtime[_min], _xmin, self.envelop[_min]))
					self.detf.write("Max,%.3f,%.3f,%d\n" % (self.dtime[_max], _xmax, self.envelop[_max]))
					self.detf.write("Dist,%s\n" % _dist)

				_color, _width, _label = self.read_config('MarkerProp')
				# print vertical range
				plt.axvline(x=_xmin, color=_color, linewidth=_width, linestyle='dashed')
				plt.axvline(x=_xmax, color=_color, linewidth=_width, linestyle='dashed')

				# print distance arrow line
				if 'True' in _label:
					ax1.text((_xmax + _xmin) / 2, 12, _dist, ha='center', color=_color)
				ax1.annotate('', (_xmin, 10), (_xmax, 10),
							 arrowprops=dict(arrowstyle='<->', color=_color, linewidth=_width, linestyle='dashed'))
			elif not _min == 0:
				if self.outputDetail:
					self.detf.write("Min,%.3f,%.3f,%d\n" % (self.dtime[_min], self.time[_min], self.envelop[_min]))

		if self.outputWave:
			if not _min == 0:
				# for _index, enve in enumerate(self.envelop):
				#	if _index >= _min:
				#		self.wavf.write("%.3f\t%d\t%.1f\n" % (self.time[_index], enve, self.actualDist))
				# self.wavf.write("\n")
				''' second version of NUS output '''
				if not _max == 0:
					_ref = int(_max) - 5
					for _index, enve in enumerate(self.envelop):
						if _index >= _ref and _index < (_ref + 50):
							self.wavf.write("%.3f\t%d\t%.1f\t%.1f\n" % (
								self.time[_index], enve, self.actualDist, self.actualHeight))
					self.wavf.write("\n")

		# phi: add "excluded to the title
		plt.title(self.graphTitle)
		if self.outputGraph:
			plt.savefig(os.path.join(self.root, self.graphTitle.replace('.', ',')))
		if self.showGraph:
			plt.show()
		plt.close('all')

		if self.combinePlot:
			self.combine.insert(_min, _max, self.time, self.envelop, self.allplotTitle)
			self.data = self.combine.update()

	# calculate distance
	def get_dist(self, xmin, xmax):
		_sound_speed = 331.4 + .61 * (self.temperature)
		_travel_time = (xmax - xmin) / 1000

		return _sound_speed * _travel_time / 2

	# threshold should vary based on distance
	# need to explore more on this
	def get_threshold(self, emax, ecur):
		_start = self.time[emax]
		_to = self.time[ecur]

		if _start == _to:
			return 0.

		_dist = self.get_dist(_start, _to)
		_thres = (1. / _dist) * 10
		# if _to > 26. or _to < 28.:			# 4m
		# if _to > 20.5 or _to < 22.5:		# 3m
		#	_thres = 1.3
		# print "Threshold: ", _thres
		return _thres

	# calculate index range for start and stop peak
	def cal_range(self):
		_first_index = 0
		_second_index = 0

		for i, x in enumerate(self.envelop):
			_ave = 0
			for y in xrange(self.averaging):
				_ave += self.envelop[i - (y + 1)]

			_value = x - (_ave / self.averaging)

			# print '***'
			# print "[%.3f] - %d\taverage: %.3f" % (self.time[i], self.envelop[i], _value)
			if _first_index == 0:
				if _value >= 5:
					_first_index = i
			elif _value >= self.get_threshold(_first_index, i):
				# sometime value will change from 52 -> 54 and cause trigger of threshold
				#	this is to make sure if threshold trigger, envelop should more than 55
				if self.envelop[i] <= 55:
					continue
				if (i - _first_index) > self.window:
					_second_index = i
					break
		# handling error when change for _max too close to end
		if (len(self.envelop) - _second_index) < 10:
			_second_index = 0

		# phi: the relfected signal must be followed by a serials of blank signal
		# reflected range is the sequence of consecutive positive signal starting
		# Any discrete signal should be ignored.

		return _first_index, _second_index

	# version 2 to calculate and find the valid signal base on properties
	# 1/The right side must be silent with saturation (adjust by % of noise tolerance to the peak, length, and offset)
	# 2/The highest peak in the vicinity - find the peak of the signal
	# 3/The left side should be silent enough but also be filled with noise
	# **** version 3 to be developed to decided the correct signal with there are multiple valid signal
	def cal_range2(self):
		start_index = 0
		start_index_end = 0
		end_index = 0
		end_index_end = 0
		tolerance = 75
		emitting_envelop_min_diff = 1200
		saturation = 0

		initial_test_range = self.averaging * 10
		for x in xrange(initial_test_range):
			saturation += self.envelop[x]
		saturation /= (initial_test_range)
		tolerance_reflected = 30

		# print "\nplotting %s" % self.graphTitle
		# check if the begining is flat, if not, it fluctuate so much to determined the reflected signal
		if not self.check_silent(0, initial_test_range, tolerance, saturation, 0, True):
			# print "Fluctuate too much to measure reflected signal"
			return 0, 0

		# finding the range of emitting signal
		for i, self.envelop[i] in enumerate(self.envelop):
			_ave_advance = 0
			_ave_behind = 0
			for y in xrange(self.averaging):
				_ave_advance += self.envelop[i - (y + 1)]
				if (i + y + 1) >= len(self.envelop):
					_ave_behind = self.envelop[i]
				else:
					_ave_behind += self.envelop[i + (y + 1)]

			# _diff = (_ave_behind / self.averaging) - (_ave_advance / self.averaging)
			if i < len(self.envelop) - 1:
				_diff = self.envelop[i + 1] - self.envelop[i]

			if start_index == 0:
				if _diff >= emitting_envelop_min_diff:
					start_index = i
			elif start_index_end == 0:
				# looking for the end of the emitting signal
				# end when the subsequent is silent for half the tolerance reflected / 2
				if self.check_silent(i, tolerance_reflected + 10, tolerance + 20, saturation, 0, True):
					start_index_end = i
					break

		# print "starting: %d %d" % (start_index, start_index_end)
		# finding the range, peak, of reflected signal
		# This signal is followed by a serials of silence. The pattern before it usually silent
		# if there is noise very close the the signal, they cannot be larger than the relected signal
		# finding the feature vector of signal
		if start_index == 0:
			return 0, 0
		end_index, end_index_end = self.get_reflected_signal(start_index_end, tolerance, saturation)
		print "---min = %d %d, max = %d %d, data size = %d" % (
			start_index, start_index_end, end_index, end_index_end, len(self.envelop))

		# print "finally: start = %d %d , end = %d %d" % (start_index, start_index_end, end_index, end_index_end)
		return start_index, end_index

	# Get the reflected signal
	def get_reflected_signal(self, emit_end, tolerance, saturation):
		# features of signal
		signals = []
		start = 0
		end = 0
		peak = 0, 0  # index and value
		noise_percent = 0.15
		noise_envelop_tolerance = 0.3
		tolerance_range_right = 80  # interval to the right to check silent
		tolerance_range_left = int(tolerance_range_right / 1.5)  # interval to the left to check silent
		tolerance_range_signal = 30  # length of the signal itself, should not exceed this value
		i = len(self.envelop) - 2
		offset = 10
		found = False

		while i > emit_end + 10:
			# searching for for the slope increase
			# decrease tolerance to get more non-steep slope
			if self.check_slope(i - 1, i, tolerance - 25) == -1:
				# it is decreasing get the end of signal
				end = i
				i -= 1

				# searching for peak
				# print "starting Loop, i = ", i
				while np.abs(self.envelop[i] - saturation) > tolerance and i > emit_end:
					if self.envelop[i] > peak[1]:
						peak = i, self.envelop[i]
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
				right_silent = self.check_silent(end, tolerance_range_right, tole_silent, saturation, offset, True)

				# left_silent is less important and may be used if there is multiple reflected signal
				# left_silent required less range and higher tole_silent since they encouter noise
				# If left_silent is False, consider, find the peak in the left side and compared with peak
				# if peak is larger, consider as reflected.
				left_silent = self.check_silent(start, tolerance_range_left, tole_silent * 1.5, saturation, offset, False)
				if not left_silent:
					max_noise = start, self.envelop[start]
					while i > start - tolerance_range_left:
						if self.envelop[i] > max_noise[1]:
							max_noise = i, self.envelop[i]
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
	def check_slope(self, start, end, tolerance):
		# check if it is monotonous
		mono = True
		max_val = start, self.envelop[start]
		min_val = start, self.envelop[start]

		for x in xrange(start, end + 1, 1):
			if self.envelop[x] > max_val[1]:
				max_val = x, self.envelop[x]
			if self.envelop[x] < min_val[1]:
				min_val = x, self.envelop[x]
		if start < max_val[0] < end:
			mono = False
		if start < min_val[0] < end:
			mono = False

		if not mono:
			return 0
		if self.envelop[end] - self.envelop[start] > tolerance:
			return 1
		elif self.envelop[end] - self.envelop[start] < -1 * tolerance:
			return -1
		return 0

	def check_silent(self, i, tolerance_length=30, tolerance=50, saturation=940, offset=0, forward=True):
		start = 0
		end = 0
		if forward:
			start = i + offset
			end = i + tolerance_length
			if end >= len(self.envelop):
				end = len(self.envelop) - 1
		else:
			start = i - tolerance_length
			if start < 0:
				start = 0
			end = i - offset
		silent = True
		for j in xrange(start, end, 1):
			if np.abs(self.envelop[j] - saturation) > tolerance:
				silent = False
				return silent

		return silent

	def check_peak(self, i, check_range=10, tolerance_length=5, tolerance=5):
		right_slope = self.check_slope(i + tolerance_length, i + tolerance_length + check_range, tolerance)
		left_slop = self.check_slope(i - tolerance_length - check_range, i - tolerance_length, tolerance)
		if right_slope == -1 and left_slop == 1:
			# peak
			return 1
		elif right_slope == 1 and left_slop == -1:
			# trough
			return -1
		else:
			# nothing
			return 0


# prepare argparse to read command from user input
#	all parameters can be changed without modified config file
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
	# end pulse method
	parser.add_argument('-ep', dest='endpulse', default=1, type=int,
						help="End Pulse method: 0=averaging, 1=immediate")
	return parser.parse_args()


# since there have two ways to configure, if user insert command, need to overwrite parameter
#	read from config file to user setting
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
	if '-ig' in cmd:
		_dict['ignore_reflected'] = args.ig
	return _dict


def json_generate(JSON, filename):
	_index = 0
	JSON.set_name(filename)
	print "Generating files for %s" % filename
	validId[filename] = []
	count = 0
	"""
	while True:
		s = JSON.inf.readline()
		if s == '':
			break
		if not ('{' in s):
			continue
		_index = JSON.execution(_index, JSON.get_json_object(s))
	"""
	fileString = ""
	while True:
		s = JSON.inf.readline()
		if s == '':
			break
		fileString += s
	fileString = fileString.replace("\n", "").replace(' ', "")
	jsonObject = eval(fileString)

	# extracting objects
	for count in xrange(len(jsonObject)):
		_index = JSON.execution(count, jsonObject[count])
	JSON.close()


validId = {}

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

	if os.path.isfile(_pathname):
		_root = os.path.dirname(_pathname)
		_jsonf = os.path.basename(_pathname)

		_json = JSON_EXE(Config, _dict, _root)
		if _json is None:
			print "Error: Failed to initialize JSON Class!"
			exit(1)
		json_generate(_json, _jsonf)

	else:
		_json = JSON_EXE(Config, _dict, _pathname)
		if _json is None:
			print "Error: Failed to initialize JSON Class!"
			exit(1)

		for root, dirs, files in os.walk(_pathname):
			for jsonf in files:
				if jsonf.endswith(".json"):
					json_generate(_json, jsonf)
