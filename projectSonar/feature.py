#!/bin/env python2

# -------------------------
# feature.py
#	Adapted from Kian Soon's combine.py
# -------------------------

import sys, json, os
import matplotlib.pyplot as plt
import ConfigParser, argparse
from argparse import RawTextHelpFormatter
import numpy as np

DEBUG = 0


def get_value_by_key(dic, key, default=None):
    if dic.has_key(key):
        return dic[key]
    else:
        return default


'''
Class to extract features for each reflected signal
'''


class singleReflect(object):
    def __init__(self, envelop, time, _min, _max, config={}):
        self.envelop = envelop  # array for envelop
        self.time = time  # array for timestamp
        self._min = _min  # sensor output signal index
        self._max = _max  # index when reflected signal detected
        # self.index = index			# current cycle
        self.data = {}  # feature dictionary
        self.averaging = 2  # averaging windows to calculate saturation (CWNG: why is this hardcoded?)
        self.saturate = 53  # saturation value to detect when reflected signal end (now replace with mod value in self.envelop)
        self.zeroGrad = get_value_by_key(config, 'zero-gradient', 2)
        # maximum differnce in value to be considered zero gradient
        self.pulseEnd = get_value_by_key(config, 'pulse-end-detect-method', 1)

    # what pulse end detect method
    #	0 = Kian Soon's averaging method
    #	1 = pulse is considered ended once the envelop value hit the saturate value
    #	2 = pulse is considered ended based on gradient and signal value

    # split envelop and timestamp array to unmodified arrays and array with value before sensor output removed
    def split_array(self):
        _xmin = self.time[self._min]  # obtain time when sensor output signal
        # self.oriTimeList = 'otl'	# original time array
        # self.oriEnvList = 'oel'		# original envelop array
        # self.TimeEndList = 'tel'	# time array contain only sound signal (without initialize portion and timing change by offset)
        # self.EnvEndList = 'eel'		# envelop array contain only sound signal (without initialize portion and timing change by offset)
        # self.initIndex = 'iId'		# index for signal output
        # self.refIndex = 'rId'		# index for reflect signal

        # relfect signal characteristic
        # self.pulseWidth = 'pw'	 	# pulse width
        # self.timeWidth = 'td'			# time width
        # self.areaRatio = 'ar'			# area ratio areaA / areaB
        # self.peakValue = 'pv'			# peak value for reflect signal
        # self.minValue = 'mv'			# minimum value for reflect signal
        # self.totalArea = 'ta'			# total area for reflect signal
        # self.peakTime = 'pt'			# time require to reach peak value
        # self.refEnvList = 'rel' 		# reflect signal envelop array (until saturation)
        # self.refTimeList = 'rtl'		# reflect signal time array (until saturation)

        _timeEnd = []  # time array with value before sensor output removed (and reset to 0)
        _envEnd = []  # envelop array with value before sensor output removed
        _oriTime = []  # unmodified timestamp array
        _oriEnv = []  # unmodified envelop array

        # feed into dictionary
        for _index, enve in enumerate(self.envelop):
            _oriTime.append(self.time[_index])
            _oriEnv.append(self.envelop[_index])
            if _index >= self._min:
                _timeEnd.append(self.time[_index] - _xmin)
                _envEnd.append(self.envelop[_index])

        self.data['oriTimeList'] = _oriTime
        self.data['oriEnvList'] = _oriEnv
        self.data['TimeEndList'] = _timeEnd
        self.data['EnvEndList'] = _envEnd
        self.data['initIndex'] = self._min
        self.data['refIndex'] = self._max

    # main function
    def process_data(self):
        self.data['envGradient'] = self._get_gradient(self.envelop)
        self.data['floorValue'] = self.get_saturate_value()
        self.data['txPulseStart'] = self._min
        self.data['refPulseStart'] = self._max
        self.get_all_pulse_width(-1)
        self.get_all_pulse_width(self._min)
        self.data['pulseWidth'], self.data['timeWidth'] = self.get_pulse_width()
        self.data['txPulseWidth'], self.data['txTimeWidth'] = self.get_pulse_width(self._min)
        self.data['normTimeWidth'] = self.data['timeWidth'] / self.data['txTimeWidth']

        # print '\n*************** %d ***************' % self.index
        # print 'Pulse Width: %d, Time Width: %.3f, Saturate: %d'% (self.data['pulseWidth'], self.data['timeWidth'], self.saturate)

        # get reflected signal array and obtain their characteristic
        if self.data['pulseWidth'] > 1:
            self.get_ref_env()
            if len(self.data['refEnvList']) > 1:
                self.get_peak()
                self.get_area()
                self.calc_gradient()
            else:
                print 'No Reflected Signal wave'
                return None

        # process additional feature
        self.data['peakWidthDiff'] = self.data['timeWidth'] - self.data['peakTime']
        self.data['peakWidthDiv'] = self.data['peakTime'] / self.data['timeWidth']
        self.data['peakValueDiv'] = self.data['peakValue'] / self.data['peakTime']

        return self.data

    # extract the feature vector
    def get_feature_vector(self, features=['pulseWidth', 'peakValue', 'totalArea', 'peakTime']):
        ret = []
        for f in features:
            if self.data.has_key(f):
                ret.append(self.data[f])
            else:
                ret.append(0.0)
        return ret

    # obtain saturation value for single signal
    #	return mod value
    def get_saturate_value(self):
        # first method
        _np = np.array(self.envelop)
        from scipy import stats
        self.saturate = int(stats.mode(_np)[0])
        # second method
        _np = np.array(self.envelop[:50])
        self.data['altFloor'] = int(stats.mode(_np)[0])
        return self.saturate

    # get pulse & time width for reflected signal
    #	return pulse width and time width
    def get_pulse_width(self, start=-1):
        if start == -1:
            start = self._max
        if self.pulseEnd == 1:
            return self.__get_pulse_width_using_immed(start)
        if self.pulseEnd == 2:
            return self.__get_pulse_width_using_grad(start)
        return self.__get_pulse_width_using_ave(start)

    # get all pulse width for reflected signal
    def get_all_pulse_width(self, start=-1):
        if start == -1:
            start = self._max
            label = 'ref'
        else:
            label = 'tx'
        self.data[label + 'PulseWidth-immed'], self.data[
            label + 'TimeWidth-immed'] = self.__get_pulse_width_using_immed(start)
        self.data[label + 'PulseWidth-grad'], self.data[label + 'TimeWidth-grad'] = self.__get_pulse_width_using_grad(
            start)
        self.data[label + 'PulseWidth-ave'], self.data[label + 'TimeWidth-ave'] = self.__get_pulse_width_using_ave(
            start)

    # original Kian Soon method using averge of a window
    def __get_pulse_width_using_ave(self, start):
        for i, x in enumerate(self.envelop):
            if i > start:
                _ave = 0
                # print "[%d] - %d (%.3f)" % (i, x, self.time[i])
                _ave = x
                for y in xrange(self.averaging):
                    if (i + (y + 1)) > (len(self.envelop) - 1):
                        break
                    else:
                        _ave += self.envelop[i + (y + 1)]

                _value = _ave / (self.averaging + 1)
                if _value <= self.saturate:
                    pulse_width = i - start
                    # print "pulse width: %d - %d - %d = %d" % (i, self.averaging, self._max, pulse_width)
                    _time_diff = self.time[start + pulse_width - 1] - self.time[start]
                    return pulse_width, _time_diff  # , self.envelop[i - self.averaging]
        return 0, 0

    # new method using gradient of signal
    def __get_pulse_width_using_grad(self, start):
        for i in xrange(start + self.averaging, len(self.envelop)):
            _zero = True
            for g in self.data['envGradient'][i - self.averaging:i + 1]:
                if g != 0:
                    _zero = False
                    break
            if _zero and (self.envelop[i] < self.saturate + self.zeroGrad):
                # previous 3 gradients are all 0 and signal drop below saturate + margin
                pulse_width = i - start
                _time_diff = self.time[start + pulse_width - 1] - self.time[start]
                return pulse_width, _time_diff
        return 0, 0

    # new method using signal value once it touches the saturate value
    def __get_pulse_width_using_immed(self, start):
        for i, x in enumerate(self.envelop):
            if i > start + self.averaging:
                if x <= self.saturate:
                    pulse_width = i - start
                    _time_diff = self.time[start + pulse_width - 1] - self.time[start]
                    return pulse_width, _time_diff
        return 0, 0

    # get reflected signal timestamp and envelop array
    def get_ref_env(self):
        ref_env_list = []
        ref_time_list = []
        for i, x in enumerate(self.envelop[self._max:self._max + self.data['pulseWidth']]):
            ref_time_list.append(self.time[self._max + i])
            ref_env_list.append(x)

        self.data['refEnvList'] = ref_env_list
        self.data['refTimeList'] = ref_time_list

    # for i, x in enumerate(self.data['refEnvList']):
    #	print self.data['refTimeList'][i], x

    # get reflected signal peak amplitude
    def get_peak(self, rev=None):
        _np = np.array(self.data['refEnvList'])
        self.data['peakValue'] = np.amax(_np)
        self.data['minValue'] = np.amin(_np) - 1
        self.data['peakTime'] = self.get_peak_time()

    # print 'Peak: %d, Min: %d, PeakTime: %.3f' % (self.data['peakValue'], self.data['minValue'], self.data['peakTime'])

    # get reflected signal peak amplitude time offset
    def get_peak_time(self):
        _index = 0
        for i, x in enumerate(self.data['refEnvList']):
            if x == self.data['peakValue']:
                _index = i
        return self.data['refTimeList'][_index] - self.data['refTimeList'][0]

    # calculate area
    def cal_area(self, dtime, e1, e2):
        return float(e1 + e2) / 2. * dtime

    '''
        Reflected signal contain four characteristic
        1. peak value appear only one sample [Single peak, normal]
        2. peak value appear more than one samples continuously [Single peak, range]
        3. peak value appear more than one samples separately [Multi peak, normal]
        4. peak value appear more than one samples separately, each time appear will involve more than a sample
            [Multi peak, range]

    '''

    # multi peak calculation
    def multi_peak_cal(self, group, group_id):
        _area = 0.
        self.areaA = 0.
        _last_id_value = group_id[len(group_id) - 1]
        if len(group[_last_id_value]) == 1:
            # print 'Multi Peak, normal'
            _act_count = 0
            for x in xrange(len(group)):
                if x == group_id[len(group_id) - 1]:
                    break
                _act_count += len(group[x])

            for i, x in enumerate(self.data['refEnvList']):
                if i == 0:
                    continue

                _timeDiff = self.data['refTimeList'][i] - self.data['refTimeList'][i - 1]
                _envelop1 = self.data['refEnvList'][i] - self.data['minValue']
                _envelop2 = self.data['refEnvList'][i - 1] - self.data['minValue']

                # print _timeDiff, _envelop1, _envelop2
                if x == self.data['peakValue'] and x == _act_count:
                    _area += self.cal_area(_timeDiff, _envelop1, _envelop2)
                    self.areaA = _area
                    if DEBUG == 1:
                        print 'areaA: ', self.areaA
                    continue
                _area += self.cal_area(_timeDiff, _envelop1, _envelop2)
                if DEBUG == 1:
                    print _area
            self.data['totalArea'] = _area
            self.areaB = self.data['totalArea'] - self.areaA
        else:
            # print 'Multi Peak, range'
            self.range_cal(group, group_id, group_id[len(group_id) - 1])

    def single_peak_cal(self, group, group_id):
        _area = 0.
        self.areaA = 0.
        _len = len(group[group_id[0]])
        if _len == 1:
            # print 'Single Peak, normal'
            for i, x in enumerate(self.data['refEnvList']):
                if i == 0:
                    continue

                _timeDiff = self.data['refTimeList'][i] - self.data['refTimeList'][i - 1]
                _envelop1 = self.data['refEnvList'][i] - self.data['minValue']
                _envelop2 = self.data['refEnvList'][i - 1] - self.data['minValue']

                if DEBUG == 1:
                    print _timeDiff, _envelop1, _envelop2
                if x == self.data['peakValue']:
                    _area += self.cal_area(_timeDiff, _envelop1, _envelop2)
                    self.areaA = _area
                    if DEBUG == 1:
                        print 'areaA: ', self.areaA
                    continue
                _area += self.cal_area(_timeDiff, _envelop1, _envelop2)
                if DEBUG == 1:
                    print _area
            self.data['totalArea'] = _area
            self.areaB = self.data['totalArea'] - self.areaA
        else:
            # print 'Single Peak, range'
            self.range_cal(group, group_id, group_id[0])

    # function to mainly calculate range area ( only use for single peak range and multi peak range)
    def range_cal(self, group, group_id, range_id):
        _range_area1 = 0.
        _range_area2 = 0.
        _areaA = 0.
        _areaB = 0.
        _max_index_A = 0
        _max_index_B = 0

        _act_count = 0
        for x in xrange(len(group)):
            if x == range_id:
                # if peak value row
                _env = self.data['peakValue'] - self.data['minValue']
                _time = 0.
                if (len(group[x]) % 2) == 0:
                    # if even number
                    _first_index = (len(group[x]) / 2) + _act_count - 1
                    _second_index = _first_index + 1
                    _time = (self.data['refTimeList'][_second_index] + self.data['refTimeList'][_first_index]) / 2.
                    if DEBUG == 1:
                        print 'even time: ', _time
                else:
                    # if odd number
                    _middle_index = int(len(group[x]) / 2) + _act_count
                    _time = self.data['refTimeList'][_middle_index]
                    if DEBUG == 1:
                        print 'odd time: ', _time
                _range_area1 = self.cal_area((_time - self.data['refTimeList'][_act_count]), _env, _env)
                _range_area2 = self.cal_area((self.data['refTimeList'][_act_count - 1 + len(group[x])] - _time), _env,
                                             _env)
                _max_index_B = _max_index_A + len(group[x])
                break
            _max_index_A += len(group[x])
            _act_count += len(group[x])
        if DEBUG == 1:
            print 'max A, max B: ', _max_index_A, _max_index_B
            print 'Range Area: ', _range_area1, _range_area2

        for i, x in enumerate(self.data['refEnvList'][:_max_index_A + 1]):
            if i == 0:
                continue
            _timeDiff = self.data['refTimeList'][i] - self.data['refTimeList'][i - 1]
            _envelop1 = self.data['refEnvList'][i] - self.data['minValue']
            _envelop2 = self.data['refEnvList'][i - 1] - self.data['minValue']
            _areaA += self.cal_area(_timeDiff, _envelop1, _envelop2)

        for i, x in enumerate(self.data['refEnvList']):
            if i >= _max_index_B:
                _timeDiff = self.data['refTimeList'][i] - self.data['refTimeList'][i - 1]
                _envelop1 = self.data['refEnvList'][i] - self.data['minValue']
                _envelop2 = self.data['refEnvList'][i - 1] - self.data['minValue']
                _areaB += self.cal_area(_timeDiff, _envelop1, _envelop2)
        if DEBUG == 1:
            print 'Area: ', _areaA, _areaB
        self.areaA = _areaA + _range_area1
        self.areaB = _areaB + _range_area2
        self.data['totalArea'] = self.areaA + self.areaB

    # get area of reflect signal
    def get_area(self):
        # firstly we need to know the reflect signal charateristic before area calculation
        #	it require to find out index of peak value, and how long/many times it appear
        from itertools import groupby

        _group = []  # list where it will group same value in an array together
        _max_group_id = []  # index for _group which contain peak value, if len > 1 means multi peak value appear
        if DEBUG == 1:
            self.data['peakValue'] = 63
            self.data['refTimeList'] = [21.12, 21.168, 21.216, 21.264, 21.312, 21.36, 21.408, 21.456, 21.504, 21.552,
                                        21.6, 21.648, 21.696, 21.744, 21.792, 21.84, 21.888, 21.936, 21.984]
            self.data['refEnvList'] = [58, 58, 61, 63, 63, 62, 62, 61, 57, 57, 63, 63, 63, 57, 57, 57, 57, 57, 55]
            print len(self.data['refTimeList']), len(self.data['refEnvList'])

        # expected _group output will be:
        # _group = [[58,58], [61], [63,63], [62,62], [61], [57,57], [63,63,63], [57,57,57,57,57], [55]]

        for i, j in groupby(self.data['refEnvList']):
            _group.append(list(j))

        for x in xrange(len(_group)):
            if _group[x][0] == self.data['peakValue']:
                _max_group_id.append(x)

        if DEBUG == 1:
            print 'group: ', _group
            print 'max group index: ', _max_group_id

        # there have multiple peak in separate group
        _len = len(_max_group_id)
        if _len > 1:
            self.multi_peak_cal(_group, _max_group_id)
        else:
            # single peak situation
            self.single_peak_cal(_group, _max_group_id)
        self.get_area_ratio()
        if DEBUG == 1:
            exit(1)

    # calculate area ratio
    def get_area_ratio(self):
        # print "Total Area: %.3f, AreaA: %.3f" % (self.data['totalArea'], self.areaA)
        if self.areaB == 0:
            self.data['areaRatio'] = self.areaA / self.data['totalArea']
        else:
            self.data['areaRatio'] = self.areaA / self.areaB
        # print "AreaB: %.3f, Ratio: %.3f" % (self.areaB, self.data['areaRatio'])

    # gradient-related features
    def calc_gradient(self):
        if not self.data.has_key('refEnvList'):
            return
        _sig = self.data['refEnvList']
        _grad = []
        _prev = 1
        _numc = 0
        _nump = 0
        _numn = 0
        _num0 = 0
        for i in xrange(1, len(_sig)):
            if _sig[i] > _sig[i - 1] + self.zeroGrad:
                if _prev == -1:
                    _numc += 1
                _grad.append(1)
                _nump += 1
                _prev = 1
            elif _sig[i] < _sig[i - 1] - self.zeroGrad:
                if _prev == 1:
                    _numc += 1
                _grad.append(-1)
                _numn += 1
                _prev = -1
            else:
                _grad.append(0)
                _num0 += 1
        self.data['gradientList'] = _grad
        self.data['+gradRatio'] = 1.0 * _nump / len(_grad)
        self.data['-gradRatio'] = 1.0 * _numn / len(_grad)
        self.data['0gradRatio'] = 1.0 * _num0 / len(_grad)
        self.data['gradChange'] = _numc

    # gradient-related features
    def _get_gradient(self, sig):
        _grad = [0]
        for i in xrange(1, len(sig)):
            if sig[i] > sig[i - 1] + self.zeroGrad:
                _grad.append(1)
            elif sig[i] < sig[i - 1] - self.zeroGrad:
                _grad.append(-1)
            else:
                _grad.append(0)
        return _grad


'''
	class use to plot combine graph
'''


class COMBINE(object):
    def __init__(self, combinePlot, root, comName):
        self.combinePlot = combinePlot
        self.data = []
        self.root = root
        self.comName = comName

    def insert(self, _min, _max, _time, _envelop, title):
        self._min = _min
        self._max = _max
        self.time = _time
        self.envelop = _envelop
        self.allplotTitle = title

    def update(self):
        if not self._min == 0:
            sr = singleReflect(self.envelop, self.time, self._min, self._max)
            sr.split_array()
            if not self._max == 0:
                self.data.append(sr.process_data())
            else:
                print "Return as no reflected signal detected"
        return self.data

    def output_json(self):
        # output data to self.comName
        if self.combinePlot:
            with open(os.path.join(self.root, self.comName), "w") as fp:
                json.dump(self.data, fp)

    def plot(self):
        fig, ax = plt.subplots()
        _legend = []

        _max_env = 0
        _min_env = 1000
        _start_time_ind = 1000
        _end_time_ind = 0

        # for x in self.data['index']:
        for d in self.data:
            # self.refIndex = 'rId'
            # self.TimeEndList = 'tel'	# time array contain only sound signal (withou initialize portion at beginning)
            # self.EnvEndList = 'eel'		# envelop array contain only sound signal (without initialize portion at beginning)
            # self.initIndex = 'iId'		# index for signal output
            # self.refIndex = 'rId'		# index for reflect signal

            # relfect signal characteristic
            # self.pulseWidth = 'pw'		# pulse width
            # self.refEnvList = 'rel'		# reflect signal envelop array (until saturation)

            if d.has_key('refIndex'):
                if d['refIndex'] == 0:
                    print 'No reflect signal detected for index: ', self.data.index(d)
                    continue

                if d.has_key('TimeEndList') and d.has_key('EnvEndList'):
                    _legend.append(str(self.data.index(d)))
                    plt.plot(d['TimeEndList'], d['EnvEndList'])

                    # update time range for all cycle to find out minimum and maximum range
                    _start_index = d['refIndex'] - d['initIndex']
                    _end_index = _start_index + d['pulseWidth']

                    if _start_index < _start_time_ind:
                        _start_time_ind = _start_index
                    if _end_index > _end_time_ind:
                        _end_time_ind = _end_index

                    # update envelop range for all cycle to find out minimum and maximum range
                    _min = np.amin(np.array(d['refEnvList']))
                    _max = np.amax(np.array(d['refEnvList']))

                    if _min < _min_env:
                        _min_env = _min
                    if _max > _max_env:
                        _max_env = _max
                else:
                    print("No Key for %s or %s" % ('TimeEndList', 'EnvEndList'))

        ax.set_ylim(0, 200)
        ax.set_xlabel('Time (ms)')
        ax.set_ylabel('Envelop', color='b')

        # plot all signal plot into single graph
        plt.title(self.allplotTitle)
        # plt.legend(_legend, loc='upper right')
        plt.savefig(os.path.join(self.root, self.allplotTitle.replace('.', ',')))

        # zoom in reflection signal
        plt.title(self.allplotTitle + '_rezoom')
        _xmaxInd = _end_time_ind + 25
        # ???????  Shouldn't below be self.data[self.TimeEndList] ?????
        # if _xmaxInd > (len(self.TimeEndList) - 1):
        #	_xmaxInd = len(self.TimeEndList) - 1
        if _xmaxInd > (len(self.data[-1]['TimeEndList']) - 1):
            _xmaxInd = len(self.data[-1]['TimeEndList']) - 1
        ax.set_xlim(self.data[-1]['TimeEndList'][_start_time_ind - 10], self.data[-1]['TimeEndList'][_xmaxInd])
        ax.set_ylim(_min_env - 20, _max_env + 20)
        plt.savefig(os.path.join(self.root, self.allplotTitle.replace('.', ',') + '_rezoom'))

        plt.close('all')
        self.output_json()
