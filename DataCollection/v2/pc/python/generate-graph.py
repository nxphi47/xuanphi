import sys, json, os
import utils
import matplotlib.pyplot as plt
from collections import defaultdict
from matplotlib import gridspec

# the feature vector to extract
#FEATURES=['pulseWidth', 'peakValue', 'totalArea', 'peakTime']
FEATURES=['normTimeWidth', 'timeWidth', 'peakValue', 'totalArea', 'peakTime', '+gradRatio', '-gradRatio', '0gradRatio' ]
CLASS=['C<15', 'C>15', 'POLE', 'WALL']
COLOR = [ "blue", "green", "red", "magenta", "cyan", "yellow", "black" ]

def usage():
	print "usage \t   %s -f [train_file_name] [processed.data_file]" % sys.argv[0]
	print "	or %s -fv [train_file_name] [processed.data_file]" % sys.argv[0]
	print '	or %s -p [processed.data_file]' % sys.argv[0]
	print '	or %s -l [processed.data_file] [hazard=]' % sys.argv[0]
	print '	-f	- use training.sample to plot error feature'
	print '	-fv	- use training.sample to plot error feature with annotate'
	print '	-p	- use processed.data to plot graph by class'
	print '	-l	- use processed.data to plot graph by hazard'
	exit(1)

# check input file whehter is it valid or not
#	if valid, return opened file as read
def check_input_file(filepath):
	inf = open(filepath, 'r')
	if not inf:
		print "Unable to open '%s' as input!" % sys.argv[1]
		print 
		exit(1)
	return inf

# class to plot signal by classification class and hazard
class PlotAll(object):
	def __init__ (self, inf):
		self.inf = check_input_file(inf)
		self.fv = FEATURES	
		self.svmclass = CLASS
	
	# since plotting for classification class will split single plot to multiple graph
	#	hence need to obtain dimension for matplotlib 
	def getbin(self, counter):
		_bin = bin(counter)
		_ix = 0
		_iy = 0
		if len(_bin) < 4:
			_ix = 0
			_iy = int(_bin[2])
		else:
			_ix = int(_bin[2])
			_iy = int(_bin[3])
		return _ix, _iy
	
	# defined dimension for matplotlib in hazard plotting
	def get_ab (self, length):
		if length == 1:
			return 1, 1
		elif length == 2:
			return 2, 2
		elif length == 3:
			return 2, 3
		elif length == 4:
			return 3, 2
		elif length == 5 or length == 6:
			return 3, 3
		elif length >= 7 and length <= 9:
			return 4, 3
		elif length >= 10 and length <= 16:
			return 4, 4
	
	# get matplotlib dimension and return a list contain plot dimension
	def get_shape (self, length):
		_a, _b = self.get_ab(length)
		fig = plt.figure(figsize=(16.0,12.0))
		_axl = []
		
		# first plot should always alone and plot all data into same graph
		_axl.append(plt.subplot2grid((_a, _b), (0,0)))
		
		if _a == 1 and _b == 1:
			return fig, _axl
		
		# after loop based on _a & _b
		_break = False
		_int = 0
		for x in range (1, _a):
			for y in range (_b):
				_axl.append(plt.subplot2grid((_a, _b), (x,y)))
				_int += 1
				if _int == length:
					_break = True
					break
			if _break:
				break
		return fig, _axl		
	
	# plot graph based on hazard 
	def plot_hazard (self, hazard_list):
		_label = hazard_list.split('=')[1]
		_hazard = _label.split(',')
		for h in _hazard:
			_obj = utils.get_sample_by_hazard(self.inf, h)
			
			# remove duplicated filename and store in new list
			_files = []
			for o in _obj:
				_files.append(o['filename'])
			_lfile = list(set(_files))
			
			# get shape for matplotlib
			fig, _axl = self.get_shape(len(_lfile))
				
			# plot all reflect signal		
			for o in _obj:
			#	print o['DataID'], o['filename'], o['Working']['refEnvList'], o['Working']['refTimeList']
				if 'refTimeList' in o['Working'].keys() and 'refEnvList' in o['Working'].keys():
					_x = o['Working']['refTimeList']
					_y = o['Working']['refEnvList']
				else:
					continue
				_index = _lfile.index(o['filename'])
				
				_axl[0].set_title(h)
				_axl[0].plot(_x, _y, color=COLOR[_index])
				_axl[0].set_xlim(20, 25)
				_axl[0].set_ylim(40, 180)
			
			# sometime it only contain one data file, hence no need to split data plot
			if not len(_lfile) == 1:			
				# plot reflect signal based on filename	in separate graph
				_i = 0
				for f in _lfile:
					_index = _lfile.index(f)
					_sf = f.split('-', 2)
					
					_i += 1
					for o in _obj:
						if o['filename'] == f:
							if 'refTimeList' in o['Working'].keys() and 'refEnvList' in o['Working'].keys():
								_x = o['Working']['refTimeList']
								_y = o['Working']['refEnvList']
								
								_axl[_i].set_title(_sf[2][:-5])
								_axl[_i].plot(_x, _y, color=COLOR[_index])
								_axl[_i].set_xlim(20, 25)
								_axl[_i].set_ylim(40, 180)
				
			#plt.show()
			_filename = os.path.realpath(sys.argv[2]).replace('.', '_') + '-h-' + h
			plt.savefig(_filename)
			plt.close('all')		
		
	# plot graph based on classfiction class	
	def plot_class (self):
		plt.close('all')
		fig, ax = plt.subplots(2,2, figsize=(16.0,12.0))
		
		while True:
			sample = utils.get_sample(self.inf, self.fv, META=True, NUS=True)
			if sample is None:
				break
			if sample.has_key('META'):
				break
			
			_index = self.svmclass.index(sample['label'])
			_ix, _iy = self.getbin(_index)
						
			_len = len(sample['fv'])
			_x = sample['fv'][_len - 1]
			_y = sample['fv'][_len - 2]
			
			ax[_ix, _iy].set_title(sample['label'])
			ax[_ix, _iy].plot(_x, _y, color=COLOR[_index])
			ax[_ix, _iy].set_xlim(20, 25)
			ax[_ix, _iy].set_ylim(40, 180)
		
		#plt.show()
		_filename = os.path.realpath(sys.argv[2]).replace('.', '_')
		plt.savefig(_filename)
		plt.close('all')			

class CheckWrong(object):
	def __init__ (self, train_file, inf, ANN=False):
		self.trainf = check_input_file(train_file)
		self.testf = check_input_file(train_file + '.test')
		self.woutf = check_input_file(train_file + '.output')
		self.inf = check_input_file(inf)
		self.ann = ANN
		self.fv = FEATURES
		self.svmclass = CLASS					
	
	# plot graph based on feature
	def feature_plot (self):
		# extract only show-wrong result
		#	first show-wrong output is list of sample cross-validating wrongly
		#	second show-wrong output is list of sample predicted wrongly
		_wout = self.woutf.readlines()
		_counter = 0
		
		for _w in _wout:
			if len(_w) > 1:
				try:
					_load = eval(_w)
					if 'predicted' in str(_load):
						if _counter == 1:
							self.wout = _load
							break
						else:
							_counter += 1
				except:
					pass
					
		_wrongID = []
		_wrongPredict = []
		for w in self.wout:
			_wrongID.append(w['id'])
			_wrongPredict.append(w['predicted'])
		
		# create a dict that handle keyError automatically
		#	In this case, each key contain an array
		self.dicti = defaultdict(list)
		
		
		while True:
			sample = utils.get_sample(self.inf, self.fv, META=True, NUS=True)
			
			# refTimeList and refEnvList is not require for feature plotting
			if 'refTimeList' in self.fv:
				self.fv.remove('refTimeList')
			if 'refEnvList' in self.fv:
				self.fv.remove('refEnvList')
				
			if sample is None:
				break
			if sample.has_key('META'):
				break
			
			# get label id in svm and dataid, after that import into dicti
			_index = self.svmclass.index(sample['label'])
			
			self.dicti['id'].append(sample['id'])
			self.dicti['counter'].append(_index)
			for x in xrange (len(self.fv)):
				self.dicti[self.fv[x]].append(sample['fv'][x])
			
		# start plot the graph based on feature vector given
		for x in xrange(len(self.fv)):
			_fv = self.fv[x]
			print 'Processing feature: ', _fv
			
			plt.close('all')
			fig, ax = plt.subplots(figsize=(16.0,12.0))
			
			# defined output file name, if annotation indicated, addin '-ann' as suffix
			_filename = os.path.realpath(sys.argv[2]).replace('.', '_') + '-fv-' + _fv
			if self.ann:
				_filename += '-ann'
			
			# applied scatter plot for each id
			for y in xrange(len(self.dicti['id'])):
				_id = self.dicti['id'][y]
				_val = self.dicti[_fv][y]
				_cout = self.dicti['counter'][y]
				
				ax.scatter(_id, _val, color=COLOR[_cout], s=2.0, cmap=plt.get_cmap('Spectral'), label=self.svmclass[_cout])
				
				# if id idicate as wrong classification in svm result, place a marker or apply annotation
				if _id in _wrongID:
					_in = _wrongID.index(_id)
					_col = self.svmclass.index(_wrongPredict[_in])
					if self.ann:
						ax.annotate(str(_id), xy=(_id,_val), xytext=(-20,60), textcoords = 'offset points', ha = 'right', va = 'bottom', 
							bbox = dict(boxstyle = 'round,pad=0.2', fc = COLOR[_col], alpha = 0.2), arrowprops = dict(arrowstyle = '->', 
							connectionstyle = 'arc3,rad=0'))
					else:
						ax.plot(_id, _val, marker='+', markersize=5, color='black')
						ax.text(_id, -0.1, str(_id), ha='center', rotation=90, color=COLOR[_cout], fontsize=8.)
			
			# remove duplicated legend
			handles, labels = ax.get_legend_handles_labels()
			handle_list, label_list = [], []
			for handle, label in zip(handles, labels):
				if label not in label_list:
					handle_list.append(handle)
					label_list.append(label)
			plt.legend(handle_list, label_list)
			
			ax.set_xlabel('DataID')
			plt.title(_fv)
			plt.savefig(_filename)
			plt.close('all')
			
if __name__ == "__main__":
	if len(sys.argv) < 3:
		usage()
	
	if '-f' in sys.argv[1]:
		if len(sys.argv) < 4:
			usage()
		if '-fv' in sys.argv[1]:
			cw = CheckWrong(sys.argv[2], sys.argv[3], ANN=True)
		else:
			cw = CheckWrong(sys.argv[2], sys.argv[3], ANN=False)
		cw.feature_plot()
	elif '-p' in sys.argv[1]:
		pa = PlotAll(sys.argv[2])
		pa.plot_class()
	elif '-l' in sys.argv[1]:
		if len(sys.argv) < 4:
			usage()	
		if not 'hazard' in sys.argv[3]:
			usage()
		pa = PlotAll(sys.argv[2])
		pa.plot_hazard(sys.argv[3])
	else:
		usage()
