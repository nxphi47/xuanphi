import numpy as np
from scipy import stats
from scipy import misc
from scipy import linalg
from scipy import optimize

def func(x):
	return 2*x*x + 5*x + 1

x = np.linspace(-10, 10, num=20)
y = func(x) + np.random.randn(x.size)

