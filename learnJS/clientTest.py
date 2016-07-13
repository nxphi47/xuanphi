import numpy as np
import matplotlib.pyplot as plt

X = np.linspace(-np.pi, np.pi, 256, endpoint=True)
C, S = np.cos(X), np.sin(X)

plt.figure(figsize=(16,12), dpi=80)

plt.subplot(2,1,1)
plt.plot(X, C)
plt.subplot(2,1,2)
plt.plot(X, S)
plt.show()