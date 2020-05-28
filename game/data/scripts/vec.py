import numpy as np

#
# This file implements common vector operations
#

def magnitude(v):
    return np.linalg.norm(v)

def sqr_magnitude(v):
    result = 0
    if v.size == 1: return v ** 2
    for i in range(v.size):
        result += v[i] ** 2
    return result
    
def normalized(v):
    m = magnitude(v)
    if np.isclose(m, 0): return [0, 0]
    return np.array(v) / m

# Return a copy of v but with length clamped
def clamp_magnitude(v, length):
    v = np.array(v).astype(float)

    sq_m = sqr_magnitude(v)
    if sq_m < length ** 2: return v
    return v * length / np.sqrt(sq_m)

