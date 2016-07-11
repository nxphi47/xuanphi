#!/bin/env python2

# ------------------------------
# svm-train.py
#	train the svm according to the input training samples
# ------------------------------

import json
import sys
import pprint
import os
import svmclassifier as SVM
import utils
import pca as pca

pp = pprint.PrettyPrinter(indent=2)


def usage():
	print "Usage: %s <infile> <svmfile> [test-only show-wrong show-all dump-pred smooth[=n] late=[hard/soft]]" % \
		  sys.argv[0]
	print '''
	Train/Test an SVM with the training samples.
	For training, a new <svmfile> will be saved after SVM is trained.
		In this case, training samples will be in <infile>, and if <infile>.test
		is present, it will be loaded as test samples to test the trained SVM.
	For testing only, <infile> will be treated as test samples, and a previously
		trained SVM will be loaded from <svmfile>.

	<infile>: the input file generated by generate-training-samples.py
	<svmfile>: the filename to save/load the trained SVM
	test-only: if specified, <infile> will be treated as test samples, and <svmfile>
		must exist
	show-wrong: list the wrongly classified IDs
	show-all: list the classification results of all samples
	dump-pred: dump the prediction results in an output file named <infile>.output
	smooth=n: use smoothing (default n=3).  If late is not specified, early smoothing is done
	late[=hard/soft/both]: specify late smoothing using hard (default) or soft, or both.
	'''
	exit(0)


def get_wrong(samples, labels, ids, all_labels, pred, ALL=False):
	wrong = []
	for i in xrange(len(samples)):
		_label = int(labels[i])
		_pred = int(pred[i])
		if ALL or (_label != _pred):
			wrong.append({'id': ids[i], 'actual': all_labels[_label], 'predicted': all_labels[_pred]})
	return wrong


def print_misclassified(wrong_list):
	print "\nMisclassified samples:"
	for w in wrong_list:
		print "  ID: %s   Actual: %s  ->  Predicted: %s" % (w['id'], w['actual'], w['predicted'])
	print


def print_all(all_list):
	print "\nAll samples:"
	for w in all_list:
		print "  ID: %s   Actual: %s  ->  Predicted: %s" % (w['id'], w['actual'], w['predicted'])
	print


def get_val_for_id(ID, ids, pred):
	if ID in ids:
		idx = ids.index(ID)
		return pred[idx]
	else:
		return None


def hard_smooth(svm, samples, labels, ids, all_labels, pred, seqobjs):
	seqpred = [0] * len(samples)
	for i, x in enumerate(ids):
		_sample = []
		_label = []
		if seqobjs[i]:
			for fv in seqobjs[i]['fv']:
				_sample.append(fv)
				_label.append(labels[i])
			_pred = svm.predict(_sample, _label)
			score = [0] * len(all_labels)
			for p in _pred:
				score[int(p)] += 1
			seqpred[i] = score.index(max(score))
		else:
			seqpred[i] = int(pred[i])
	return seqpred


def soft_smooth(svm, samples, labels, ids, all_labels, pred, seqobjs):
	seqpred = [0] * len(samples)
	for i, x in enumerate(ids):
		_sample = []
		_label = []
		if seqobjs[i]:
			for fv in seqobjs[i]['fv']:
				_sample.append(fv)
				_label.append(labels[i])
			_pred, _prob = svm.predict_p(_sample, _label)
			score = [0.] * len(all_labels)
			for p in _prob:
				for j in xrange(len(p)):
					score[j] += p[j]
			seqpred[i] = score.index(max(score))
		else:
			seqpred[i] = int(pred[i])
	return seqpred


if __name__ == "__main__":
	# ---------- handle input options --------------
	testonly = False
	showwrong = False
	showall = False
	showdump = False
	smooth = False
	late = 'NO'
	numpca = 0
	if len(sys.argv) < 3:
		usage()
	if len(sys.argv) > 3:
		for a in sys.argv[3:]:
			if a == 'test-only':
				testonly = True
			elif a == 'show-wrong':
				showwrong = True
			elif a == 'show-all':
				showall = True
			elif a == 'dump-pred':
				showwrong = True
				showdump = True
			elif 'smooth' in a:
				smooth = True
				if '=' in 'a':
					print "Now only smoothing of 3 is supported"
			elif 'num-pca' in a:
				numpca = int(a.split('=')[1])
			elif 'late' in a:
				late = 'HARD'
				smooth = False
				if 'soft' in a:
					late = 'SOFT'
				if 'both' in a:
					late = 'BOTH'
			else:
				usage()

	# initialize SVM
	svm = SVM.SVMClassifier('-q')
	dumpf = None
	if showdump:  # and (showwrong or showall):
		_filename = sys.argv[2].replace('.svm', '.output')
		if os.path.exists(_filename):
			os.remove(_filename)
		dumpf = open(_filename, 'a')
	pcaf = None
	if numpca > 0:
		pcaf = sys.argv[2].replace('.svm', '.npz')

	# ---------- Training Mode ----------------------
	if not testonly:
		if late == 'NO':
			samples, labels, ids, all_labels, meta = utils.load_training_samples(sys.argv[1], META=True, SMOOTH=smooth)
		else:
			samples, labels, ids, all_labels, meta, seqobjs = utils.load_training_samples(sys.argv[1], META=True,
																						  SMOOTH=False, SEQOBJ=True)
		if meta:
			meta.append({'training-samples': sys.argv[1], 'labels-order': all_labels, 'num-pca': numpca})
		acc = utils.AccStats(all_labels)
		if numpca > 0:
			U, S, V = pca.get_pca(samples, pcaf)
			_v, _m = pca.load(pcaf, numpca)
			svm.cv_train(labels, pca.process_with_pca(samples, _v, _m))
		else:
			svm.cv_train(labels, samples)
		svm.save(sys.argv[2], meta)
		if numpca > 0:
			_v, _m = pca.load(pcaf, numpca)
			pred = svm.predict(pca.process_with_pca(samples, _v, _m), labels)
		else:
			pred = svm.predict(samples, labels)
		acc.update(labels, pred)
		acc.print_table("Training Set Accuracy (Individual):")
		acc.print_table("Training Set Accuracy (Individual):", dumpf)
		if showwrong:
			wr = get_wrong(samples, labels, ids, all_labels, pred)
			print_misclassified(wr)
		if showall:
			wr = get_wrong(samples, labels, ids, all_labels, pred, ALL=True)
			print_all(wr)
		if showdump:
			dumpf.write(json.dumps(wr) + '\n')
		if late in ['HARD', 'BOTH']:
			acc.reset()
			acc.update(labels, hard_smooth(svm, samples, labels, ids, all_labels, pred, seqobjs))
			acc.print_table("Training Set Accuracy (Hard-Smoothing):")
			acc.print_table("Training Set Accuracy (Hard-Smoothing):", dumpf)
		if late in ['SOFT', 'BOTH']:
			acc.reset()
			acc.update(labels, soft_smooth(svm, samples, labels, ids, all_labels, pred, seqobjs))
			acc.print_table("Training Set Accuracy (Soft-Smoothing):")
			acc.print_table("Training Set Accuracy (Soft-Smoothing):", dumpf)

	# ---------- Testing Mode ----------------------
	if testonly:
		svm.load(sys.argv[2])
		for m in svm.meta:
			if m.has_key('labels-order'):
				all_labels = m['labels-order']
			if m.has_key('num-pca'):
				numpca = m['num-pca']
		acc = utils.AccStats(all_labels)
		if late == 'NO':
			samples, labels, ids, all_labels, meta = utils.load_training_samples(sys.argv[1], all_labels=all_labels,
																				 META=True, SMOOTH=smooth)
		else:
			samples, labels, ids, all_labels, meta, seqobjs = utils.load_training_samples(sys.argv[1], META=True,
																						  SMOOTH=False, SEQOBJ=True)
	else:
		acc.reset()
		if late == 'NO':
			samples, labels, ids, all_labels, meta = utils.load_training_samples(sys.argv[1] + '.test',
																				 all_labels=all_labels, META=True,
																				 SMOOTH=smooth)
		else:
			samples, labels, ids, all_labels, meta, seqobjds = utils.load_training_samples(sys.argv[1] + '.test',
																						   META=True, SMOOTH=False,
																						   SEQOBJ=True)
	if numpca > 0:
		_v, _m = pca.load(pcaf, numpca)
		pred = svm.predict(pca.process_with_pca(samples, _v, _m), labels)
	else:
		pred = svm.predict(samples, labels)
	acc.update(labels, pred)
	acc.print_table("Test Set Accuracy (Individual):")
	acc.print_table("Test Set Accuracy (Individual):", dumpf)
	if showwrong:
		wr = get_wrong(samples, labels, ids, all_labels, pred)
		print_misclassified(wr)
	if showall:
		wr = get_wrong(samples, labels, ids, all_labels, pred, ALL=True)
		print_all(wr)
	if showdump:
		dumpf.write(json.dumps(wr) + '\n')
	if late in ['HARD', 'BOTH']:
		acc.reset()
		acc.update(labels, hard_smooth(svm, samples, labels, ids, all_labels, pred, seqobjs))
		acc.print_table("Test Set Accuracy (Hard-Smoothing):")
		acc.print_table("Test Set Accuracy (Hard-Smoothing):", dumpf)
	if late in ['SOFT', 'BOTH']:
		acc.reset()
		acc.update(labels, soft_smooth(svm, samples, labels, ids, all_labels, pred, seqobjs))
		acc.print_table("Test Set Accuracy (Soft-Smoothing):")
		acc.print_table("Test Set Accuracy (Soft-Smoothing):", dumpf)

	if meta:
		print "Meta:",
		pprint.pprint(meta)
		if showdump:
			dumpf.write('\n')
			dumpf.write(str(meta))
			dumpf.close()