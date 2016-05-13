#!/bin/env python2

# -------------------------
# process-json.py
#	Adapted from Kian Soon's json2graph.py to generate only the feature vector
# -------------------------


import sys, json, os
import matplotlib.pyplot as plt
import ConfigParser, argparse
from argparse import RawTextHelpFormatter
from feature import singleReflect


# Convenience: config or default value
def get_cfg(cfg, k, default):
    if cfg.has_key(k) and (cfg[k] != None):
        return cfg[k]
    else:
        return default


# converting condition string to boolean
def convert_cfg(status):
    if isinstance(status, bool):
        return status
    if 'True' in status:
        return True
    else:
        return False


# read config file
def configSectionMap(Config, section, _dict=None):
    if _dict is None:
        _dict = {}
    options = Config.options(section)
    for option in options:
        try:
            _dict[option] = Config.get(section, option)
            if _dict[option] == -1:
                print("Error: skip reading config: %s" % option)
        except:
            print("Error: exception on %s!" % option)
            _dict[option] = None
    return _dict


# actual class to process the data
class JSONProcessor(object):
    def __init__(self, config, _dict, root):
        self.config = config
        self.cfg = _dict
        self.root = root
        self.settings = {}
        self.settings['temperature'] = float(get_cfg(self.cfg, 'temperature', '22'))
        self.settings['threshold'] = float(get_cfg(self.cfg, 'threshold', '5'))
        self.settings['window'] = float(get_cfg(self.cfg, 'windows', '100'))
        self.settings['averaging'] = int(get_cfg(self.cfg, 'averaging', '3'))
        self.settings['outputDetail'] = convert_cfg(get_cfg(self.cfg, 'output_detail', 'True'))
        self.settings['outputWave'] = convert_cfg(get_cfg(self.cfg, 'output_wave', 'True'))
        self.settings['actualDist'] = float(get_cfg(self.cfg, 'actual_dist', '3'))
        self.settings['actualHeight'] = float(get_cfg(self.cfg, 'actual_height', '3'))
        self.settings['features'] = ['timeWidth', 'peakValue', 'totalArea', 'peakTime']
        self.settings['zero-gradient'] = int(get_cfg(self.cfg, 'zerograd', '3'))
        self.settings['pulse-end-detect-method'] = int(get_cfg(self.cfg, 'endpulse', '1'))
        self.settings['valid-id-file'] = get_cfg(self.cfg, 'validid', 'validid.csv')
        self.settings['output_graph'] = convert_cfg(get_cfg(self.cfg, 'output_graph', 'False'))
        self.settings['distance_marker'] = convert_cfg(get_cfg(self.cfg, 'distance_marker', 'False'))
        self.validid = {}
        # if len(self.settings['valid-id-file']) > 0:
        #	self.load_validid()
        self.data_id = 0

    # return the important settings metadata
    def get_metadata(self, info={}):
        _METADATA = ['temperature', 'threshold', 'window', 'averaging', 'actualDist',
                     'actualHeight', 'zero-gradient', 'pulse-end-detect-method', "RPi",
                     'valid-id-file']
        for k in self.settings:
            if k in _METADATA:
                info[k] = self.settings[k]
        return {'META': info}

    # load object from a filename
    def load_file(self, filename):
        ret = []
        self.inf = open(os.path.join(self.root, filename), "r")
        print 'Processing: ', filename
        while True:
            s = self.inf.readline()
            if s == '':
                break
            if not ('{' in s):
                continue
            obj = self.get_json_object(s)
            if not self.check_valid_id(obj, filename):
                continue
            data = self.process_object(obj)
            if data:
                data['filename'] = filename
                self.data_id += 1
                data['DataID'] = self.data_id
                ret.append(data)
                print "Sample %d - OK, assigned ID: %d" % (obj['id'], data['DataID'])
                if self.settings['output_graph']:
                    self.plot_graph(data)
        self.inf.close()
        return ret

    # getting JSON object
    def get_json_object(self, sbegin):
        stmp = sbegin.replace("'", '"')
        while not ('}' in stmp):
            s = self.inf.readline()
            if s == '':
                break
            stmp += ' ' + s.replace("'", '"')
            if '}' in s:
                break
        stmp = stmp.replace('\n', ' ')
        # print stmp
        return eval(stmp)

    # process the input JSON object
    def process_object(self, obj):
        if obj == None:
            return
        if obj.has_key("sensors"):
            # setting of rpi
            self.settings.update({"RPi": obj})
        else:
            # should be sensor data
            data = {'Start-Time': [obj['start'][0], obj['start'][1]], 'Timing': [], 'Envelop': []}
            if obj.has_key('recv-sensor'):
                data['RX-Sensor'] = obj['recv-sensor']
            if obj.has_key('firing-sensor'):
                data['TX-Sensor'] = obj['firing-sensor']
            if obj.has_key('id'):
                data['Sample-ID'] = obj['id']
            # convert timing data
            t = 0.0
            for d in obj['samples']:
                td = d[0] / 1000.0
                t += td
                data['Timing'].append(t)
                data['Envelop'].append(float(d[1]))
            # calculate threshold index range
            _min, _max = self.cal_range(data)
            if (not _min == 0) and (not _max == 0):
                # process the detected single Reflect signal
                sr = singleReflect(data['Envelop'], data['Timing'], _min, _max)
                sr.split_array()
                data['Working'] = sr.process_data()
                # data['Feature-Vector'] = sr.get_feature_vector(self.settings['features'])
                return data
            else:
                print "Sample %d - no reflected signal detected" % obj['id']
                return None

    # calculate distance
    def get_dist(self, xmin, xmax):
        _sound_speed = 331.4 + .61 * (self.settings['temperature'])
        _travel_time = (xmax - xmin) / 1000
        return _sound_speed * _travel_time / 2

    # threshold should vary based on distance
    # need to explore more on this
    def get_threshold(self, data, emax, ecur):
        _start = data['Timing'][emax]
        _to = data['Timing'][ecur]
        if _start == _to:
            return 0.
        _dist = self.get_dist(_start, _to)
        _thres = (1. / _dist) * 10
        return _thres

    # calculate index range for start and stop peak
    def cal_range(self, data):
        _first_index = 0
        _second_index = 0
        for i, x in enumerate(data['Envelop']):
            _ave = 0
            for y in xrange(self.settings['averaging']):
                _ave += data['Envelop'][i - (y + 1)]
            _value = x - (_ave / self.settings['averaging'])
            if _first_index == 0:
                if _value >= 5:
                    _first_index = i
            elif _value >= self.get_threshold(data, _first_index, i):
                # sometime value will change from 52 -> 54 and cause trigger of threshold
                #	this is to make sure if threshold trigger, envelop should more than 55
                if data['Envelop'][i] <= 55:
                    continue
                if (i - _first_index) > self.settings['window']:
                    _second_index = i
                    break
        # handling error when change for _max too close to end
        if (len(data['Envelop']) - _second_index) < 10:
            _second_index = 0
        return _first_index, _second_index

    # Load the valid ID file
    def load_validid(self, path=''):
        if len(self.settings['valid-id-file']) < 0:
            return
        if path == '':
            path = self.root
        _fname = os.path.join(path, self.settings['valid-id-file'])
        if os.path.isfile(_fname):
            with open(_fname, 'r') as _f:
                while True:
                    _s = _f.readline()
                    if _s == '':
                        break
                    if len(_s) < 3:
                        continue
                    _s = _s.strip()
                    if _s[0] == '#':
                        continue  # comment
                    _tmp = _s.split(',')
                    self.validid[_tmp[0]] = (int(_tmp[1]), int(_tmp[2]))

    # Check if ID is valid
    def check_valid_id(self, obj, filename):
        if not self.validid.has_key(filename):
            # no valid id for this filename, assumed all valid
            return True
        if not obj.has_key('id'):
            return True
        # print "CurrentID: %d, check within %d - %d" % (obj['id'], self.validid[filename][0], self.validid[filename][1])
        if obj['id'] < self.validid[filename][0] + 1:
            print "Sample %d < %d  -- ignored." % (obj['id'], self.validid[filename][0])
            return False
        if obj['id'] > self.validid[filename][1] - 1:
            print "Sample %d > %d  -- ignored." % (obj['id'], self.validid[filename][1])
            return False
        return True

    # plot graph for valid id
    def plot_graph(self, data):
        fig, ax1 = plt.subplots()

        _color = 'blue'
        _width = 2.0
        ax1.plot(data["Timing"], data["Envelop"], color=_color, linewidth=_width)
        ax1.set_ylim(0, 200)
        ax1.set_xlabel('Time (ms)')
        ax1.set_ylabel('Envelop', color=_color)

        if self.settings['distance_marker']:
            _initIndex = data["Working"]["txPulseStart"]
            _refIndex = data["Working"]["refPulseStart"]
            _initTime = data["Timing"][_initIndex]
            _refTime = data["Timing"][_refIndex]
            _dist = str('%.3f' % self.get_dist(_initTime, _refTime)) + ' m'

            _color = 'red'
            _width = 1.0
            plt.axvline(x=_initTime, color=_color, linewidth=_width, linestyle='dashed')
            plt.axvline(x=_refTime, color=_color, linewidth=_width, linestyle='dashed')
            ax1.text((_refTime + _initTime) / 2, 12, _dist, ha='center', color=_color)
            ax1.annotate('', (_initTime, 10), (_refTime, 10),
                         arrowprops=dict(arrowstyle='<->', color=_color, linewidth=_width, linestyle='dashed'))

        _title = 'ID-' + str(data["DataID"]) + '_' + data["filename"][:-5]
        plt.title(_title)
        plt.savefig(os.path.join(self.root, _title.replace('.', ',')))
        plt.close('all')


# prepare argparse to read command from user input
#	all parameters can be changed without modified config file
def read_parser():
    parser = argparse.ArgumentParser(description='Convert data to json and plot graph',
                                     formatter_class=RawTextHelpFormatter)

    # adding argument (SectionOne)
    parser.add_argument('-dir', dest="inputdir", required=True, help="Directory contain json file(s)")
    parser.add_argument('-c', dest='config', default=os.path.join(os.getcwd(), 'config.ini'),
                        help="Getting config file")
    parser.add_argument('-d', dest='actualdist', default="3", type=float,
                        help="Setting for actual horizontal distance (use for wave output). \n\t[default: %(default)s]")
    parser.add_argument('-g', dest='actualheight', default="3", type=float,
                        help="Setting for actual height of curb/pole (use for wave output). \n\t[default: %(default)s]")
    parser.add_argument('-t', dest='temperature', default="22", type=float,
                        help="Setting temperature information. \n\t[default: %(default)s]")
    parser.add_argument('-w', dest='winsize', default="100", type=int,
                        help="Windows size to ignore envelop exist threshold. \n\t[default: %(default)s]")
    parser.add_argument('-e', dest='threshold', default="5", type=float,
                        help="Setting envelop threshold to trigger soundwave detection. \n\t[default: %(default)s]")
    parser.add_argument('-a', dest='averaging', default="3", type=int,
                        help="Number of previous sampling use for threshold calculation. \n\t[default: %(default)s]")

    # adding argument (SectionTwo)
    parser.add_argument('-od', default=True, action="store_false",
                        help="Output Detail, Output info to *-det.txt. \n\t[default: %(default)s]")
    parser.add_argument('-ow', default=True, action="store_false",
                        help="Output Wave, Output only wave with signal (special for NUS). \n\t[default: %(default)s]")
    parser.add_argument('-og', default=True, action="store_false",
                        help="Output Graph, Output all graph. \n\t[default: %(default)s]")
    parser.add_argument('-sg', default=False, action="store_true",
                        help="Show Graph, Show graph when run. \n\t[default: %(default)s]")
    parser.add_argument('-sa', default=False, action="store_true",
                        help="Secondary Axis, Plot secondary axis (in this case delta-time). \n\t[default: %(default)s]")
    parser.add_argument('-dm', default=True, action="store_false",
                        help="Distance Marker, Show distance calculation marker during plot. \n\t[default: %(default)s]")
    parser.add_argument('-cp', default=True, action="store_false",
                        help="Combine signal from all cycle into single graph. \n\t[default: %(default)s]")
    parser.add_argument('-cw', default=True, action="store_false",
                        help="Combine output reflected envelop to a file. \n\t[default: %(default)s]")

    # output filename
    parser.add_argument('-out', dest='outfilename', default=os.path.join(os.getcwd(), 'processed.data'),
                        help="Output Filename")
    # end pulse method
    parser.add_argument('-ep', dest='endpulse', default=1, type=int,
                        help="End Pulse method: 0=averaging, 1=immediate, 2=gradient")
    # valid sample id
    parser.add_argument('-vid', dest='validid', default="validid.csv",
                        help="CSV file containing valid ID range (inclusive) for each raw json file")

    return parser.parse_args()


# since there have two ways to configure, if user insert command, need to overwrite parameter
#	read from config file to user setting
def update_value(cmd, args, _dict):
    if '-d' in cmd:
        _dict['actual_dist'] = args.actualdist
    if '-g' in cmd:
        _dict['actual_height'] = args.actualheight
    if '-t' in cmd:
        _dict['temperature'] = args.temperature
    if '-w' in cmd:
        _dict['windows'] = args.winsize
    if '-e' in cmd:
        _dict['threshold'] = args.threshold
    if '-a' in cmd:
        _dict['averaging'] = args.averaging
    if '-od' in cmd:
        _dict['output_detail'] = args.od
    if '-ow' in cmd:
        _dict['output_wave'] = args.ow
    if '-og' in cmd:
        _dict['output_graph'] = args.og
    if '-sg' in cmd:
        _dict['show_graph'] = args.sg
    if '-sa' in cmd:
        _dict['secondary_axis'] = args.sa
    if '-dm' in cmd:
        _dict['distance_marker'] = args.dm
    if '-cp' in cmd:
        _dict['combine_plot'] = args.cp
    if '-cw' in cmd:
        _dict['combine_wave'] = args.cw
    if '-ep' in cmd:
        _dict['endpulse'] = args.endpulse
    if '-out' in cmd:
        _dict['outfilename'] = args.outfilename
    if '-vid' in cmd:
        _dict['validid'] = args.validid
    return _dict


if __name__ == "__main__":
    args = read_parser()

    Config = ConfigParser.ConfigParser()
    Config.read(os.path.join(os.getcwd(), args.config))

    _dict = {}
    for section in Config.sections():
        if 'Section' in section:
            _dict = configSectionMap(Config, section, _dict)

    _dict = update_value(sys.argv[1:], args, _dict)
    _pathname = os.path.abspath(args.inputdir)

    if os.path.isfile(_pathname):
        _root = os.path.dirname(_pathname)
        _jsonf = os.path.basename(_pathname)
        _json = JSONProcessor(Config, _dict, _root)
        if _json is None:
            print "Error: Failed to initialize JSON Class!"
            exit(1)
        pdata = _json.load_file(_jsonf)
        with open(args.outfilename, 'w') as outf:
            for d in pdata:
                outf.write(json.dumps(d) + '\n')
    else:
        _json = JSONProcessor(Config, _dict, _pathname)
        if _json is None:
            print "Error: Failed to initialize JSON Class!"
            exit(1)
        _fnames = []
        with open(args.outfilename, 'w') as outf:
            for root, dirs, files in os.walk(_pathname):
                _json.load_validid(path=root)
                for jsonf in files:
                    if jsonf.endswith(".json"):
                        pdata = _json.load_file(jsonf)
                        for d in pdata:
                            outf.write(json.dumps(d) + '\n')
                        _fnames.append(jsonf)
            outf.write(json.dumps(_json.get_metadata({'raw-json-files': _fnames})) + '\n')
