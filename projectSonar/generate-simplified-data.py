import json, sys
import itertools

# the feature vector to extract
#FEATURES=['pulseWidth', 'peakValue', 'totalArea', 'peakTime']
FEATURES=['normTimeWidth', 'timeWidth', 'peakValue', 'totalArea', 'peakTime', '+gradRatio', '-gradRatio', '0gradRatio' ]

# Extract the feature vector according to the order given in <features>
def get_feature_vector(obj, features=FEATURES):
	ret = []
	data = obj['Working']
	for f in features:
		if data.has_key(f):
			ret.append(data[f])
		else:
			ret.append(0.0)
	return ret

# get the curb height from filename
# Make sure the filename has 'curb' before using this function
def get_curb_height(filename):
	name_label = filename.split('-')
	for x in name_label:
		if 'curb' in x:
			try:
				return float(x[4:])
			except ValueError, e:
				print "Could not convert curb height to float from filename: ", filename
	return -1.

# Guess the label based on the filename of the raw json file
def guess_label_from_filename(filename, CLASS="Normal"):
	if CLASS == "Normal":
		if 'wall' in filename:
			return 'WALL'
		if ('pole' in filename) or ('roll' in filename):
			return 'POLE'
		# other case which is curb
		if 'curb' in filename:
			_height = get_curb_height(filename)
			if _height > 0.:
				'''
				if _height >= 15.:
					return 'C>=15'
				else:
					return 'C<15'
				'''
				return 'C=' + str(get_curb_height(filename))
		return ''
	elif CLASS == "Surface":
		if 'rough' in filename:
			return 'ROUGH'
		else:
			return 'SMOOTH'
	
def get_sample_with_obj(inf, features, META=False):
	ins = inf.readline()
	if ins == '':
		return None, None
	try:
		obj = json.loads(ins)
	except:
		# ignore those that is not valid json
		print 'invalid json'
		return { 'invalid':None }, None
	if obj.has_key('META'):
		if not META:
			print 'Metadata ignored'
			return { 'META':None }, None
		else:
			return { 'META':obj['META'] }, obj
	if not obj.has_key('filename'):
		# ignore those that does not contain filename
		print "missing key: 'filename'"
		return { 'invalid':None }, None
	if not obj.has_key('DataID'):
		# ignore those that does not contain filename
		print "missing key: 'DataID'"
		return { 'invalid':None }, None
	label = guess_label_from_filename(obj['filename'])
	if label == '':
		# ignore those that cannot be labelled
		print "cannot estimate label from filename: '%s'" % obj['filename']
		return { 'invalid':None }, None
	return { 'fv': get_feature_vector(obj, features), 'label': label, 'id': obj['DataID'], 'filename': obj['filename']}

def process(inf, outf, features=FEATURES):
	outf.write('DataID\tLabel\tFilename\t%s\n' % ('\t'.join("%s" % s for s in FEATURES)))
	while True:
		sample = get_sample_with_obj(inf, features, META=False)
		if not sample:
			break
		if sample.has_key('fv'):
			#outf.write('%d\t%s\t%s\t%s\n' % (sample['id'], sample['label'], sample['filename'], '\t'.join("%s" % s for s in sample['fv'])))
			outf.write(json.dumps(sample) + '\n')
			
def process_by_id(inf, _id, features=FEATURES):
	while True:
		sample = get_sample_with_obj(inf, features, META=False)
		if not sample:
			break
		if sample['id'] == _id:
			print
			print 'DataID:\t\t%d' % sample['id']
			print 'Filename:\t%s' % sample['filename']
			print 'Label:\t\t%s' % sample['label']
			print 'fv:'
			for f,s in zip(features, sample['fv']):
				print '\t\t%s: %s' % (f, s)
			print
			break

def check_input_file(filepath):
	inf = open(filepath, 'r')
	if not inf:
		print "Unable to open '%s' as input!" % sys.argv[1]
		print 
		exit(1)
	return inf

if __name__ == '__main__':
	if len(sys.argv) == 3:
		inf = check_input_file(sys.argv[1])
		features = FEATURES
		process_by_id(inf, int(sys.argv[2]), features)
		exit(1)
	elif len(sys.argv) > 3:
		print 'Usage %s [proceed-file-name] <Dataid>' % sys.argv[0]
		print 
		exit(1)
	
	inf = check_input_file(sys.argv[1])
	features = FEATURES
	outf = open(sys.argv[1] + '.extract', 'w')
	process(inf, outf, features)
	outf.close()
	inf.close()
