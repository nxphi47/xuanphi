#!/bin/env python2

import sys
import os
import matplotlib.pyplot as plt
import ConfigParser
import argparse
from argparse import RawTextHelpFormatter
from combine import COMBINE

'''

	OBSOLETE - use json2graph2.py instead

'''
# Convenience: config or default value
def get_cfg(cfg, k, default):
    if not cfg[k] is None:
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

        self.data = {}
        self.data['index'] = []
        if self.combinePlot:
            self.combine = COMBINE(self.config, self.data, self.root, self.comName)

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
            #print stmp
        return eval(stmp)

    # main running module
    def execution(self, _index, obj):
        plt.close('all')
        if obj is None:
            return
        if obj.has_key("sensors"):
            # setting of rpi
            # output as comment
            self.csvf.write("Sensors,%s\n" % (",".join('%d' % x for x in obj['sensors'])))
            self.csvf.write("Timings,%s\n" % (",".join('%d' % obj[x] for x in ['T-ranging', 'T-sampling', 'T-next', 'T-cycle', 'T-sample'])))

            if self.outputDetail:
                self.detf.write("Sensors,%s\n" % (",".join('%d' % x for x in obj['sensors'])))
                self.detf.write("SoundSpeed,%.1f,%.1f\n" % (self.temperature, (331.4 + .61 * (self.temperature))))
                self.detf.write("Threshold,%.1f\n" % self.threshold)
                self.detf.write("Windows,%d\n" % self.window)
                self.detf.write("Averaging,%d\n" % self.averaging)
                self.detf.write("ActualDistance,%.2f\n" % self.actualDist)
                self.detf.write("ActualHeight,%.2f\n" % self.actualHeight)
                self.detf.write("Timings,%s\n" % (",".join('%d' % obj[x] for x in ['T-ranging', 'T-sampling', 'T-next', 'T-cycle', 'T-sample'])))

            self.graphTitle = ''
            _buf = "%s" % ("-".join('%d' % x for x in obj['sensors']))
            self.allplotTitle = self.csvName.replace('.csv', '_' + _buf) + '_all'
            self.graphTitle = self.csvName.replace('.csv', '_' + _buf) + '_' + str(_index)
        else:
            # should be sensor data
            self.csvf.write("Start,%d,%d\n" % (obj['start'][0], obj['start'][1]))
            self.csvf.write(",delta-time(ms),time(ms),Envelop\n")

            if self.outputDetail:
                self.detf.write("File," + self.graphTitle + '_' + str(_index) + '\n')
                self.detf.write("Start,%d,%d\n" % (obj['start'][0], obj['start'][1]))

            t = 0.0
            self.dtime = []
            self.time = []
            self.envelop = []
            for d in obj['samples']:
                td = d[0] / 1000.0
                t += td
                self.csvf.write(",%.3f,%.3f,%d\n" % (td, t, d[1]))
                self.dtime.append(td)
                self.time.append(t)
                self.envelop.append(float(d[1]))

            _split = self.graphTitle.split('_')
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
            self.graphTitle = _split[0] + '_SEQ-' + str(_index) + '_' + str(self.recv) + '-' + str(self.fir)
            #self.graphTitle = _split[0] + '_SEQ-' + str(obj['id']) + '_' + str(self.recv) + '-' + str(self.fir)
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
        fig, ax1 = plt.subplots()

        # Print for envelop axis
        _color, _width, _label = self.read_config('FirstAxisProp')
        ax1.plot(self.time, self.envelop, color=_color, linewidth=_width)
        ax1.set_ylim(0, 200)
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
        _min, _max = self.cal_range()

        # print distance marker
        if self.DistanceMarker:
            #print self.graphTitle, _min, _max
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
                ax1.annotate('', (_xmin, 10), (_xmax, 10), arrowprops=dict(arrowstyle='<->', color=_color, linewidth=_width, linestyle='dashed'))
            elif not _min == 0:
                if self.outputDetail:
                    self.detf.write("Min,%.3f,%.3f,%d\n" % (self.dtime[_min], self.time[_min], self.envelop[_min]))

		if self.outputWave:
			if not _min == 0:
				#for _index, enve in enumerate(self.envelop):
				#	if _index >= _min:
				#		self.wavf.write("%.3f\t%d\t%.1f\n" % (self.time[_index], enve, self.actualDist))
				#self.wavf.write("\n")
				''' second version of NUS output '''
				if not _max == 0:
					_ref = int(_max) - 5
					for _index, enve in enumerate(self.envelop):
						if _index >= _ref and _index < (_ref + 50):
							self.wavf.write("%.3f\t%d\t%.1f\t%.1f\n" % (self.time[_index], enve, self.actualDist, self.actualHeight))
					self.wavf.write("\n")

		plt.title(self.graphTitle)
		if self.outputGraph:
			plt.savefig(os.path.join(self.root, self.graphTitle.replace('.', ',')))
		if self.showGraph:
			plt.show()
		plt.close('all')

		if self.combinePlot:
			self.data['index'].append(index)
			self.combine.insert(_min, _max, self.time, self.envelop, self.allplotTitle)
			self.data = self.combine.update(index, self.data)

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
		#if _to > 26. or _to < 28.:			# 4m
		#if _to > 20.5 or _to < 22.5:		# 3m
		#	_thres = 1.3
		#print "Threshold: ", _thres
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

			#print '***'
			#print "[%.3f] - %d\taverage: %.3f" % (self.time[i], self.envelop[i], _value)
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
		return _first_index, _second_index


# prepare argparse to read command from user input
#	all parameters can be changed without modified config file
def read_parser():
	parser = argparse.ArgumentParser(description='Convert data to json and plot graph', formatter_class=RawTextHelpFormatter)

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
	return _dict


def json_generate(JSON, filename):
	_index = 0
	JSON.set_name(filename)
	print "Generating files for %s" % filename
	while True:
		s = JSON.inf.readline()
		if s == '':
			break
		if not ('{' in s):
			continue
		_index = JSON.execution(_index, JSON.get_json_object(s))
	JSON.close()

if __name__ == "__main__":
	print 'OBSOLETE code - use json2graph2.py instead'
	exit(1)

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
