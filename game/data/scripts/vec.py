import numpy as np

#
# This file implements common vector operations
#

def magnitude(v):
    return np.linalg.norm(v)

def sqr_magnitude(v):
    return v[0] ** 2 + v[1] ** 2
    
def normalized(v):
    m = magnitude(v)
    if np.isclose(m, 0): return [0, 0]
    return np.array(v) / m
