import sys, json, os, random, random
import matplotlib.pyplot as plt
import matplotlib.patches as mpatches

DATAs = []
COLOR = [ "blue", "red", "black", "yellow", "purple", "orange", "white", "green" ]
title = []
dirname = ''
distance = ''

def scan_input(_file):
	dirname = os.path.dirname(_file)
	with open(_file) as json_file:
		DATAs.append(json.load(json_file))

def mat_plot_all():
	fig, ax = plt.subplots()
	_legend = []
	
	_max_envelop = 0
	_min_envelop = 100
	list_max = []
	list_min = []
	_time_ref = ''
	
	_counter = 0
	for data in DATAs:
		_list_max = []
		_list_min = []
		
		for x in data['index']:
			_maxKey = 'max' + str(x)
			if data.has_key(_maxKey):
				_list_max.append(data[_maxKey])
			_minKey = 'min' + str(x)
			if data.has_key(_minKey):
				_list_min.append(data[_minKey])
		
		_ave_max_index = reduce(lambda x, y: x + y, _list_max) / len(_list_max)
		list_max.append(_ave_max_index)
		_ave_min_index = reduce(lambda x, y: x + y, _list_min) / len(_list_min)
		list_min.append(_ave_min_index)
		
		for x in data['index']:
			_timeKey = 't' + str(x)
			_envKey = 'e' + str(x)
			_minKey = 'min' + str(x)
			_maxKey = 'max' + str(x)
			
			if data.has_key(_timeKey) and data.has_key(_envKey):
				
				if data[_maxKey] is None:
					print "Skip range calc for [%s] graph as no reflected signal detected" % x
					continue
					
				_time_ref = _timeKey
				plt.plot(data[_timeKey], data[_envKey], color=COLOR[_counter])
				
				for x in data[_envKey][(_ave_max_index - 10 - data[_minKey]):]:
					if x > _max_envelop:
						_max_envelop = x
					if x < _min_envelop:
						_min_envelop = x
			else:
				print("No Key for %s or %s" % (_timeKey, _envKey))
		_legend.append(mpatches.Patch(color=COLOR[_counter], label=title[_counter]))
		_counter += 1
		
	ave_max = reduce(lambda x, y: x + y, list_max) / len(list_max)
	ave_min = reduce(lambda x, y: x + y, list_min) / len(list_min)
	ax.set_xlim(data[_time_ref][ave_max-ave_min-8], data[_time_ref][ave_max-ave_min+30])
	ax.set_ylim(_min_envelop - 20, _max_envelop + 20)
	plt.title(distance)
	#plt.legend(handles=_legend, loc='upper right')	
	ax.set_xlabel('Time (ms)')
	ax.set_ylabel('Envelop')
	plt.savefig(os.path.join(dirname, 'com-'+distance))	

if __name__ == "__main__":
	if len(sys.argv) < 3 or len(sys.argv) > 9:
		print "Usage:	%s [File1] [File2] ... [File8]" % sys.argv[0]
		exit(1)
	
	_files = []
	for x in xrange (len(sys.argv)):
		if x > 0:
			_name = os.path.abspath(sys.argv[x])
			_files.append(_name)
			scan_input(_name)
			t = sys.argv[x].split('-')
			title.append(t[3] + '-' + t[4])
			distance = t[3]
	mat_plot_all()
