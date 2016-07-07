#!/bin/env python2

import sys, json, os
import matplotlib.pyplot as plt
import ConfigParser, argparse
from argparse import RawTextHelpFormatter
import numpy as np

class singleReflect (object):
	def __init__(self, envelop, time, _max):
		self.envelop = envelop
		self.time = time
		self._max = _max
		self.averaging = 3
	
	def get_pulse_width(self):
		if not self._max == 0:
			for i, x in enumerate(self.envelop):
				if i > self._max:
					_ave = 0
					#print "[%d] - %d (%.3f)" % (i, x, self.time[i])
					_ave = x
					for y in xrange(self.averaging):
						_ave += self.envelop[i - (y + 1)]
				
					_value = _ave / (self.averaging + 1)
					if _value < 55:
						pulse_width = i - self.averaging - self._max
						#print "pulse width: %d - %d - %d = %d" % (i, self.averaging, self._max, pulse_width)
						_time_diff = self.time[self._max + pulse_width - 1] - self.time[self._max]
						return pulse_width, _time_diff
		else:
			return -1
	
	def get_ref_env(self, startIndex, width):
		self._list = []
		self.time_list = []
		for i, x in enumerate(self.envelop[startIndex:startIndex+width]):
			self.time_list.append(self.time[startIndex + i])
			self._list.append(x)
			
		return self._list, self.time_list

	def get_peak(self, rev=None):
		if not rev is None:
			_np = np.array(rev)
		else:
			_np = np.array(self._list)
		if np.size(_np) > 0:
			return np.amax(_np), np.amin(_np)
		else:
			return None, None
	
	def cal_area(self, dtime, e1, e2):
		return float(e1+e2) / 2. * dtime
	
	def get_area(self, peak=None):
		_area = 0.
		self.areaA = 0.
		if not peak is None:
			for i, x in enumerate(self._list):
				if i > 0:
					if self._list[i] == peak:
						if self._list[i] == self._list[i-1]:
							continue
						_area += self.cal_area((self.time_list[i] - self.time_list[i-1]), self._list[i], self._list[i-1])
						self.areaA = _area
						continue
					_area += self.cal_area((self.time_list[i] - self.time_list[i-1]), self._list[i], self._list[i-1])
					#print "(%d + %d)/2*(%.3f - %.3f) = %.3f" %(self._list[i], self._list[i-1], self.time_list[i], self.time_list[i-1], _area)
			
			#handle situation where all value are equal to peak
			if len(self._list) == self._list.count(peak):
				self.area = (self.time_list[len(self._list)-1] - self.time_list[0]) * peak
				self.areaA = self.area
		
		self.area = _area
		return self.area
						
	def get_area_ratio(self):
		print "Total Area: %.3f, AreaA: %.3f" % (self.area, self.areaA)
		self.areaB = self.area - self.areaA
		if self.areaB == 0:
			self.ratio = self.areaA / self.area
		else:
			self.ratio = self.areaA / self.areaB
		print "AreaB: %.3f, Ratio: %.3f" % (self.areaB, self.ratio)
		return self.ratio

class COMBINE (object):
	def __init__(self, combinePlot, data, root, comName):
		self.combinePlot = combinePlot
		self.data = data
		self.root = root
		self.comName = comName

	def insert(self, _min, _max, _time, _envelop, title):
		self._min = _min
		self._max = _max
		self.time = _time
		self.envelop = _envelop
		self.allplotTitle = title
		
	def update(self, index, data):
		self.data = data
		
		if not self._min == 0:
			_xmin = self.time[self._min]
			_in = str(index)
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
			
			_t = []
			_e = []
			_ot = []
			_oe = []
			for _index, enve in enumerate(self.envelop):
				_ot.append(self.time[_index])
				_oe.append(self.envelop[_index])
				if _index >= self._min:
					_t.append(self.time[_index] - _xmin)
					_e.append(self.envelop[_index])
			
			self.data[_oritKey] = _ot
			self.data[_orieKey] = _oe
			self.data[_timeKey] = _t
			self.data[_envKey] = _e
			self.data[_minKey] = self._min
			self.data[_maxKey] = self._max	
			
			if not self._max == 0:
				sr = singleReflect(self.envelop, self.time, self._max)
				self.data[_widthKey], self.data[_diffKey] = sr.get_pulse_width()								# get reflect pulse width
				self.data[_refKey], self.data[_refTimeKey] = sr.get_ref_env(self._max, self.data[_widthKey])	# get reflect pulse envelop
				self.data[_peakKey], self.data[_lowKey] = sr.get_peak(rev=self.data[_refKey])				# get reflest pulse peak
				if not self.data[_peakKey] is None:
					self.data[_areaKey] = sr.get_area(peak=self.data[_peakKey])					# get reflect pulse area
					self.data[_ratioKey] = sr.get_area_ratio()													# get reflect pulse area ratio
				else:
					self.data[_areaKey] = None
					self.data[_ratioKey] = None
			else:
				print "Return as no reflected signal detected"
		
			#self.plot_single(index)
		return self.data
	
	def plot_single(self, index):
		fig, ax = plt.subplots()

		_max_envelop = 0
		_min_envelop = 100
		
		_in = str(index)
		_timeKey = 't' + _in
		_envKey = 'e' + _in
		_minKey = 'min' + _in
		_maxKey = 'max' + _in	
		
		# relfect signal characteristic
		_widthKey = 'w' + _in
		_ratioKey = 'r' + _in
		_peakKey = 'p' + _in
		_lowKey = 'l' + _in
		_areaKey = 'a' + _in	
			
		if self.data.has_key(_timeKey) and self.data.has_key(_envKey):
			plt.plot(self.data[_timeKey], self.data[_envKey])
			
			if self.data[_maxKey] == 0:
				print "Skip range calc for [%s] graph as no reflected signal detected" % x
			else:
				for x in self.data[_envKey][(self._max - 10 - self.data[_minKey]):]:
					if x > _max_envelop:
						_max_envelop = x
					if x < _min_envelop:
						_min_envelop = x
				
				# print vertical range
				plt.axvline(x=self.time[self._max-self._min], color='r', linewidth=1., linestyle='dashed')
				plt.axvline(x=self.time[self._max + self.data[_widthKey] - self._min], color='r', linewidth=1., linestyle='dashed')
				#ax.set_xlim(self.time[self._max-10], self.time[self._max+25])
				#ax.set_ylim(self.data[_lowKey] - 20, self.data[_peakKey] + 20)
				#plt.show()
		else:
			print("No Key for %s or %s" % (_timeKey, _envKey))	
		plt.close('all')	
	
	def output_json(self):
		# output data to self.comName
		if self.combinePlot:
			with open(os.path.join(self.root, self.comName), "w") as fp:
				json.dump(self.data, fp)
	
	def plot(self):
		# calculate average of _max index
		list_min = []
		list_max = []
		_max_envelop = 0
		_min_envelop = 100
				
		fig, ax = plt.subplots()
		_legend = []
		
		# calculate average min and max time for all cycle
		_zero_count = 0
		for x in self.data['index']:
			_maxKey = 'max' + str(x)
			_minKey = 'min' + str(x)
			if self.data.has_key(_maxKey):
				if self.data[_maxKey] == 0:
					_zero_count += 1
				list_max.append(self.data[_maxKey])
				list_min.append(self.data[_minKey])
		
		_ave_min_index = reduce(lambda x, y: x + y, list_min) / len(list_min)
		_hv_list = len(list_max)- _zero_count
		if _hv_list > 0:
			_ave_max_index = (reduce(lambda x, y: x + y, list_max) / _hv_list) - _ave_min_index
		else:
			print "None of reflected signal detected for all cycles"
			self.output_json()
			return

		# plot all signal in single graph
		for x in self.data['index']:
			_timeKey = 't' + str(x)
			_envKey = 'e' + str(x)
			_minKey = 'min' + str(x)
			_maxKey = 'max' + str(x)
				
			if self.data.has_key(_timeKey) and self.data.has_key(_envKey):
				_legend.append(str(x))
				plt.plot(self.data[_timeKey], self.data[_envKey])
				
				if self.data[_maxKey] == 0:
					print "Skip range calc for [%s] graph as no reflected signal detected" % x
					continue
				
				for x in self.data[_envKey][(_ave_max_index - 10 - self.data[_minKey]):]:
					if x > _max_envelop:
						_max_envelop = x
					if x < _min_envelop:
						_min_envelop = x
			else:
				print("No Key for %s or %s" % (_timeKey, _envKey))

		ax.set_ylim(0, 200)
		ax.set_xlabel('Time (ms)')
		ax.set_ylabel('Envelop', color='b')
		
		# plot all signal plot into single graph
		plt.title(self.allplotTitle)
		plt.legend(_legend, loc='upper right')
		plt.savefig(os.path.join(self.root, self.allplotTitle.replace('.', ',')))
		
		# zoom in reflection signal
		plt.title(self.allplotTitle + '_rezoom')
		ax.set_xlim(self.time[_ave_max_index-10], self.time[_ave_max_index+25])
		ax.set_ylim(_min_envelop - 20, _max_envelop + 20)
		plt.savefig(os.path.join(self.root, self.allplotTitle.replace('.', ',') + '_rezoom'))
		
		plt.close('all')
		
		self.output_json()
