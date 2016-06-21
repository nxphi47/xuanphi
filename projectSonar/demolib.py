import json
import sys
import os
import pprint
import collections
import utils
import numpy as np
import svmclassifier as SVM

'''
	Class for feature extraction
'''
class feature_extract (object):
	def __init__(self, data, feature, smooth=3):
		self.data = data		# processed data structure
		self.feature = feature	# feature to be extract
		self.sm = smooth		# smooth integer
	
	# obtain feature vector based on defined self.feature
	def get_fv(self, obj):
		ret = []
		data = obj['Working']
		for f in self.feature:
			if data.has_key(f):
				ret.append(data[f])
			else:
				print '%s feature not found!!!' % f
				ret.append(0.0)
		return ret
	
	# perform neccessary fv extraction and perform smoothing
	#	return array of smoothed feature
	def run(self, REALTIME=True):
		_sv = []
		_dist = []
		for p in self.data:
			if REALTIME:
				obj = p
			else:
				obj = json.loads(p)
			_sv.append(self.get_fv(obj))
			_dist.append(obj['distance'])
			
		_buf_sv = []
		_fv = []
		for sv in _sv:
			_buf_sv.append(sv)
			_buf_fv = []
			if len(_buf_sv) == self.sm:
				for s in _buf_sv:
					_buf_fv.extend(s)
				_fv.append(_buf_fv)
				_buf_sv.pop(0)
		_dist_round = ['%.2f' % elem for elem in _dist]	
		return _fv, _dist

'''
	perform svm classification
'''
class svm_classify (object):
	def __init__(self, svmFile, fv, dist):
		self.svmFile = svmFile		# svm file
		self.fv = fv				# feature vector to be test
		self.dist = dist
	
	def process_non_real(self, pred, all_labels):
		_most_common = 0.
		for item in set(pred):
			_most_common = item
			break
		
		self.pred_obst = 'None'
		self.pred_height = 'NA'
		_detected_class = all_labels[int(_most_common)]
		if _detected_class[0:1] == 'C':
			self.pred_obst = 'Curb'
			self.pred_height = _detected_class[2:]
		
		'''
			FIXME: might need to add in new class by detect different _detected_class
					'Curb', 'Unknown', 'None'
		'''
		
		self.details = []
		for i in xrange(len(self.fv)):
			_str = 'Class: ' + all_labels[int(pred[i])] + '; '
			_str += 'Dist: %.2f' % self.dist[i]
			_fv = ['%.2f' % elem for elem in self.fv[i]]
			_str += '; fv: [%s] ' % ','.join(x for x in _fv)
			self.details.append(_str)		
		
		'''
			all_labels should be ['c=5', 'c=10', 'c=15', 'unknown']
		'''
		
	
	def run(self):
		self.svm = SVM.SVMClassifier('-q')
		self.svm.load(self.svmFile)
		
		all_labels = []
		for m in self.svm.meta:
			if m.has_key('labels-order'):
				all_labels = m['labels-order']
		
		pred = self.svm.predict(self.fv)
		self.process_non_real(pred, all_labels)

		return self.pred_obst, self.pred_height, self.details
		
