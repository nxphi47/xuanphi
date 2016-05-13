#!/bin/env python2

# ------------------------------
# process PCA function before SVM
# ------------------------------

import numpy as np

# get transformation matrix 
#	samples = array to be transform
#	OUTFILE = specify output file pathname, default is not output
#	return transformation matrix and eigenvector valus
def get_pca(samples, OUTFILE=None):
	X = np.array(samples)
	
	np_mean = np.mean(X, axis=0)
	Xzero = X - np_mean					# zero-center the data
	#cov = np.dot(Xzero.T, Xzero) / Xzero.shape[0]	# get data covariance matrix
	
	U, S, V = np.linalg.svd(Xzero)
	
	if not OUTFILE is None:
		save(V, np_mean, OUTFILE)
	
	return U, S, V

def process_with_pca(samples, V, M=None, NUM=0):
	X = np.array(samples)
	Xzero = X - M					# zero-center the data
	
	print 'V Shape: ', V.shape
	
	if NUM > 0:
		Vrot = V[:,:NUM]
	else:
		Vrot = V
	
	print 'Xzero Shape: ', Xzero.shape
	print 'Urot Shape: ', Vrot.shape
	
	Xrot = np.dot(Xzero, Vrot)
	return Xrot.tolist()

def load(filename, NUM=0):
	npzfile = np.load(filename)
	V = npzfile['V']
	mean = npzfile['mean']
	print 'V: ', V
	print 'V shape: ', V.shape
	print 'MEAN: ', mean
	print 'Mean shape: ', mean.shape
	if NUM > 0:
		Vprint = V[:,:NUM]
		print 'EigenVector Shape: ', Vprint.shape
		return Vprint, mean
	return V, mean	

# output transformation matrix to specify file
#	U = transformation matrix
#	outputFile = output file path name
def save(V, np_mean, outputFile):
	np.savez(outputFile, V=V, mean=np_mean)


	

