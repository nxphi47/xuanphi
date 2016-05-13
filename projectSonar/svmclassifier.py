import sys, os
import numpy
import copy

try:
    from svmutil import *
except:
    from libsvm.svmutil import *
from StringIO import StringIO
import logging


def vect_str(v):
    if len(v) < 5:
        return repr(v)
    else:
        return "[ %f, %f, ..., %f, %f ](%d)" % (v[0], v[1], v[-2], v[-1], len(v))


#
# Support Vector Machine Class to wrap around libsvm
# 
class SVMClassifier(object):
    # opts is ignored.
    # fname is to specify a previously saved SVM to load
    # scaling is to set if scaling should be performed
    def __init__(self, opts='', fname='', scaling=True):
        self.model = None
        self.scaling = scaling
        self.scaling_mean = []
        self.scaling_std = []
        self.scaling_opt = []
        self.opts = opts.split(' ')
        self.meta = []
        if fname != '':
            self.load(fname)
        pass

    # Loads a previously saved SVM
    def load(self, fname):
        logging.debug("SVMClassifier: loading %s ..." % fname)
        self.model = svm_load_model(fname)
        tmp = numpy.load(fname + ".npy").tolist()
        self.scaling_mean = tmp[0]
        self.scaling_std = tmp[1]
        if len(tmp) > 2:
            self.scaling_opt = tmp[2]
        else:
            self.scaling_opt = [1.] * len(tmp[0])
        if os.path.isfile(fname + '.meta'):
            import json
            with open(fname + '.meta', 'r') as f:
                self.meta = json.load(f)
        self.model_labels = self.model.get_labels()

    # Saves a presumably trained SVM to specified file
    # One more file called <fname>.npy will be created containing the sclaing info used
    # If <META> is specified, an additional file with name <fname>.meta will be saved containing a json dump of <META>
    def save(self, fname, META=None):
        svm_save_model(fname, self.model)
        tmp = numpy.array([self.scaling_mean, self.scaling_std, self.scaling_opt])
        numpy.save(fname + ".npy", tmp)
        if META:
            import json
            self.meta.extend(META)
            if self.cv_res:
                self.meta[-1]['CV-Result'] = self.cv_res
            with open(fname + '.meta', 'w') as f:
                json.dump(self.meta, f)
        logging.debug("SVMClassifier: saved to %s ..." % fname)

    # Because libsvm no longer accepts numpy arrays, we need to convert numpy into list
    def __convert_data_label(self, label, data):
        # check if the inputs are ndarrays
        if type(data) == type(numpy.array(0)):
            _data = data.tolist()
        else:
            _data = copy.deepcopy(data)
        if len(label) > 1:
            if type(label) == type(numpy.array(0)):
                _label = label.tolist()
            else:
                _label = copy.deepcopy(label)
        else:
            _label = [0] * len(_data)
        return _label, _data

    # Set up the SVM problem.
    def __get_prob(self, label, data):
        # convert from numpy
        _label, _data = self.__convert_data_label(label, data)

        # get the scaling mean and standard deviation
        _s = [0.] * len(_data[0])
        _s2 = [0.] * len(_data[0])
        for i in xrange(len(_data)):
            for j in xrange(len(_data[i])):
                _s[j] += _data[i][j]
                _s2[j] += _data[i][j] * _data[i][j]
        for j in xrange(len(_s)):
            _s[j] /= len(_data)
            _s2[j] = _s2[j] / len(_data) - _s[j] * _s[j]
            if _s2[j] < 0.0001:
                _s2[j] = 1.0
            else:
                _s2[j] = 1.0 / (_s2[j] ** 0.5)
        self.scaling_mean = _s
        self.scaling_std = _s2
        if len(self.scaling_opt) != len(self.scaling_std):
            self.scaling_opt = [1.] * len(self.scaling_std)

        # scale the input data
        if self.scaling:
            for i in xrange(len(_data)):
                _data[i] = self.__scale_vect(_data[i])

        self.prob = svm_problem(_label, _data)
        return self.prob

    # Scale input vector prior to training/prediction
    def __scale_vect(self, v):
        for i in xrange(len(v)):
            if self.scaling_opt[i] > 0:
                v[i] -= self.scaling_mean[i]
                v[i] *= self.scaling_std[i]
        return v

    # Train an SVM.  Almost not used, since we do cross validation
    def train(self, label, data, params=None, quiet=False):
        # get svm parameters
        if params != None:
            param = svm_parameter(params + ' -b 1')
        else:
            param = svm_parameter('-b 1')
            param.kernel_type = RBF
        # start training
        self.__suppress_stdout(quiet)
        prob = svm_problem(label, data)
        self.model = svm_train(prob, param)
        self.model_labels = self.model.get_labels()
        self.__restore_stdout(quiet)

    # Cross Validation training
    # Keep training the SVM with the specified C range and G range
    def cv_train(self, label, data, cs={'start': -20., 'end': 20., 'step': 4.},
                 gs={'start': -20., 'end': 20., 'step': 4.}, levels=3):
        self.__get_prob(label, data)
        self.cv_accuracy = []
        best_cg = None
        for i in xrange(levels):
            best_cg = self.__cv_train(cs, gs, best_cg)
            cs = self.__cv_set_next_spec(cs, best_cg['c'])
            gs = self.__cv_set_next_spec(gs, best_cg['g'])
        # self.__cv_sort_accuracy()

        # Retrain using the best c,g, now withoit the '-k' cross validation and
        # with '-b 1' to build probability model
        self.cv_final_train(label, data, best_cg['c'], best_cg['g'])
        self.cv_res = best_cg

    # Train with Final c,g values
    def cv_final_train(self, label, data, c, g):
        self.__get_prob(label, data)
        param = svm_parameter('-q -c %f -g %f -b 1' % (2. ** c, 2. ** g))
        self.model = svm_train(self.prob, param)
        self.model_labels = self.model.get_labels()

    # Actual iteration of cross validation training
    def __cv_train(self, cs, gs, best_cg=None):
        clist = self.__cv_gen_list(cs)
        glist = self.__cv_gen_list(gs)
        for c in clist:
            cval = 2. ** c
            for g in glist:
                gval = 2. ** g
                param = svm_parameter('-q -v 5 -c %f -g %f' % (cval, gval))
                print "cross-validating with c=2^%f=%f, g=2^%f=%f " % (c, cval, g, gval),
                try:
                    v = svm_train(self.prob, param)
                except:
                    print "\n\n\nERROR in value key\n\n\n"
                    v = 0.
                self.cv_accuracy.append({'c': c, 'g': g, 'v': v})
                if best_cg == None:
                    best_cg = {'c': c, 'g': g, 'v': v}
                    print " -> yeilds v=%f ==> set as best" % v
                elif v > best_cg['v']:
                    best_cg = {'c': c, 'g': g, 'v': v}
                    print " -> yeilds v=%f ==> set as best" % v
                else:
                    print " -> yeilds v=%.2f ==> discard" % v
        print "Best c=2^%f, g=2^%f with v=%.2f" % (best_cg['c'], best_cg['g'], best_cg['v'])
        return best_cg

    # generates the list of C/G values given the spec { 'start', 'end', 'step' }
    def __cv_gen_list(self, spec):
        r = [spec['start']]
        while r[-1] < spec['end']:
            r.append(r[-1] + spec['step'])
        return r

    # Generates the next C/G spec given the best C/G values now
    def __cv_set_next_spec(self, prev, best):
        ret = {'start': best - 1.25 * prev['step'], 'end': best + 1.25 * prev['step'], 'step': 0.5 * prev['step']}
        if abs(best - prev['start']) < prev['step']:
            # close to start: we give more leeway to start
            ret['start'] -= 0.5 * prev['step']
        if abs(best - prev['end']) > prev['step']:
            # close to end: we give more leeway to end
            ret['end'] += 0.5 * prev['step']
        return ret

    def __cv_sort_accuracy(self):
        res = [{'v': -100.}]
        for a in self.cv_accuracy:
            for i in xrange(len(res)):
                if a['v'] > res[i]['v']:
                    res.insert(i, a)
                    break
        del res[-1]
        self.cv_accuracy = res

    # Perform prediction
    # <data> must be list of vectors
    # <label> can be omitted
    def predict(self, data, quiet=True, labels=[]):
        # convert from numpy
        _label, _data = self.__convert_data_label(labels, data)
        # scale the data
        if self.scaling:
            for i in xrange(len(_data)):
                _data[i] = self.__scale_vect(_data[i])

        self.__suppress_stdout(quiet)
        self.pred_labels, self.pred_acc, self.pred_vals = svm_predict(_label, _data, self.model, '-b 1')
        # self.pred_vals contains the probability vector.  But it is in SVM own order of class labels
        # Call reorder_label() to reorder it according to the training labels given in cv_train()
        self.__restore_stdout(quiet)
        return self.pred_labels

    # Perform prediction with probability
    # <data> must be list of vectors
    # <label> can be omitted
    def predict_p(self, data, labels=[], quiet=True):
        # convert from numpy
        _label, _data = self.__convert_data_label(labels, data)
        # scale the data
        if self.scaling:
            for i in xrange(len(_data)):
                _data[i] = self.__scale_vect(_data[i])
        self.__suppress_stdout(quiet)
        self.pred_labels, self.pred_acc, self.pred_vals = svm_predict(_label, _data, self.model, '-b 1')
        self.__restore_stdout(quiet)
        return self.pred_labels, self.reorder_label(copy.deepcopy(self.pred_vals))

    def __suppress_stdout(self, quiet=True):
        # suppress output
        if quiet or ('-q' in self.opts):
            self.org_stdout = sys.stdout
            sys.stdout = StringIO()

    def __restore_stdout(self, quiet=True):
        # restore output
        if quiet or ('-q' in self.opts):
            sys.stdout = self.org_stdout

    def __argmax(self, vect):
        m = 0
        for i in len(vect):
            if vect[i] > vect[m]:
                m = i
        return m

    # Convert label vectors	from [ 1, 0, 0, 0 ] to [ 0 ]
    def convert_label(self, label):
        if type(label) == type(numpy.array(0)):
            return [numpy.argmax(x) for x in label]
        else:
            return [self.__argmax(x) for x in label]

    # Reorder the label according to training labels
    def reorder_label(self, labels=None):
        if labels == None:
            labels = self.pred_vals
        for i in xrange(len(labels)):
            v = [0.] * len(labels[i])
            for j in xrange(len(self.model_labels)):
                v[int(self.model_labels[j])] = labels[i][j]
            labels[i] = v
        return labels

    # Debug
    def debug(self):
        logging.debug("-- SVM:: model = %s, scaling = %s" % (repr(self.model), repr(self.scaling)))
        logging.debug("-- SVM:: scaling_mean = %s" % vect_str(self.scaling_mean))
        logging.debug("-- SVM:: scaling_std = %s" % vect_str(self.scaling_std))
        logging.debug("-- SVM:: scaling_opt = %s" % vect_str(self.scaling_opt))
        logging.debug("-- SVM:: model_labels = %s" % vect_str(self.model_labels))


#
# Automatically scale the SVM
#
class ScaledSVMClassifier(SVMClassifier):
    def __init__(self, opts='', fname=''):
        SVMClassifier.__init__(self, opts, fname, True)

    def scaled_cv_train(self, label, data, cs={'start': -20., 'end': 20., 'step': 4.},
                        gs={'start': -20., 'end': 20., 'step': 4.}, levels=3):
        # prepapre scaled training buffer
        _size = len(data[0])
        self.scaled_train = []
        for i in xrange(2 ** _size):
            a = {'opt': [1] * _size, 'bestcg': []}
            for j in xrange(_size):
                if ((2 ** j) & i) > 0:
                    a['opt'][j] = 1
                else:
                    a['opt'][j] = -1
            self.scaled_train.append(a)
        # start training
        best = 0
        for st in self.scaled_train:
            print "-------------------------------------------------------------"
            print "Training with scaling = %s" % repr(st['opt'])
            print "-------------------------------------------------------------"
            self.scaling_opt = st['opt']
            self.cv_train(label, data, cs, gs, levels)
            st['bestcg'] = self.cv_res
            if st['bestcg']['v'] > self.scaled_train[best]['bestcg']['v']:
                best = self.scaled_train.index(st)
        # completed training
        print "-------------------------------------------------------------"
        print "Training with best scaling = %s" % repr(self.scaled_train[best]['opt'])
        print "-------------------------------------------------------------"
        self.scaling_opt = self.scaled_train[best]['opt']
        self.cv_res = self.scaled_train[best]['bestcg']
        self.cv_final_train(label, data, self.cv_res['c'], self.cv_res['g'])

    def print_res(self):
        for st in self.scaled_train:
            print "scaling=%s, c=2^%f, g=2^%f, v=%.2f" % (
            repr(st['opt']), st['bestcg']['c'], st['bestcg']['g'], st['bestcg']['v'])


if __name__ == "__main__":
    def __test_criteria(x, y):
        if (x ** 2 + y ** 2) < 16.:
            return 0
        if (x ** 2 + y ** 2) < 36.:
            return 1
        if (x ** 2 + y ** 2) < 64.:
            return 2
        return 3


    train_data = numpy.random.randint(-10, 10, 1000).reshape(500, 2)
    train_label = [__test_criteria(x, y) for (x, y) in train_data]
    svm = SVMClassifier('-q')
    svm.cv_train(train_label, train_data)
    for a in svm.cv_accuracy:
        if a['v'] > 60.0:
            print "c=2^%f, g=2^%f, v=%.1f" % (a['c'], a['g'], a['v'])
    svm.save("test-svm.svm")
    del svm

    svm = SVMClassifier(opts='-q', fname="test-svm.svm")
    test_data = numpy.random.randint(-10, 10, 100).reshape(50, 2)
    test_label = [__test_criteria(x, y) for (x, y) in test_data]
    svm.predict(test_data, labels=test_label)

    for i in xrange(len(test_data)):
        print "point(%.1f,%.1f):  Actual=%d  Predicted=%d" % (
        test_data[i][0], test_data[i][1], int(test_label[i]), int(svm.pred_labels[i]))
