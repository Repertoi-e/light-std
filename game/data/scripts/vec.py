import numpy as np

#
# This file implements common vector operations
#

def magnitude(v):
    return np.linalg.norm(v)

def sqr_magnitude(v):
    return v[0] ** 2 + v[1] ** 2
    
def normalized(v):
    return np.array(v) / magnitude(v)
