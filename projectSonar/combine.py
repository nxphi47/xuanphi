#!/bin/env python2

import sys, json, os
import matplotlib.pyplot as plt
import ConfigParser, argparse
from argparse import RawTextHelpFormatter
import numpy as np

DEBUG = 0


def get_value_by_key(dic, key):
    if dic.has_key(key):
        return dic[key]
    else:
        return None


'''
Class to extract features for each reflected signal
'''


class singleReflect(object):
    def __init__(self, envelop, time, _min, _max, index, data):
        if DEBUG == 1:
            print '\n\n*********** %d ***********' % index
        self.envelop = envelop  # array for envelop
        self.time = time  # array for timestamp
        self._min = _min  # sensor output signal index
        self._max = _max  # index when reflected signal detected
        self.index = index  # current cycle
        self.data = data  # feature dictionary
        self.averaging = 2  # averaging windows to calculate saturation
        self.saturate = 53  # saturation value to detect when reflected signal end (now replace with mod value in self.envelop)

    # split envelop and timestamp array to unmodified arrays and array with value before sensor output removed
    def split_array(self):
        _xmin = self.time[self._min]  # obtain time when sensor output signal
        _in = str(self.index)
        self.oriTimeList = 'otl' + _in  # original time array
        self.oriEnvList = 'oel' + _in  # original envelop array
        self.TimeEndList = 'tel' + _in  # time array contain only sound signal (without initialize portion and timing change by offset)
        self.EnvEndList = 'eel' + _in  # envelop array contain only sound signal (without initialize portion and timing change by offset)
        self.initIndex = 'iId' + _in  # index for signal output
        self.refIndex = 'rId' + _in  # index for reflect signal

        # relfect signal characteristic
        self.pulseWidth = 'pw' + _in  # pulse width
        self.timeWidth = 'td' + _in  # time width
        self.areaRatio = 'ar' + _in  # area ratio areaA / areaB
        self.peakValue = 'pv' + _in  # peak value for reflect signal
        self.minValue = 'mv' + _in  # minimum value for reflect signal
        self.totalArea = 'ta' + _in  # total area for reflect signal
        self.peakTime = 'pt' + _in  # time require to reach peak value
        self.refEnvList = 'rel' + _in  # reflect signal envelop array (until saturation)
        self.refTimeList = 'rtl' + _in  # reflect signal time array (until saturation)

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

        self.data[self.oriTimeList] = _oriTime
        self.data[self.oriEnvList] = _oriEnv
        self.data[self.TimeEndList] = _timeEnd
        self.data[self.EnvEndList] = _envEnd
        self.data[self.initIndex] = self._min
        self.data[self.refIndex] = self._max

    # main function
    def process_data(self):
        self.get_saturate_value()
        self.data[self.pulseWidth], self.data[self.timeWidth] = self.get_pulse_width()
        print '\n*************** %d ***************' % self.index
        print 'Pulse Width: %d, Time Width: %.3f, Saturate: %d' % (
        self.data[self.pulseWidth], self.data[self.timeWidth], self.saturate)

        # get reflected signal array and obtain their characteristic
        if self.data[self.pulseWidth] > 1:
            self.get_ref_env()
            if len(self.data[self.refEnvList]) > 1:
                self.get_peak()
                self.get_area()
            else:
                print 'No Reflected Signal wave'
        return self.data

    # obtain saturation value for single signal
    #	return mod value
    def get_saturate_value(self):
        _np = np.array(self.envelop)
        from scipy import stats
        self.saturate = int(stats.mode(_np)[0])

    # get pulse & time width for reflected signal
    #	return pulse width and time width
    def get_pulse_width(self):
        for i, x in enumerate(self.envelop):
            if i > self._max:
                _ave = 0
                print "[%d] - %d (%.3f)" % (i, x, self.time[i])
                _ave = x
                for y in xrange(self.averaging):
                    if (i + (y + 1)) > (len(self.envelop) - 1):
                        break
                    else:
                        _ave += self.envelop[i + (y + 1)]

                _value = _ave / (self.averaging + 1)
                if _value <= self.saturate:
                    pulse_width = i - self._max
                    # print "pulse width: %d - %d - %d = %d" % (i, self.averaging, self._max, pulse_width)
                    _time_diff = self.time[self._max + pulse_width - 1] - self.time[self._max]
                    return pulse_width, _time_diff  # , self.envelop[i - self.averaging]

    # get reflected signal timestamp and envelop array
    def get_ref_env(self):
        ref_env_list = []
        ref_time_list = []
        for i, x in enumerate(self.envelop[self._max:self._max + self.data[self.pulseWidth]]):
            ref_time_list.append(self.time[self._max + i])
            ref_env_list.append(x)

        self.data[self.refEnvList] = ref_env_list
        self.data[self.refTimeList] = ref_time_list

        for i, x in enumerate(self.data[self.refEnvList]):
            print self.data[self.refTimeList][i], x

    # get reflected signal peak amplitude
    def get_peak(self, rev=None):
        _np = np.array(self.data[self.refEnvList])
        self.data[self.peakValue] = np.amax(_np)
        self.data[self.minValue] = np.amin(_np) - 1
        self.data[self.peakTime] = self.get_peak_time()

        print 'Peak: %d, Min: %d, PeakTime: %.3f' % (
        self.data[self.peakValue], self.data[self.minValue], self.data[self.peakTime])

    # get reflected signal peak amplitude time offset
    def get_peak_time(self):
        _index = 0
        for i, x in enumerate(self.data[self.refEnvList]):
            if x == self.data[self.peakValue]:
                _index = i
        return self.data[self.refTimeList][_index] - self.data[self.refTimeList][0]

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
            print 'Multi Peak, normal'
            _act_count = 0
            for x in xrange(len(group)):
                if x == group_id[len(group_id) - 1]:
                    break
                _act_count += len(group[x])

            for i, x in enumerate(self.data[self.refEnvList]):
                if i == 0:
                    continue

                _timeDiff = self.data[self.refTimeList][i] - self.data[self.refTimeList][i - 1]
                _envelop1 = self.data[self.refEnvList][i] - self.data[self.minValue]
                _envelop2 = self.data[self.refEnvList][i - 1] - self.data[self.minValue]

                # print _timeDiff, _envelop1, _envelop2
                if x == self.data[self.peakValue] and x == _act_count:
                    _area += self.cal_area(_timeDiff, _envelop1, _envelop2)
                    self.areaA = _area
                    if DEBUG == 1:
                        print 'areaA: ', self.areaA
                    continue
                _area += self.cal_area(_timeDiff, _envelop1, _envelop2)
                if DEBUG == 1:
                    print _area
            self.data[self.totalArea] = _area
            self.areaB = self.data[self.totalArea] - self.areaA
        else:
            print 'Multi Peak, range'
            self.range_cal(group, group_id, group_id[len(group_id) - 1])

    def single_peak_cal(self, group, group_id):
        _area = 0.
        self.areaA = 0.
        _len = len(group[group_id[0]])
        if _len == 1:
            print 'Single Peak, normal'
            for i, x in enumerate(self.data[self.refEnvList]):
                if i == 0:
                    continue

                _timeDiff = self.data[self.refTimeList][i] - self.data[self.refTimeList][i - 1]
                _envelop1 = self.data[self.refEnvList][i] - self.data[self.minValue]
                _envelop2 = self.data[self.refEnvList][i - 1] - self.data[self.minValue]

                if DEBUG == 1:
                    print _timeDiff, _envelop1, _envelop2
                if x == self.data[self.peakValue]:
                    _area += self.cal_area(_timeDiff, _envelop1, _envelop2)
                    self.areaA = _area
                    if DEBUG == 1:
                        print 'areaA: ', self.areaA
                    continue
                _area += self.cal_area(_timeDiff, _envelop1, _envelop2)
                if DEBUG == 1:
                    print _area
            self.data[self.totalArea] = _area
            self.areaB = self.data[self.totalArea] - self.areaA
        else:
            print 'Single Peak, range'
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
                _env = self.data[self.peakValue] - self.data[self.minValue]
                _time = 0.
                if (len(group[x]) % 2) == 0:
                    # if even number
                    _first_index = (len(group[x]) / 2) + _act_count - 1
                    _second_index = _first_index + 1
                    _time = (
                            self.data[self.refTimeList][_second_index] + self.data[self.refTimeList][_first_index]) / 2.
                    if DEBUG == 1:
                        print 'even time: ', _time
                else:
                    # if odd number
                    _middle_index = int(len(group[x]) / 2) + _act_count
                    _time = self.data[self.refTimeList][_middle_index]
                    if DEBUG == 1:
                        print 'odd time: ', _time
                _range_area1 = self.cal_area((_time - self.data[self.refTimeList][_act_count]), _env, _env)
                _range_area2 = self.cal_area((self.data[self.refTimeList][_act_count - 1 + len(group[x])] - _time),
                                             _env, _env)
                _max_index_B = _max_index_A + len(group[x])
                break
            _max_index_A += len(group[x])
            _act_count += len(group[x])
        if DEBUG == 1:
            print 'max A, max B: ', _max_index_A, _max_index_B
            print 'Range Area: ', _range_area1, _range_area2

        for i, x in enumerate(self.data[self.refEnvList][:_max_index_A + 1]):
            if i == 0:
                continue
            _timeDiff = self.data[self.refTimeList][i] - self.data[self.refTimeList][i - 1]
            _envelop1 = self.data[self.refEnvList][i] - self.data[self.minValue]
            _envelop2 = self.data[self.refEnvList][i - 1] - self.data[self.minValue]
            _areaA += self.cal_area(_timeDiff, _envelop1, _envelop2)

        for i, x in enumerate(self.data[self.refEnvList]):
            if i >= _max_index_B:
                _timeDiff = self.data[self.refTimeList][i] - self.data[self.refTimeList][i - 1]
                _envelop1 = self.data[self.refEnvList][i] - self.data[self.minValue]
                _envelop2 = self.data[self.refEnvList][i - 1] - self.data[self.minValue]
                _areaB += self.cal_area(_timeDiff, _envelop1, _envelop2)
        if DEBUG == 1:
            print 'Area: ', _areaA, _areaB
        self.areaA = _areaA + _range_area1
        self.areaB = _areaB + _range_area2
        self.data[self.totalArea] = self.areaA + self.areaB

    # get area of reflect signal
    def get_area(self):
        # firstly we need to know the reflect signal charateristic before area calculation
        #	it require to find out index of peak value, and how long/many times it appear
        from itertools import groupby

        _group = []  # list where it will group same value in an array together
        _max_group_id = []  # index for _group which contain peak value, if len > 1 means multi peak value appear
        if DEBUG == 1:
            self.data[self.peakValue] = 63
            self.data[self.refTimeList] = [21.12, 21.168, 21.216, 21.264, 21.312, 21.36, 21.408, 21.456, 21.504, 21.552,
                                           21.6, 21.648, 21.696, 21.744, 21.792, 21.84, 21.888, 21.936, 21.984]
            self.data[self.refEnvList] = [58, 58, 61, 63, 63, 62, 62, 61, 57, 57, 63, 63, 63, 57, 57, 57, 57, 57, 55]
            print len(self.data[self.refTimeList]), len(self.data[self.refEnvList])

        # expected _group output will be:
        # _group = [[58,58], [61], [63,63], [62,62], [61], [57,57], [63,63,63], [57,57,57,57,57], [55]]

        for i, j in groupby(self.data[self.refEnvList]):
            _group.append(list(j))

        for x in xrange(len(_group)):
            if _group[x][0] == self.data[self.peakValue]:
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
        print "Total Area: %.3f, AreaA: %.3f" % (self.data[self.totalArea], self.areaA)
        if self.areaB == 0:
            self.data[self.areaRatio] = self.areaA / self.data[self.totalArea]
        else:
            self.data[self.areaRatio] = self.areaA / self.areaB
        print "AreaB: %.3f, Ratio: %.3f" % (self.areaB, self.data[self.areaRatio])


'''
	class use to plot combine graph
'''


class COMBINE(object):
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

            sr = singleReflect(self.envelop, self.time, self._min, self._max, index, self.data)
            sr.split_array()

            if not self._max == 0:
                self.data = sr.process_data()
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

        for x in self.data['index']:
            _in = str(x)

            self.refIndex = 'rId' + _in
            self.TimeEndList = 'tel' + _in  # time array contain only sound signal (withou initialize portion at beginning)
            self.EnvEndList = 'eel' + _in  # envelop array contain only sound signal (without initialize portion at beginning)
            self.initIndex = 'iId' + _in  # index for signal output
            self.refIndex = 'rId' + _in  # index for reflect signal

            # relfect signal characteristic
            self.pulseWidth = 'pw' + _in  # pulse width
            self.refEnvList = 'rel' + _in  # reflect signal envelop array (until saturation)

            if self.data.has_key(self.refIndex):
                if self.data[self.refIndex] == 0:
                    print 'No reflect signal detected for index: ', _in
                    continue

                if self.data.has_key(self.TimeEndList) and self.data.has_key(self.EnvEndList):
                    _legend.append(str(x))
                    plt.plot(self.data[self.TimeEndList], self.data[self.EnvEndList])

                    # update time range for all cycle to find out minimum and maximum range
                    _start_index = self.data[self.refIndex] - self.data[self.initIndex]
                    _end_index = _start_index + self.data[self.pulseWidth]

                    if _start_index < _start_time_ind:
                        _start_time_ind = _start_index
                    if _end_index > _end_time_ind:
                        _end_time_ind = _end_index

                    # update envelop range for all cycle to find out minimum and maximum range
                    _min = np.amin(np.array(self.data[self.refEnvList]))
                    _max = np.amax(np.array(self.data[self.refEnvList]))

                    if _min < _min_env:
                        _min_env = _min
                    if _max > _max_env:
                        _max_env = _max
                else:
                    print("No Key for %s or %s" % (self.TimeEndList, self.EnvEndList))

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
        if _xmaxInd > (len(self.TimeEndList) - 1):
            _xmaxInd = len(self.TimeEndList) - 1
        ax.set_xlim(self.data[self.TimeEndList][_start_time_ind - 10], self.data[self.TimeEndList][_xmaxInd])
        ax.set_ylim(_min_env - 20, _max_env + 20)
        plt.savefig(os.path.join(self.root, self.allplotTitle.replace('.', ',') + '_rezoom'))

        plt.close('all')
        self.output_json()
