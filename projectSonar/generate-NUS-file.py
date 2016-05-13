import sys, json, os
import utils

FIX_LEN = True
LEN = 40
# the feature vector to extract
#FEATURES=['pulseWidth', 'peakValue', 'totalArea', 'peakTime']
FEATURES=['peakValue', 'totalArea', 'peakTime', 'refTimeWidth-immed', 'areaRatio']

class GETOUTPUT(object):
	def __init__ (self, inf, outf, features):
		self.inf = inf
		self.outf = outf
		self.fv = features
		self.bypass = False
		self.bypasslist = []
	
	def list_to_string(self, arr):
		if not arr == 0.0:
			_s = ''
			for l in arr:
				_s += str(l) + ','
			return _s[:-1]
		else:
			self.bypass = True
			return 'None'
	
	def get_time_env_list(self, obj):
		_refIndex=0
		if obj.has_key('Working'):
			_refIndex = obj['Working']['refIndex']
		
		_time = ''
		_env = ''
		if not _refIndex == 0:
			_ref = int(_refIndex) - 5
			for _index, env in enumerate(obj['Envelop']):
				if _index >= _ref and _index < (_ref + LEN):
					_time += str(obj['Timing'][_index]) + ','
					_env += str(obj['Envelop'][_index]) + ','
		return _time[:-1], _env[:-1]
			
	def process (self):
		self.outf.write('ID\tTime\tEnvelop\tFeatures\tactual_distance\tactual_height\thazard\tsurface\tmovement\tfilename\n')
		while True:
			sample, obj = utils.get_sample_with_obj(self.inf, self.fv, META=True, NUS=True)
			if sample.has_key('META'):
				break
			_s = ''
			_len = len(sample['fv'])
			print 'Processing ID: ', sample['id']
			_s += str(sample['id']) + '\t'
			if FIX_LEN:
				_time, _env = self.get_time_env_list(obj)
				_s += _time + '\t'
				_s += _env + '\t'
			else:
				_s += self.list_to_string(sample['fv'][_len - 1]) + '\t'
				_s += self.list_to_string(sample['fv'][_len - 2]) + '\t'
			_s += self.list_to_string(sample['fv'][:-2]) + '\t'
			_s += str(sample['actual_distance']) + '\t'
			_s += str(sample['actual_height']) + '\t'
			_s += str(sample['hazard']) + '\t'
			_s += str(sample['surface']) + '\t'
			_s += str(sample['movement']) + '\t'
			_s += str(sample['filename']) + '\n'
			if self.bypass:
				self.bypasslist.append(sample['id'])
				self.bypass = False
			else:
				self.outf.write(_s)
		print 'ByPassed ID: ', self.bypasslist

if __name__ == "__main__":
	if len(sys.argv) < 2:
		print "usage %s [processed_file]" % sys.argv[0]
		exit(1)
	
	inf = open(sys.argv[1], 'r')
	outf = open(sys.argv[1]+'-NUS.csv', 'w')
	features = FEATURES

	get_output = GETOUTPUT(inf, outf, features)
	get_output.process()
	
	inf.close()
	outf.close()
