# for basic tools
import os, sys, time, copy
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.pylab as pl
import argparse, ConfigParser

# for SVM
from sklearn.svm import SVC
from sklearn.cross_validation import train_test_split, StratifiedShuffleSplit
from sklearn.grid_search import GridSearchCV, RandomizedSearchCV
from sklearn.metrics import classification_report
from sklearn import datasets
from sklearn.preprocessing import StandardScaler

# from sonar sensing project
import svmclassifier
import utils


############
# SVM classifier that use rbf and grid search strategy to find best C and gamma

class SVMclassifer():
    ###### procedure
    # init or get method, get_problem, set_cross-valid, train, predict
    def __init__(self, cv='split', search='grid'):
        self.cv = cv
        self.search = search
        self.set_split = False
        self.Trained = False

    # copy the training data to avoid modify the origin
    def get_problem(self, X, y):
        self.X = copy.deepcopy(X)
        self.y = copy.deepcopy(y)

    # restrict the sameples and feature and the labels
    def restrict_problem(self, samples_shape, labels):
        self.X = self.X.reshape(samples_shape)
        self.y = self.y[labels]

    def set_method(self, cv='split', search='grid'):
        self.cv = cv
        self.search = search

    def setup_cross_valid(self, X=None, y=None, n_iter=10, test_size=0.4, random_state=0):
        if y is None:
            y = self.y
        if X is None:
            X = self.X

        if self.cv == 'split':
            # train test split method
            if X is None:
                print "No samples is use"
                return
            self.X_train, self.X_test, self.y_train, self.y_test = train_test_split(X, y, test_size=test_size,
                                                                                    random_state=random_state)
        else:
            # Strategied Shuffle split
            self.shuffle_split = StratifiedShuffleSplit(y, n_iter=n_iter, test_size=test_size,
                                                        random_state=random_state)

        self.set_split = True

    def train(self, C_set=None, gamma_set=None, kernel_set=None):
        # setting the parameter set
        if kernel_set is None:
            # default kernel
            kernel_set = ['rbf']
        if gamma_set is None:
            # default gamma set
            gamma_set = [0.01, 0.1, 1, 10, 100, 1000]
        if C_set is None:
            # default C set
            C_set = [0.01, 0.1, 1, 10, 100, 1000]

        start_time = time.time()
        param_set = {'C': C_set, 'gamma': gamma_set, 'kernel': kernel_set}

        # set cv
        if self.cv == 'split':
            cv = 5
        else:
            cv = self.shuffle_split
        # set search parameter
        self.rand_search_iter = 40
        # training with grid search
        if self.search == 'grid':
            self.model = GridSearchCV(SVC(cache_size=1000), param_grid=param_set, cv=cv)
        else:
            self.model = RandomizedSearchCV(SVC(), n_iter=self.rand_search_iter, param_distributions=param_set)

        # training
        if self.set_split == False:
            print "Need to setup the split method first"
            return
        if self.cv == 'split':
            self.model.fit(self.X_train, self.y_train)
        else:
            self.model.fit(self.X, self.y)

        # set the training
        self.Trained = True
        return time.time() - start_time

    def get_best_param_score(self):
        if self.Trained == False:
            print "You need to train SVM first"
            return {}, None
        else:
            return self.model.best_params_, self.model.best_score_

    def get_grid_scores(self):
        if self.Trained == False:
            print "You need to train first"
            return []
        else:
            return self.model.grid_scores_

    def predict(self, X, y=None):
        self.y_pred = self.model.predict(X)
        self.y_test_afterPred = y
        return self.y_pred


# reading input parameter
def read_parser():
    # argument settting for cv, iter, search, Crange, gamma range
    parser = argparse.ArgumentParser()
    parser.add_argument('-cv', help="cross-validation method", default='split')
    parser.add_argument('-i', help="iteration", default='5', type=int)
    parser.add_argument('-s', help="search method", default='grid', choices=['grid', 'random'])
    parser.add_argument('-C', '-Crange', help="C parameter range = start, end, num", default='[-1,1,3')
    parser.add_argument('-g', '-gammaRange', help="gamma range", default='[-1,1,3')
    parser.add_argument('-k', '-kernel', help="kernel setting", default='rbf', choices=['rbf', 'linear'])
    parser.add_argument('-t', '-test_size', help="test_size", default='0.5', type=float)
    parser.add_argument('-d', '-data', help="data set", default='digits', choices=['digits'])
    parser.add_argument('-fig', '-config', help="the config file", default='phi_config.ini')
    parser.add_argument('-kian', help="using kian soon svm", action='store_true', default=False)
    args = parser.parse_args()
    return args


# updating the settings with argument input
def update_settings(command, args, settings):
    Crange = args.C.replace('[', '').split(',')
    for i, x in enumerate(Crange):
        Crange[i] = int(x)
    gammaRange = args.g.replace('[', '').split(',')
    for i, x in enumerate(gammaRange):
        gammaRange[i] = int(x)

    if '-cv' in command: settings['cross_valid'] = args.cv
    if '-i' in command: settings['iteration'] = args.i
    if '-s' in command: settings['search_method'] = args.s
    if '-k' in command: settings['kernel'] = [args.k]
    if '-t' in command: settings['test_size'] = args.t
    if '-C' in command:
        settings['c_range'] = np.logspace(Crange[0], Crange[1], Crange[2])
    if '-g' in command:
        settings['gamma_range'] = np.logspace(gammaRange[0], gammaRange[1], gammaRange[2])
    if '-d' in command: settings['data'] = args.d
    return settings


# read config file
def read_config(settings, filename):
    config = ConfigParser.ConfigParser()
    config.read(os.path.join(os.getcwd() + '/' + filename))
    for section in config.sections():
        for option in config.options(section):
            # this include the string type of numbers
            settings[option] = config.get(section, option)
    # iteration
    settings['iteration'] = int(settings['iteration'])

    # C set
    Crange = settings['c_range'].replace('[', '').split(',')
    for i, x in enumerate(Crange):
        Crange[i] = int(x)
    settings['c_range'] = np.logspace(Crange[0], Crange[1], Crange[2])

    # gamme set
    gammaRange = settings['gamma_range'].replace('[', '').split(',')
    for i, x in enumerate(gammaRange):
        gammaRange[i] = int(x)
    settings['gamma_range'] = np.logspace(gammaRange[0], gammaRange[1], gammaRange[2])

    # test_size
    settings['test_size'] = float(settings['test_size'])

    # predict number
    settings['predict'] = int(settings['predict'])
    settings['predict_random'] = int(settings['predict_random'])

    # kernel
    settings['kernel'] = [settings['kernel']]
    # output graph:
    settings['output_graph'] = bool(settings['output_graph'])
    return settings


test_digits = np.array([
    [0, 0, 0, 10, 10, 0, 0, 0],
    [0, 0, 10, 10, 10, 0, 0, 0],
    [0, 10, 0, 10, 10, 0, 0, 0],
    [0, 0, 0, 10, 10, 0, 0, 0],
    [0, 0, 0, 10, 10, 0, 0, 0],
    [0, 0, 0, 10, 10, 0, 0, 0],
    [0, 0, 0, 10, 10, 0, 0, 0],
    [0, 0, 0, 10, 10, 0, 0, 0]
])

# main function
if __name__ == '__main__':
    # argument settting for cv, iter, search, Crange, gamma range
    settings = {}
    args = read_parser()
    read_config(settings, args.fig)
    update_settings(sys.argv[1:], args, settings)

    # print settings
    print "-----printing Settings --------------------"
    for x in settings:
        if x == 'c_range' or x == 'gamma_range':
            print "%s :" % (x),
            for val in settings[x]:
                print "%.1g " % val,
        else:
            print x + ' : ' + str(settings[x])
    print "-------------------------------------------"
    # create datasets based on arguments
    digits = datasets.load_digits()
    iris = datasets.load_iris()
    samples_set = []
    labels_set = []
    if settings['data'] == 'digits':
        digits = datasets.load_digits()
        samples_set = digits.images.reshape((len(digits.images), -1))
        labels_set = digits.target

    # for kian soon test, take only the first 4 first feature
    samples_set = samples_set[:, 30:45]
    # scaling the datasets
    # scaler = StandardScaler()
    # sameples_set = scaler.fit_transform(sameples_set)
    # print sameples_set[1]
    # print "datasets scaled"
    test_samples = samples_set[30:45]
    test_labels = labels_set[30:45]

    ##### svm_train part, getting the real training samples
    samples_file = '/home/nxphi/Desktop/phiphi/testsklean/training.samples.20160518'
    samples_set, labels_set, ids, all_labels, meta = utils.load_training_samples(samples_file, META=True, SMOOTH=True)
    samples_set, samples_test, labels_set, labels_test = train_test_split(samples_set, labels_set, test_size=settings['test_size'])
    # kian soon parameter set, using 2 exponent
    cs = {'start': -20., 'end': 20., 'step': 4.}
    gs = {'start': -20., 'end': 20., 'step': 4.}

    # change C and g set to be the same as KS
    C_range = []
    gamma_range = []
    for i in range(-20, 20, 4):
        C_range.append(2. ** i)
        gamma_range.append(2. ** i)
    settings['c_range'] = C_range
    settings['gamma_range'] = gamma_range
    # SVM train
    # kian soon machine
    if args.kian:
        print "Kien Soon algo selected"
        model = svmclassifier.SVMClassifier('-q')
        startTime = time.time()
        best_para = model.cv_train(labels_set, samples_set)
        duration = time.time() - startTime
        print "Kian soon test timing: ", duration
        print "best param: ", 2 ** best_para['c'], "and g : ", 2 ** best_para['g']
        prediction = model.predict(samples_test)

        print classification_report(labels_test, prediction)
        # search for parameter set
        samples_set = np.asarray(samples_set)
        labels_set = np.asarray(labels_set)
        print "shapes: ", samples_set.shape
        parameter_set = [{'C': C_range, 'gamma': gamma_range}]

        myMachine = GridSearchCV(SVC(kernel='rbf'), param_grid=parameter_set, cv=2)
        startTime = time.time()
        myMachine.fit(samples_set, labels_set)
        durationSearch = time.time() - startTime
        print "sklearn method: ", myMachine.best_params_
        print "timing: ", durationSearch
        pred = myMachine.predict(samples_test)
        print classification_report(labels_test, pred)
        exit(1)

    # my machine
    machine = SVMclassifer(cv=settings['cross_valid'], search=settings['search_method'])
    machine.get_problem(samples_set, labels_set)
    machine.setup_cross_valid(n_iter=settings['iteration'], test_size=settings['test_size'])
    timing = machine.train(C_set=settings['c_range'], gamma_set=settings['gamma_range'],
                           kernel_set=settings['kernel'])

    best_pa, best_score = machine.get_best_param_score()
    print "best param and scores: %s --- %.3f%%" % (best_pa, best_score * 100)
    print "training timing = %.4f second" % (timing)

    # prediction
    y_pred = machine.predict(samples_set)

    print classification_report(labels_set, y_pred)
    exit(1)
    """
    pred_size = settings['predict_random']
    max_pred_val = np.random.randint(0, settings['predict'], pred_size)
    y_pred = machine.predict(sameples_set[max_pred_val])
    """
    # outputting to image
    # the test image
    if settings['output_graph']:
        plot_dimension = np.ceil(np.sqrt(pred_size))
        for k in xrange(pred_size):
            plt.subplot(plot_dimension, plot_dimension, k + 1)
            plt.imshow(digits.images[max_pred_val[k]], cmap=plt.cm.gray_r, interpolation='nearest')
            plt.title("Test: %d, predict: %d" % (digits.target[max_pred_val[k]], y_pred[k]))

        plt.show()
        plt.savefig('/home/phi/Desktop/digits.png')
        plt.close('all')
