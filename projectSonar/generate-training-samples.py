#!/bin/env python2

# ------------------------------
# generate-training-samples.py
# Generate the set of training samples (i.e. feature vector + label)
# ------------------------------

import json
import sys
import os
import utils
import collections
from copy import deepcopy

# the feature vector to extract
# FEATURES=['pulseWidth', 'peakValue', 'totalArea', 'peakTime']
# FEATURES=['normTimeWidth', 'timeWidth', 'peakValue', 'totalArea', 'peakTime', '+gradRatio', '-gradRatio', '0gradRatio', 'peakValueDiv']
# FEATURES=['normTimeWidth', 'timeWidth', 'peakValue', 'totalArea', 'peakTime', 'peakWidthDiff', 'peakWidthDiv', 'peakValueDiv', 'areaRatio' ]
# FEATURES=['refPulseWidth-immed', 'pulseWidth', 'timeWidth', 'peakWidthDiff', 'peakValueDiv', 'peakTime', 'areaRatio' ]
# FEATURES = ['normTimeWidth', 'peakValue', 'totalArea', 'peakTime', '+gradRatio', '-gradRatio', '0gradRatio',
#			'peakWidthDiff', 'peakWidthDiv', 'peakValueDiv', 'areaRatio', 'refTimeWidth-ave', 'refTimeWidth-immed',
#			'refTimeWidth-grad', 'gradChange']
FEATURES_A = ['peakValue', 'totalArea', 'peakTime', 'refTimeWidth-immed', 'areaRatio']
FEATURES_B = ['peakValue', 'totalArea', 'peakTime', 'refTimeWidth-grad', '0gradRatio']
FEATURES_C = ['peakValue', 'totalArea', 'peakTime', 'refTimeWidth-grad', '+gradRatio', 'peakWidthDiff']

FEATURES = []

# number sample to smooth, can change by 'sm' command
SMOOTH = 1

# define how to split the train/test, can change by 'st' command
#	smooth - split based on smoothing integer
#	side	- split by discard side of each json file and take only middle for test
#   split	- split train/test based on json file
DIVIDER = 'smooth'

# define whether trying to output a NUS training file
# 	if TRUE, it will not separate training and testing seq file
NUS = False


def usage():
	print "Usage: %s <infile> <outfile> [<num-test>] [fv=<features>] [sm=<smooth-int>]" % sys.argv[0]
	print "\tGenerate the training samples values from the 'processed' file containing all data\n"
	print "Parameters:"
	print "\t<infile>: the input file generated by process-json.py\n"
	print "\t<outfile>: the output file to be generated\n"
	print "\t<num-test>: this is optional.  If specified as a non-zero number (say T), "
	print "\t\tevery T-th sample for each class is set aside as test sample. A file "
	print "\t\tnamed <outfile>.test will be created to store the test samples."
	print "\t\tDefault: 0\n"
	print "\t<fv=features>: this is optional.  If specified, it should be a list of feature names"
	print "\t\taccording to names used by feature.py that should be extracted as feature vector."
	print "\t\tThe feature names are separated by commas (without quote or spaces),  and in the"
	print "\t\torder of how they will appear in the feature vector. "
	print "\t\tDefault: %s\n" % ','.join(x for x in FEATURES)
	print "\t<sm=smooth-int>: this is optional.  If specified, it should be a number of sample to smooth."
	print "\t\tThis method implement for early binding on SVM will it will combine <smooth-int> number of sample's"
	print "\t\tfv together for SVM classification. It will generate a new training sample file with extension .seq"
	print "\t\tWhen perform svm-train, svm will check .seq file and load fv accordingly"
	print "\t\tDefault: 1\n"
	print

	exit(0)


def process(inf, outf, features=FEATURES, extraMETA=None):
	while True:
		sample, obj = utils.get_sample_with_obj(inf, features, META=True)
		if not sample:
			break
		if sample.has_key('fv'):
			# check for all zero fv
			# if utils.is_all_zero_fv(sample['fv']):
			#	# ignore samples with all-zero fv
			#	print "sample(%s,%d) has all zero fv %s --> ignored" % (obj['filename'], obj['DataID'], repr(sample['fv']))
			#	continue
			outf.write(json.dumps(sample) + '\n')
		elif sample.has_key('META'):
			if extraMETA:
				meta = {'json-settings': sample['META']}
				meta.update(extraMETA)
				if not NUS:
					outf.write(json.dumps({'META': meta}) + '\n')
			else:
				if not NUS:
					outf.write(json.dumps(sample) + '\n')


def process_with_test(inf, outf, ntest, toutf, ignoreID, features=FEATURES, smooth=SMOOTH, extraMETA=None, seqf=None,
					  tseqf=None, divider=DIVIDER):
	labels = {}
	dataID = []
	datafv = []
	lists = []
	meta = {}
	FILENAME = ''
	while True:
		sample, obj = utils.get_sample_with_obj(inf, features, META=True)
		if not sample:
			break
		if sample.has_key('META'):
			if extraMETA:
				meta = {'json-settings': sample['META']}
				meta.update(extraMETA)
				if not NUS:
					outf.write(json.dumps({'META': meta}) + '\n')
					toutf.write(json.dumps({'META': meta}) + '\n')
			else:
				if not NUS:
					outf.write(json.dumps(sample) + '\n')
					toutf.write(json.dumps(sample) + '\n')
			continue
		elif not sample.has_key('fv'):
			continue

		if sample['id'] in ignoreID:
			print '%d found in ignoreID, bypass it...' % sample['id']
			continue

		# check for all zero fv
		# if utils.is_all_zero_fv(sample['fv']):
		#	# ignore samples with all-zero fv
		#	print "sample(%s,%d) has all zero fv %s --> ignored" % (obj['filename'], obj['DataID'], repr(sample['fv']))
		#	continue
		if not sample['label'] in labels:
			labels[sample['label']] = 0
		labels[sample['label']] += 1
		if labels[sample['label']] == ntest:
			labels[sample['label']] = 0
			if not NUS:
				toutf.write(json.dumps(sample) + '\n')
		else:
			if not NUS:
				outf.write(json.dumps(sample) + '\n')

		if smooth > 1:
			if len(FILENAME) < 1 or not sample['filename'] == FILENAME:
				FILENAME = sample['filename']
				dataID = []
				datafv = []
			dataID.append(sample['id'])
			datafv.append(sample['fv'])
			if len(dataID) == smooth:
				_dict = {}
				_dict['id'] = dataID[0]
				_dict['joint-id'] = dataID
				_dict['filename'] = FILENAME
				# _l = []
				# for i in datafv:
				#	_l.extend(i)
				# _dict['fv'] = _l
				_dict['fv'] = datafv
				_dict['label'] = sample['label']
				# _odict = collections.OrderedDict(sorted(_dict.items()))
				lists.append(deepcopy(_dict))
				dataID.pop(0)
				datafv.pop(0)

	if divider == 'split':
		if smooth == 1:
			_dict = {}
			_dict['id'] = sample['id']
			_dict['filename'] = sample['filename']
			_dict['fv'] = sample['fv']
			_dict['label'] = sample['label']
			lists.append(deepcopy(_dict))

		# get list of filename from list of dictionary
		_group = list(set([d['filename'] for d in lists if 'filename' in d]))

		_test_len = int(len(_group) / 6)
		# import random
		# _test_json_file = random.sample(_group, _test_len)
		# _test_json_file = _group[len(_group)-_test_len:]

		# defined number of classes
		_class = []
		for name in (_group):
			_sname = name.split('-')
			for _curb in _sname:
				if 'curb' in _curb:
					if _curb not in _class:
						_class.append(_curb)

		if _test_len < len(_class):
			_num = 1
		else:
			_num = int(_test_len / len(_class))

		_test_json_file = []
		_test_dict = {}
		for name in reversed(_group):
			_sname = name.split('-')
			for _curb in _sname:
				if 'curb' in _curb:
					if _test_dict.has_key(_curb):
						_buf = _test_dict[_curb]
						if len(_buf) < _num and '-rev' not in name:
							_buf.append(name)
							_test_dict[_curb] = _buf
					else:
						if '-rev' not in name:
							_buf = []
							_buf.append(name)
							_test_dict[_curb] = _buf

		for key, value in enumerate(_test_dict):
			_buf = _test_dict[value]
			_test_json_file.extend(_buf)

		OLD = False
		if OLD:
			_num = int(_test_len / len(_class))
			_remain = _test_len % _num
			_cub = _num + int(_remain / 2)

			l37, l20, l10, l7, l5, l15 = [], [], [], [], [], []
			for name in reversed(_group):
				print name
				if 'curb37.5' in name and len(l37) < _num:
					l37.append(name)
				if 'curb7.5' in name and len(l7) < _num:
					l7.append(name)
				if 'curb5' in name and len(l7) < _num:
					l5.append(name)
				if 'curb15' in name and len(l7) < _num:
					l15.append(name)
				if 'curb20' in name and '-rev' not in name and len(l20) < _cub:
					l20.append(name)
				if 'curb10' in name and '-rev' not in name and len(l10) < _cub:
					l10.append(name)
				if len(l37) == _num and len(l7) == _num and len(l20) == _cub and len(l10) == _cub:
					break
			_test_json_file = l37 + l20 + l10 + l7 + l5 + l15

		if extraMETA:
			meta2 = {'test_json_file': _test_json_file}
			meta2.update(meta)

		for l in lists:
			if NUS:
				seqf.write(json.dumps(l) + '\n')
			else:
				if l['filename'] in _test_json_file:
					tseqf.write(json.dumps(l) + '\n')
				else:
					seqf.write(json.dumps(l) + '\n')

		seqf.write(json.dumps({'META': meta2}) + '\n')
		if not NUS:
			tseqf.write(json.dumps({'META': meta2}) + '\n')
	else:
		if smooth > 1:
			print 'smooth > 1... ', smooth
			# set how to split
			if divider == 'smooth':
				_counter = 1
				for l in lists:
					if not NUS:
						if _counter < smooth:
							seqf.write(json.dumps(l) + '\n')
						else:
							tseqf.write(json.dumps(l) + '\n')
							_counter = 0
						_counter += 1
					else:
						seqf.write(json.dumps(l) + '\n')
			elif divider == 'side':
				# get list of filename from list of dictionary
				_group = [d['filename'] for d in lists if 'filename' in d]

				# get number of sample occur with same filename
				from collections import Counter
				c = Counter(_group)

				_counter = 0
				for l in lists:
					if len(FILENAME) < 1 or not l['filename'] == FILENAME:
						FILENAME = l['filename']
						_counter = 0
					_lnum = c[l['filename']]
					_num = int(_lnum / smooth)
					# make sure always most middle sample choose
					if _lnum & 1 and _lnum > 4:
						_num += 1
					if not NUS:
						if not _counter == _num:
							seqf.write(json.dumps(l) + '\n')
						else:
							tseqf.write(json.dumps(l) + '\n')
						_counter += 1
					else:
						seqf.write(json.dumps(l) + '\n')
			seqf.write(json.dumps({'META': meta}) + '\n')
			if not NUS:
				tseqf.write(json.dumps({'META': meta}) + '\n')
	print divider


if __name__ == '__main__':
	if len(sys.argv) < 3:
		usage()

	test = 0
	if len(sys.argv) > 3:
		test = int(sys.argv[3])
	features = FEATURES
	features_name = ''
	smooth = SMOOTH
	divider = DIVIDER
	if len(sys.argv) > 4:
		for x in sys.argv[4:]:
			_buf = x.split('=')
			if 'fv' in x:
				features = _buf[1].split(',')
			if 'sm' in x:
				smooth = int(_buf[1])
			if 'dv' in x:
				divider = _buf[1]
			if 'nus' in x:
				NUS = True
			if 'feature' in x:
				# default is feature C
				if _buf[1] == 'a' or _buf[1] == 'A':
					features = FEATURES_A
					features_name = 'A'
				elif _buf[1] == 'b' or _buf[1] == 'B':
					features = FEATURES_B
					features_name = 'B'
				else:
					features = FEATURES_C
					features_name = 'C'
			else:
				features = FEATURES_C
				features_name = 'C'

	print "Generate using FEATURES ", features_name

	inf = open(sys.argv[1], 'r')
	if not inf:
		print "Unable to open '%s' as input!" % sys.argv[1]
		exit(-1)
	if not NUS:
		outf = open(sys.argv[2], 'w')
		if not outf:
			print "Unable to open '%s' as output!" % sys.argv[2]
			exit(-1)
	else:
		outf = None

	# check ignore dataID
	# ignoreID.csv must place at some dirname with processed.data
	ignoreID = []
	_path = os.path.dirname(sys.argv[1])
	_ignoreFile = _path + '/ignoreID.csv'
	if os.path.exists(_ignoreFile):
		s_ignoreID = open(_ignoreFile, 'r').read().split('\n')
		s_ignoreID.pop()
		ignoreID = map(int, s_ignoreID)

	meta = {'processed-json-file': sys.argv[1], 'test-sample-cycle': test, 'features-used': features,
			'ignore-id': ignoreID, 'smooth-used': smooth, 'divider-used': divider}
	if test == 0:
		process(inf, outf, features, meta)
	else:
		if not NUS:
			toutf = open(sys.argv[2] + '.test', 'w')
			if not toutf:
				print "Unable to open '%s.test' as output!" % sys.argv[2]
				exit(-1)
		else:
			toutf = None
		if smooth > 1:
			seqf = open(sys.argv[2] + '.seq', 'w')
			if not NUS:
				tseqf = open(sys.argv[2] + '.test.seq', 'w')
			else:
				tseqf = None
			if not NUS:
				if not seqf or not tseqf:
					print "Unable to open one of .seq as output"
					exit(-1)
			else:
				if not seqf:
					print "Unable to open .seq as output"
					exit(-1)
			process_with_test(inf, outf, test, toutf, ignoreID, features=features, smooth=smooth, extraMETA=meta,
							  seqf=seqf, tseqf=tseqf, divider=divider)
			seqf.close()
			if not NUS:
				tseqf.close()
		else:
			process_with_test(inf, outf, test, toutf, ignoreID, features=features, extraMETA=meta, divider=divider)
		if not NUS:
			toutf.close()
	inf.close()
	if not NUS:
		outf.close()
