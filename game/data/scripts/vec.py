import numpy as np

import math

#
# This file implements common vector operations
#

def dot_mat(mat, v):
	"""
	Helper function to transform a 2D vector by a 3x3 matrix
	"""
	return (mat @ [v[0], v[1], 1])[:2]

def dot(v1, v2):
    """
    Returns the cross product of two 2D vectors.
    "np.cross" also works but it's slower because it's designed to handle large arrays!
    """
    return v1[0] * v2[0] + v1[1] * v2[1]

def cross(v1, v2):
    """
    Returns the cross product of two 2D vectors.
    The cross product of 2D vectors results in a 3D vector with only a z component.
    This function returns the magnitude of the z value.
    "np.cross" also works but it's slower because it's designed to handle large arrays!
    """
    return v1[0] * v2[1] - v1[1] * v2[0]

def magnitude(v):
	return math.sqrt(dot(v, v))

def sqr_magnitude(v):
	return dot(v, v)
	
def normalized(v):
	m = magnitude(v)
	return v / m

def orthogonal(v):
	"""
	Returns the clockwise perpendicular vector
	"""
	return np.array([v[1], -v[0]])

def clamp_magnitude(v, length):
	"""
	Return a copy of v but with length clamped
	"""
	v = np.array(v)

	sq_m = np.dot(v, v)
	if sq_m < length ** 2: return v
	return v * length / np.sqrt(sq_m)


