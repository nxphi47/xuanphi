try:
    from svmutil import *
except:
    from libsvm.svmutil import *

matrix = [
    [0, 0, 0, 3, 0, 0, 0, 0],
    [0, 0, 0, 1, 0, 0, 0, 0],
    [0, 1, 0, 1, 0, 3, 3, 0],
    [0, 0, 0, 0, 0, 0, 3, 0],
    [0, 1, 0, 2, 2, 0, 0, 0],
    [0, 0, 0, 2, 2, 0, 0, 0],
    [0, 0, 0, 0, 2, 0, 0, 0],
    [0, 0, 0, 0, 2, 0, 0, 0]
]

samples = []
labels = []

#get labels with samples

for i in range(len(matrix)):
    for j in range(len(matrix)):
        if matrix[i][j] != 0:
            labels.append(matrix[i][j])
            samples.append([i, j])

problem = svm_problem(labels, samples)
param = svm_parameter('-s 0 -t 2 -q')

model = svm_train(problem, param)
test_label = [1,1,3,1]
test_sample = [
    [0,0],
    [0,1],
    [2,7],
    [4,2]
]


pred_label, pre_acc, pre_val = svm_predict(test_label, test_sample, model)
print pred_label

import matplotlib.pyplot as plt
import numpy as np
import math

time = np.arange(0.,5.,0.2)
exp = []
for t in time:
    exp.append(math.exp(t))

plt.plot(time, 1/time,'r', time, time, 'b', time, exp, 'g')
plt.show()