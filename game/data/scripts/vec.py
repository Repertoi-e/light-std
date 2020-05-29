import numpy as np

import math

#
# This file implements common vector operations
#

def magnitude(v):
    return np.linalg.norm(v)

def sqr_magnitude(v):
    return np.dot(v, v)
    
def normalized(v):
    m = magnitude(v)
    if math.isclose(m, 0): return [0, 0]
    return np.array(v) / m

def orthogonal(v):
    """
    Returns the clockwise perpendicular vector
    """
    return np.array(v[1], -v[0])

def clamp_magnitude(v, length):
    """
    Return a copy of v but with length clamped
    """
    v = np.array(v)

    sq_m = sqr_magnitude(v)
    if sq_m < length ** 2: return v
    return v * length / np.sqrt(sq_m)

def dot(mat, v):
    """
    Helper function to transform a 2D vector by a 3x3 matrix
    """
    return (mat @ [v[0], v[1], 1])[:2]
