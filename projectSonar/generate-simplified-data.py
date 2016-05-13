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

def guess_label_from_filename(filename):
	if 'wall' in filename:
		return 'WALL'
	if ('pole' in filename) or ('roll' in filename):
		return 'POLE'	
	if ('curb10' in filename) or ('curb7.5' in filename):
		return 'C<15'
	if ('curb20' in filename) or ('curb37.5' in filename) or ('curb58' in filename):
		return 'C>15'
	return ''
	
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
