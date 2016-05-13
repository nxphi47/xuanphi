#!/bin/env python2

# ------------------------------
# feature-eval.py
#	evaluate usefulness of feature(s)
# ------------------------------

import sys, math
import utils

# the features to eval
FEATURES=[
	'refPulseWidth-ave', 'refPulseWidth-immed', 'refPulseWidth-grad',
	'pulseWidth', 'timeWidth', 'normTimeWidth', 'peakWidthDiff', 'peakWidthDiv', 'peakValueDiv', 'areaRatio', 
	'peakValue', 'totalArea', 'peakTime', '+gradRatio', '-gradRatio', '0gradRatio', 'gradChange']

def usage():
	print "Usage: %s <infile> [<features>]" % sys.argv[0]
	print "\tEvaluate the features\n"
	print "Parameters:"
	print "\t<infile>: the input file generated by process-json.py\n"
	print "\t<features>: this is optional.  If specified, it should be a list of feature names"
	print "\t\taccording to names used by feature.py that should be evaluated.  The feature "
	print "\t\tnames are separated by commas (without quote or spaces)."
	print "\t\tDefault: %s\n" % ','.join(x for x in FEATURES)
	print
	exit(0)
	
class FeatEval(object):
	def __init__(self, features=FEATURES):
		self.features = features
		self.stats = { }
		self.labels = [ ]
		self.add_label('_overall_')
		m = 0
		for f in features:
			if len(f) > m:
				m = len(f)
		self._fmtsize = m+1
		self._fmt = "%-" + str(self._fmtsize) + "s "
		self._fmtline = '-' * self._fmtsize
		
	def process(self, inf):
		overall = self.stats['_overall_']
		while True:
			sample = utils.get_sample(inf, self.features)
			if not sample:
				break
			if not sample.has_key('fv'):
				continue
			if not (sample['label'] in self.labels):
				self.add_label(sample['label'])
			s = self.stats[sample['label']]
			for i in xrange(len(sample['fv'])):
				f = sample['fv'][i]
				s[i]['sum'] += f
				s[i]['sum2'] += f * f
				s[i]['num'] += 1
				overall[i]['sum'] += f
				overall[i]['sum2'] += f * f
				overall[i]['num'] += 1
				
	def add_label(self, label):
		ret = []
		for f in features:
			ret.append( { 'sum': 0, 'num': 0, 'sum2': 0 } )		# stats of each feature
		self.stats[label] = ret
		self.labels.insert(-1, label)

	def _calc_stats(self, s):
		if s['num'] > 0:
			s['mean'] = 1. * s['sum'] / s['num']
			s['var'] = 1. * s['sum2'] / s['num'] - s['mean'] * s['mean']
			s['std'] = math.sqrt(s['var'])
		else:
			s['mean'] = s['var'] = s['std'] = -1
		return s
		
	def _calc_each_class(self):
		for k in self.stats:
			for fidx in xrange(len(self.stats[k])):
				self.stats[k][fidx] = self._calc_stats(self.stats[k][fidx])

	def _calc_inter_class(self):
		# initialize interclass matrix
		self._interclass = []
		for f in self.features:
			a = []
			for i in xrange(len(self.labels)-1):
				aa = []
				for j in xrange(len(self.labels)-1):
					aa.append({'m': 0., 's':0., 'v':0.})
				a.append(aa)
			self._interclass.append(a)
		# calculate interclass
		for fidx in xrange(len(self.features)):
			for ki in xrange(len(self.labels)-1):
				for kj in xrange(len(self.labels)-1):
					if not (ki == kj):
						self._calc_inter_class_stats(fidx, ki, kj)
	
	def _calc_inter_class_stats(self, f, ki, kj):
		kistats = self.stats[self.labels[ki]][f]
		kjstats = self.stats[self.labels[kj]][f]
		m = self._interclass[f][ki][kj]['m'] = abs(kistats['mean'] - kjstats['mean'])
		s = self._interclass[f][ki][kj]['s'] = (kistats['std']+kjstats['std'])/2.
		if s < 0.0001:
			print "Error: std=0 (f=%s(%d), ki=%s(%d), kj=%s(%d))" % (
				self.features[f], f, self.labels[ki], ki, self.labels[kj], kj)
		else:
			self._interclass[f][ki][kj]['v'] = m/s
	
	def evaluate(self, f=sys.stdout):
		self._calc_each_class()
		self._calc_inter_class()
		# ---------------------------------------
		# Print Each Class Stats
		# ---------------------------------------
		# print labels headings first
		f.write("\n============================================\n")
		f.write("Statistics Distribution across Each Class:\n")
		f.write("============================================\n")
		f.write(self._fmt % '')
		for k in self.labels[:-1]:
			f.write(" | %4s%-11s%4s " % ('', k, ''))
		f.write(" | %4s%-11s%4s\n" % ('', 'OVERALL', ''))
		# print sub headings
		f.write(self._fmt % '')
		for k in self.labels:
			f.write(" | %9s %9s " % ('  mean', ' std-dev'))
		f.write("\n")
		# print each feature
		for feat in self.features:
			f.write(self._fmt % feat)
			fidx = self.features.index(feat)
			for k in self.labels:
				f.write(" | %9.2e %9.2e " % (self.stats[k][fidx]['mean'], self.stats[k][fidx]['std']))
			f.write("\n")
		# ---------------------------------------
		# Print Inter-class Stats
		# ---------------------------------------
		# print headings first
		f.write("\n=================================\n")
		f.write("Inter-class Feature Separation:\n")
		f.write("=================================\n")
		# print each feature
		for feat in self.features:
			fidx = self.features.index(feat)
			f.write(self._fmt % feat)
			f.write(": mean=%9.2e  std=%9.2e\n" % (self.stats['_overall_'][fidx]['mean'], self.stats['_overall_'][fidx]['std']))
			f.write('%10s ' % '')
			for k in self.labels[:-1]:
				f.write(" | %8s%-11s%8s" % ('', k, ''))
			f.write(" |\n")
			for ki in xrange(len(self.labels)-1):
				f.write("%10s " % self.labels[ki])
				for kj in xrange(len(self.labels)-1):
					if ki == kj:
						f.write(" | %9s %9s  %5s " % ("","",""))
					else:
						f.write(" | %9.2e/%9.2e= %5.2f " % 
							(self._interclass[fidx][ki][kj]['m'], self._interclass[fidx][ki][kj]['s'], self._interclass[fidx][ki][kj]['v']))
				f.write(" |\n")
			f.write("----------------------------------\n")


if __name__ == '__main__':
	if len(sys.argv) < 2:
		usage()
	inf = open(sys.argv[1], 'r')
	if not inf:
		print "Unable to open '%s' as input!" % sys.argv[1]
		exit(-1)
	features = FEATURES
	if len(sys.argv) > 2:
		features = sys.argv[2].split(',')
		
	feval = FeatEval(features)
	feval.process(inf)
	feval.evaluate()