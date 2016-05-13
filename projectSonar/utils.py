#!/bin/env python2

import json, sys

# the feature vector to extract
# FEATURES=['pulseWidth', 'peakValue', 'totalArea', 'peakTime']
FEATURES = ['pulseWidth', 'peakValue', 'totalArea', 'peakTime', '+gradRatio', '-gradRatio', '0gradRatio']

# training class label
#	Normal	- normal label for C>15, C<15, WALL, POLE
#	Surface	- label for smooth and rough surface
#	Move	- label for move and steady
TRAIN_CLASS = "Normal"


# Extract the feature vector according to the order given in <features>
def get_feature_vector(obj, features=FEATURES):
    ret = []
    data = obj['Working']
    for f in features:
        if data.has_key(f):
            ret.append(data[f])
        else:
            print '%s feature not found!!!' % f
            exit(1)
            ret.append(0.0)
    return ret


# Load a sample from the processed json file <inf>
# <features> is specified to have a feature vector in the returned dict
# <META> is set to true if the caller wants the metadata of the processed json file
def get_sample(inf, features, META=False, NUS=False):
    s, o = get_sample_with_obj(inf, features, META, NUS)
    return s


# Load a sample from the processed json file <inf>
# <features> is specified to have a feature vector in the returned dict
# <META> is set to true if the caller wants the metadata of the processed json file
# This function will also returns the loaded JSON object as a second return value
def get_sample_with_obj(inf, features, META=False, NUS=False):
    ins = inf.readline()
    if ins == '':
        return None, None
    try:
        obj = json.loads(ins)
    except:
        # ignore those that is not valid json
        print 'invalid json'
        return {'invalid': None}, None
    if obj.has_key('META'):
        if not META:
            print 'Metadata ignored'
            return {'META': None}, None
        else:
            return {'META': obj['META']}, obj
    if not obj.has_key('filename'):
        # ignore those that does not contain filename
        print "missing key: 'filename'"
        return {'invalid': None}, None
    if not obj.has_key('DataID'):
        # ignore those that does not contain filename
        print "missing key: 'DataID'"
        return {'invalid': None}, None
    label = guess_label_from_filename(obj['filename'], CLASS=TRAIN_CLASS)
    if label == '':
        # ignore those that cannot be labelled
        print "cannot estimate label from filename: '%s'" % obj['filename']
        return {'invalid': None}, None
    if not NUS:
        return {'fv': get_feature_vector(obj, features), 'label': label, 'id': obj['DataID'],
                'filename': obj['filename']}, obj
    else:
        _dist, _height, _hazard, _surface, _movement = guess_actual_from_filename(obj['filename'])
        if not 'refEnvList' in features:
            features.append('refEnvList')
        if not 'refTimeList' in features:
            features.append('refTimeList')
        return {'fv': get_feature_vector(obj, features), 'id': obj['DataID'], 'actual_distance': _dist,
                'actual_height': _height, 'hazard': _hazard,
                'surface': _surface, 'movement': _movement, 'label': label, 'filename': obj['filename']}, obj


def get_sample_by_id(inf, dataID):
    for ins in inf.readlines():
        if ins == '':
            return None, None
        try:
            obj = json.loads(ins)
        except:
            # ignore those that is not valid json
            print 'invalid json'
            return {'invalid': None}, None
        if obj.has_key('id'):
            if obj['id'] == dataID:
                inf.seek(0)
                return obj


def get_sample_by_hazard(inf, hazard):
    _obj = []
    for ins in inf.readlines():
        if ins == '':
            return None, None
        try:
            obj = json.loads(ins)
        except:
            # ignore those that is not valid json
            print 'invalid json'
            return {'invalid': None}, None
        if obj.has_key('filename'):
            if hazard in obj['filename']:
                _obj.append(obj)
    inf.seek(0)
    return _obj


def _load_all_training_objs(fname):
    meta = []
    objs = []
    with open(fname, 'r') as f:
        while True:
            s = f.readline()
            if s == '':
                break
            try:
                obj = json.loads(s)
            except:
                print "Invalid json format in %s: '%s'" % (fname, s)
                continue
            if obj.has_key('META'):
                meta = []
                meta.append(obj['META'])
                continue
            if (not obj.has_key('fv')) or (not obj.has_key('label')):
                print "No 'fv'/'label' in %s: '%s'" % (fname, s)
                continue
            objs.append(obj)
    return objs, meta


def load_smooth_training_samples(fname, all_labels=[], META=False):
    objs, meta = _load_all_training_objs(fname + '.seq')
    ids = []
    samples = []
    labels = []
    for obj in objs:
        ids.append(obj['id'])
        _l = []
        for i in obj['fv']:
            _l.extend(i)
        samples.append(_l)
        if obj['label'] in all_labels:
            # label already appear before
            labels.append(all_labels.index(obj['label']))
        else:
            # first-time label
            labels.append(len(all_labels))
            all_labels.append(obj['label'])
        # with open(_file, 'r') as t:
        # while True:
        # s = t.readline()
        # if s == '':
        # break
        # try:
        # obj = json.loads(s)
        # except:
        # print "Invalid json format in %s: '%s'" % (fname, s)
        # continue
        # if obj.has_key('META'):
        # meta = []
        # meta.append(obj['META'])
        # continue
        # if (not obj.has_key('fv')) or (not obj.has_key('label')):
        # print "No 'fv'/'label' in %s: '%s'" % (fname, s)
        # continue

        # ids.append(obj['id'])
        # _l = []
        # for i in obj['fv']:
        # _l.extend(i)
        # samples.append(_l)
        # if obj['label'] in all_labels:
        ## label already appear before
        # labels.append(all_labels.index(obj['label']))
        # else:
        ## first-time label
        # labels.append(len(all_labels))
        # all_labels.append(obj['label'])
    if META:
        return samples, labels, ids, all_labels, meta
    else:
        return samples, labels, ids, all_labels


# Load a training sample from the samples file
# <fname> is filename of the input sample file
# <all_labels> is the initial list of all known labels of the samples.  It can be empty []
#    and will be returned by the function as a 4th return value
# <META> is set to true if the caller wants the metadata to be returned as a 5th return value
# <SEQID> is set to true if the caller wants the sequence of IDs to be returned as a 6th return value
# Furnction returns at least 4 values:
#	<samples>, <labels>, <ids>, <all_labels> [, <meta>, <seqobjs>]
# <samples>: list of feature vector
# <labels>: list of labels index corresponding to the feature vector in <samples>
# <ids>: IDs of samples corresponding to the feature vector in <samples>
# <all_labels>: list of the class labels, whose index is used in <labels>
# <meta>: if <META> is True, return list of metadata encountered in the input file
# <seqobjs>: if <SEQOBJ> is True, return list of seq objects corresponding to <samples>
def load_training_samples(fname, all_labels=[], META=False, SMOOTH=False, SEQOBJ=False):
    if SMOOTH:
        return load_smooth_training_samples(fname, all_labels, META)
    ids = []
    samples = []
    labels = []
    objs, meta = _load_all_training_objs(fname)
    for obj in objs:
        samples.append(obj['fv'])
        ids.append(obj['id'])
        if obj['label'] in all_labels:
            # label already appear before
            labels.append(all_labels.index(obj['label']))
        else:
            # first-time label
            labels.append(len(all_labels))
            all_labels.append(obj['label'])
    if SEQOBJ:
        seqobjs, seqmeta = _load_all_training_objs(fname + '.seq')
        retseqobjs = []
        for i in ids:
            t = None
            for sobj in seqobjs:
                if sobj['id'] == i:
                    t = sobj
                    break
            retseqobjs.append(t)
        if META:
            return samples, labels, ids, all_labels, meta, retseqobjs
        else:
            return samples, labels, ids, all_labels, retseqobjs
    else:
        if META:
            return samples, labels, ids, all_labels, meta
        else:
            return samples, labels, ids, all_labels
        # with open(fname, 'r') as f:
        # while True:
        # s = f.readline()
        # if s == '':
        # break
        # try:
        # obj = json.loads(s)
        # except:
        # print "Invalid json format in %s: '%s'" % (fname, s)
        # continue
        # if obj.has_key('META'):
        # meta.append(obj['META'])
        # continue
        # if (not obj.has_key('fv')) or (not obj.has_key('label')):
        # print "No 'fv'/'label' in %s: '%s'" % (fname, s)
        # continue
        # samples.append(obj['fv'])
        # ids.append(obj['id'])
        # if obj['label'] in all_labels:
        ## label already appear before
        # labels.append(all_labels.index(obj['label']))
        # else:
        ## first-time label
        # labels.append(len(all_labels))
        # all_labels.append(obj['label'])


# Guess the label based on the filename of the raw json file 
def guess_label_from_filename(filename, CLASS="Normal"):
    if CLASS == "Normal":
        if 'wall' in filename:
            return 'WALL'
        if ('pole' in filename) or ('roll' in filename):
            return 'POLE'
        if 'curb' in filename:
            name_label = filename.split('-')
            for x in name_label:
                if 'curb' in x:
                    curbHeight = float(x.replace('curb', ''))
                    if curbHeight >= 15:
                        return 'C>15'
                    else:
                        return 'C<15'
        return ''
    elif CLASS == "Surface":
        if 'rough' in filename:
            return 'ROUGH'
        else:
            return 'SMOOTH'


def guess_actual_from_filename(filename):
    _dist = 0.
    _height = 0.
    _hazard = 0  # 0 - curb, 1 - pole, 2 - wall
    _surface = 0  # 0 - smooth, 1 - rough
    _movement = 0  # 0 - false, 1 - move

    _split = filename.split('-')
    if 'm' in _split[2]:
        _dist = float(_split[2][:-1])

    if 'json' in _split[3]:
        _split[3] = _split[3][:-5]
    if 'curb' in _split[3]:
        _hazard = 0
        _height = float(_split[3][4:])
    elif 'pole' in _split[3]:
        _hazard = 1
        _height = float(_split[3][4:])
    elif 'wall' in _split[3]:
        _hazard = 2
        _height = float(_split[3][4:])

    if 'rough' in filename:
        _surface = 1
    if 'move' in filename:
        _movement = 1

    return _dist, _height, _hazard, _surface, _movement


# Class for Accuracy Calculation
class AccStats(object):
    def __init__(self, labels):
        self.labels = labels
        self.reset()
        _width = 0
        for i in xrange(len(labels)):
            if _width < len(labels[i]):
                _width = len(labels[i])
        if _width < 9:
            _width = 9
        self._width = _width + 1

    def reset(self):
        self.matrix = []
        for i in xrange(len(self.labels)):
            self.matrix.append([0] * len(self.labels))
        self.corr = [0] * len(self.labels)
        self.wrong = [0] * len(self.labels)
        self.total = [0] * len(self.labels)
        self.updated = False

    def update(self, actual, pred):
        if isinstance(actual, list):
            for i in xrange(len(actual)):
                self.matrix[actual[i]][int(pred[i])] += 1
        else:
            self.matrix[actual][int(pred)] += 1
        self.updated = False

    def get_res(self):
        if not self.updated:
            for i in xrange(len(self.labels)):
                for j in xrange(len(self.labels)):
                    if j == i:
                        self.corr[i] += self.matrix[i][j]
                    else:
                        self.wrong[i] += self.matrix[i][j]
                self.total[i] = self.corr[i] + self.wrong[i]
            self.updated = True
        tc = sum(self.corr)
        tw = sum(self.wrong)
        return tc, tw

    def print_table(self, lead='', f=sys.stdout):
        if f is None:
            f = sys.stdout
        if not self.updated:
            self.get_res()
        _str = '%-' + str(self._width) + 's '
        _dec = '%-' + str(self._width) + 'd '
        f.write('\n%s\n' % lead)
        f.write(_str % '')
        for i in xrange(len(self.labels)):
            f.write(_str % self.labels[i])
        f.write("| %-12s %-12s\n" % ("  Correct  ", "    Wrong   "))
        for i in xrange(len(self.labels)):
            f.write(_str % self.labels[i])
            for j in xrange(len(self.labels)):
                f.write(_dec % self.matrix[i][j])
            f.write("| %4d %5.1f%%  " % (self.corr[i], 100. * self.corr[i] / self.total[i]))
            f.write("%4d %5.1f%%\n" % (self.wrong[i], 100. * self.wrong[i] / self.total[i]))
        tc = sum(self.corr)
        tw = sum(self.wrong)
        tn = sum(self.total)
        f.write(
            "Total %s: Correct = %d  %.1f%%,  Wrong = %d  %.1f%%\n\n" % (lead, tc, 100. * tc / tn, tw, 100. * tw / tn))
