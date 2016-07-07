import json, sys, pprint, os
import svmclassifier as SVM
import utils
import numpy as np
from pls import PartialLeastSquares

LABELS = ["C<15", "C>15"]
smooth=False
showdump=False

if __name__ == "__main__":
	
	if len(sys.argv) < 3:
		print 'wrong command'
		exit(1)

	if len(sys.argv) > 3:
		for a in sys.argv[3:]:
			if 'smooth':
				smooth = True
			if 'dump-pred':
				showdump = True 
	
	dumpf = None
	if showdump:
		_filename = sys.argv[2].replace('.svm', '.output')
		if os.path.exists(_filename):
			os.remove(_filename)
		dumpf = open(_filename, 'a')
	
	# initialize SVM
	svm = SVM.SVMClassifier('-q')
	
	# Training
	samples, labels, ids, all_labels, meta = utils.load_training_samples(sys.argv[1], all_labels=LABELS, META=True, SMOOTH=smooth)
	if meta:
		meta.append({'training-samples':sys.argv[1], 'labels-order': all_labels })
	acc = utils.AccStats(all_labels)
	pls = PartialLeastSquares(epsilon = 0.001, debug=1, )
	
	pls.set_X_matrix(samples)
	pls.set_Y_matrix(np.matrix(labels).T.tolist())
	B, W = pls.PLS()

	Xtrain = samples * W
	
	# cross-validating
	svm.cv_train(labels, Xtrain.tolist())
	svm.save(sys.argv[2], meta)
	
	# train set prediction
	pred = svm.predict(Xtrain.tolist(), labels)
	acc.update(labels, pred)
	acc.print_table("Training Set Accuracy (Individual):")
	acc.print_table("Training Set Accuracy (Individual):", dumpf)
	
	# test set prediction
	acc.reset()
	samples, labels, ids, all_labels, meta = utils.load_training_samples(sys.argv[1]+'.test', all_labels=all_labels, META=True, SMOOTH=smooth)
	Xtest = samples * W
	pred = svm.predict(Xtest.tolist(), labels)
	acc.update(labels, pred)
	acc.print_table("Test Set Accuracy (Individual):")
	acc.print_table("Test Set Accuracy (Individual):", dumpf)

