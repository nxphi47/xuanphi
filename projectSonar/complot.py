import sys, json, os, random, random
import matplotlib.pyplot as plt
import matplotlib.patches as mpatches

DATAs = []
#COLOR = [ "black", "red", "blue", "purple", "orange", "purple", "yellow", "white", "green" ]		# exclude wall
#COLOR = [ "black", "red", "blue", "purple", "green", "green", "orange", "green" ]		# include wall
COLOR = ['blue', 'purple']															# individual
NAME = ''

def scan_input(_file):
	dirname = os.path.dirname(_file)
	with open(_file) as json_file:
		DATAs.append(json.load(json_file))

def mat_plot_all():
	fig, ax = plt.subplots()
	_legend = []
	
	_max_env = 0
	_min_env = 1000
	_start_time_ind = 1000
	_end_time_ind = 0
	
	_counter = 0
	
	for data in DATAs:
		for x in data['index']:
			_in = str(x)
			oriTimeList = 'otl' + _in		# original time array
			initIndex = 'iId' + _in		# index for signal output
			refEnvList = 'rel' + _in		# reflect signal envelop array (until saturation)
			refTimeList = 'rtl' + _in		# reflect signal time array (until saturation)
			refIndex = 'rId' + _in			# index for reflect signal
			
			if data.has_key(refIndex):
				if data[refIndex] == 0:
					print 'No reflect signal detected for index: ', _in
					continue
				
				# align by cycle offset time accordingly
				_offset_time = data[oriTimeList][data[initIndex]]
				_offset_list = [x - _offset_time for x in data[refTimeList]]
				
				plt.plot(_offset_list, data[refEnvList], color=COLOR[_counter])
			else:
				print("No Key for %s or %s" % (refTimeList, refEnvList))
		_counter += 1
	
	ax.set_xlim(17, 20)
	ax.set_ylim(40, 180)
		
	plt.title(NAME)
	plt.savefig('com-'+NAME)

if __name__ == "__main__":
	if len(sys.argv) < 3 or len(sys.argv) > 9:
		print "Usage:	%s [NAME] [File1] [File2] ... [File8]" % sys.argv[0]
		exit(1)
	
	NAME = sys.argv[1]
	NAME = NAME.replace('.', ',')
	_files = []
	for x in xrange (len(sys.argv)):
		if x > 1:
			_name = os.path.abspath(sys.argv[x])
			_files.append(_name)
			scan_input(_name)
	mat_plot_all()
