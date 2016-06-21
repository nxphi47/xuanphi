import os, sys, json
			
FEATURES = ['txPulseStart', 'refPulseStart', 'minValue', 'peakValue', 'totalArea', 'areaRatio',
			'normTimeWidth', 'peakTime', 'peakWidthDiff', 'peakWidthDiv', 'peakValueDiv',
			'txPulseWidth', 'txPulseWidth-ave', 'txPulseWidth-immed', 'txPulseWidth-grad',
			'txTimeWidth', 'txTimeWidth-ave', 'txTimeWidth-immed', 'txTimeWidth-grad',
			'pulseWidth', 'refPulseWidth-ave', 'refPulseWidth-immed', 'refPulseWidth-grad',
			'timeWidth', 'refTimeWidth-ave', 'refTimeWidth-immed', 'refTimeWidth-grad',
			'+gradRatio', '-gradRatio', '0gradRatio', 'gradChange']
		
def get_sample_with_obj(inf):
	ins = inf.readline()

	if ins == '':
		return None, None
	try:
		obj = json.loads(ins)
	except:
		# ignore those that is not valid json
		print 'invalid json'
		return None, None
	if obj.has_key('META'):
		return None, None
	return obj, obj['Working']

if __name__ == '__main__':
	if len(sys.argv) < 3:
		print 'Usage python %s [processed.data_file] [FeatureName]' % sys.argv[0]
		print 'FeatureName Supported: '
		_s = ''
		_count = 0
		for i in FEATURES:
			_s += i + '\t\t'
			_count += 1
			if _count >= 2:
				_count = 0
				print _s
				_s = ''
		exit(1)
		
		
	if os.path.exists(sys.argv[1]):
		inf = open(sys.argv[1], 'r')
		while True:
			obj, fv = get_sample_with_obj(inf)
			if not obj:
				print 'invalid'
				break

			if fv.has_key(sys.argv[2]):
				print sys.argv[2], obj['DataID'], fv[sys.argv[2]]
			else:
				print '%s not found...' % sys.argv[2]
			
