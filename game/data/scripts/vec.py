import numpy as np

#
# This file implements common vector operations
#

def magnitude(v):
    return np.linalg.norm(v)

def normalized(v):
    return np.array(v) / magnitude(v)

