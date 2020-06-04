import numpy as np 
import shape as sh

import math

from shape import ConvexPolygon, Circle, edges
from vec import magnitude, sqr_magnitude, normalized, orthogonal, dot

def line_vs_line(line1, line2):
	"""
	Returns the point at which two lines intersect (or None if they don't)
	"""
	p1 = np.array(line1[0]).astype(float)
	p2 = np.array(line1[1]).astype(float)

	w1 = np.array(line2[0]).astype(float)
	w2 = np.array(line2[1]).astype(float)
	
	xdiff = (p1[0] - p2[0], w1[0] - w2[0])
	ydiff = (p1[1] - p2[1], w1[1] - w2[1])

	div = np.linalg.det([xdiff, ydiff])
	if div == 0: return None
	
	d = (np.linalg.det(line1), np.linalg.det(line2))
	x = np.linalg.det([d, xdiff]) / div
	y = np.linalg.det([d, ydiff]) / div
	return np.array([x, y])

def is_between(a, b, c):
	"""
	Checks if c is between a and b
	"""
	if a > b: 
		a, b = b, a 
	return a < c < b

def is_point_on_ray_extension(ray, k):
	"""
	Checks if the point k lies on the extension of the ray (the part of the line that isn't on top of the ray)
	"""
	b, d = ray[0], ray[0] + ray[1]
	return is_between(k[0], d[0], b[0]) or is_between(k[1], d[1], b[1])

def ray_vs_segment(ray, segment):
	"""
	Returns the point at which a ray and a segment intersect (or None if they don't).
	This doesn't pay attention to the ends of the segment.
	"""
	b, d = ray[0], ray[1]
	k = line_vs_line([b, b + d], segment)

	if k is None: return None
	if is_point_on_ray_extension(ray, k): 
		return None
	
	a, b = segment[0], segment[1]
	m = min(a[0], b[0])
	n = max(a[0], b[0])
	if k[0] < m or math.isclose(k[0], m) or k[0] > n or math.isclose(k[0], n):
		return None
	m = min(a[1], b[1])
	n = max(a[1], b[1])
	if k[1] < m or math.isclose(k[1], m) or k[1] > n or math.isclose(k[1], n):
		return None
	return k


def point_in_circle(p, center, radius):
	"""
	Returns whether a point is inside a given circle (p is in local space)
	"""
	v = np.array(p).astype(float) - center
	return sqr_magnitude(v) < radius ** 2

def point_in_polygon(p, vertices, es = None):
	"""
	Returns whether a point is inside a given convex polygon (p is in local space)
	"""
	p = np.array(p).astype(float)

	if es is None: es = edges(vertices)
	u = es[:, 0] - p
	v = es[:, 1] - p
	return np.greater(np.cross(u, v), 0).all()

def point_in_aabb(p, aabb):
	"""
	Returns whether a point is inside a given AABB (p is in local space)
	"""
	p = np.array(p).astype(float)

	b1 = abs(aabb[0][0] - p[0]) <= aabb[1][0]
	b2 = abs(aabb[0][1] - p[1]) <= aabb[1][1]
	return b1 and b2

def aabb_vs_aabb(aabb1, aabb2):
	"""
	Returns whether two AABBs are colliding.
	Each AABB is represented by a center and half-width/height.  
	"""
	c1, c2 = aabb1[0], aabb2[0]
	e1, e2 = aabb1[1], aabb2[1]

	b1 = abs(c1[0] - c2[0]) <= (e1[0] + e2[0])
	b2 = abs(c1[1] - c2[1]) <= (e1[1] + e2[1])
	return b1 and b2

def point_in_shape(p, shape):
	"""
	Generalized version of point_in_... for shapes
	"""
	if not point_in_aabb(p, shape.aabb):
		return None
	if shape.type == sh.Type.CIRCLE:
		return point_in_circle(p, [0, 0], shape.radius)
	else:
		return point_in_polygon(p, None, shape.edges)

def is_separating_axis_polygons(axis, v1, v2):
	"""
	Returns True and None if "axis" is a separating axis of v1 and v2 (v1 and v2 are vertices).
	Return False and the the overlapping amount otherwise.
	"""
	m1, n1 = float("Inf"), float("-Inf")
	m2, n2 = float("Inf"), float("-Inf")

	for proj in np.dot(v1, axis):
		if proj < m1:
			m1 = proj
		if proj > n1:
			n1 = proj

	for proj in np.dot(v2, axis):
		if proj < m2:
			m2 = proj
		if proj > n2:
			n2 = proj

	if n1 >= m2 and n2 >= m1:
		d = min(n2 - m1, n1 - m2)
		# Since we calculate the projection above without diving by 
		# the squared magnitude (saves time) we need to do that now.
		# We also push a bit more than needed so the shapes don't 
		# overlap in future tests due to float precision
		return False, (d / sqr_magnitude(axis) + 1e-10)
	else:
		return True, None

def is_separating_axis_polygon_and_circle(axis, v1, center, radius):
	"""
	Returns True and None if "axis" is a separating axis of v1 and a circle (v1 are vertices).
	Return False and the the overlapping amount otherwise.
	"""
	m1, n1 = float("Inf"), float("-Inf")

	for v in v1:
		proj = dot(v, axis)
		m1 = min(m1, proj)
		n1 = max(n1, proj)

	proj = dot(center, axis)
	m2 = proj - radius
	n2 = proj + radius

	if n1 >= m2 and n2 >= m1:
		d = min(n2 - m1, n1 - m2)
		# Since we calculate the projection above without diving by 
		# the squared magnitude (saves time) we need to do that now.
		# We also push a bit more than needed so the shapes don't 
		# overlap in future tests due to float precision
		return False, (d / sqr_magnitude(axis) + 1e-10)
	else:
		return True, None

def polygon_vs_polygon_minimum_translation_vector(polygon_a, polygon_b):
	"""
	Returns the minimum translation vector of two polygons (None if they don't intersect).
	"polygon_a" and "polygon_b" must be instances of ConvexPolygon.
	"""
	axes = np.concatenate((polygon_a.normals, polygon_b.normals), axis = 0)

	overlaps = []
	for axis in axes:
		separating, overlap = is_separating_axis_polygons(axis, polygon_a.vertices, polygon_b.vertices)
		if separating:
			return None # The polygons don't overlap if there exists at least one axis of separation
		overlaps.append(axis * overlap)

	return min(overlaps, key = (lambda x: sqr_magnitude(x)))

def polygon_vs_circle_minimum_translation_vector(polygon, circle):
	"""
	Returns the minimum translation vector of a polygon and a circle (None if they don't intersect).
	"polygon" and "circle" must be instances of ConvexPolygon and Circle respectively.
	"""
	closest, closest_dist = None, float("Inf")
	for v in polygon.vertices:
		u = v - circle.centroid
		d = sqr_magnitude(u)
		if d < closest_dist:
			closest = u
			closest_dist = d
	axes = np.concatenate((polygon.normals, [normalized(closest)]), axis = 0)

	overlaps = []
	for axis in axes:
		separating, overlap = is_separating_axis_polygon_and_circle(axis, polygon.vertices, circle.centroid, circle.radius)
		if separating:
			return None # The polygon and circle don't overlap if there exists at least one axis of separation
		overlaps.append(axis * overlap)

	return min(overlaps, key = (lambda x: sqr_magnitude(x)))

def minimum_translation_vector(shape_a, shape_b):
	"""
	Returns the minimum translation vector of two shapes. (None if they don't intersect).
	"""
	a_circle, b_circle = shape_a.type == sh.Type.CIRCLE, shape_b.type == sh.Type.CIRCLE
	
	if a_circle and b_circle:
		a_to_b = shape_b.centroid - shape_a.centroid
		r = (shape_a.radius + shape_b.radius)
		if sqr_magnitude(a_to_b) > r ** 2:
			return None
		d = magnitude(a_to_b)
		if not math.isclose(d, 0):
			return a_to_b / d * (r - d)
		else:
			return np.array([1.0, 0.0]) * r
	if a_circle != b_circle:
		# We need to feed polygon_vs_circle_overlap first the polygon and the circle second 
		if a_circle:
			return polygon_vs_circle_minimum_translation_vector(shape_b, shape_a)
		else:
			return polygon_vs_circle_minimum_translation_vector(shape_a, shape_b)
	else:
		return polygon_vs_polygon_minimum_translation_vector(shape_a, shape_b)

# OLD. Don't use this.
def get_contact_points(shape, mtv):
	if shape.type == sh.Type.CIRCLE: return [shape.centroid]

	n1, n2 = float("-Inf"), float("-Inf")
	n1_v, n2_v = None, None

	for v in shape.vertices:
		proj = dot(v, mtv)
		if proj > n2:
			if proj >= n1:
				n1, n2 = proj, n1
				n1_v, n2_v = v, n1_v
			else:
				n2 = proj
				n2_v = v
	
	tolerance = 0.1
	if n1 - n2 < tolerance:
		return [n1_v, n2_v]
	else:
		return [n1_v]

# OLD. Don't use this.
def get_average_contact_point(shape, mtv):
	"""
	Returns a very very crude approximation of the contact point of a shape when a collision occurs.
	It is used to calculate the angular velocity the shape recieves after the collision.
	Calculates the two vertices with the largest projection onto "mtv" 
	and returns the max or their average using some specified tolerance. 
	For circles it returns the center (we don't even bother, you can't tell if they are rotating haha).
	"""
	if shape.type == sh.Type.CIRCLE: return shape.centroid

	n1, n2 = float("-Inf"), float("-Inf")
	n1_v, n2_v = None, None

	for v in shape.vertices:
		proj = dot(v, mtv)
		if proj > n2:
			if proj >= n1:
				n1, n2 = proj, n1
				n1_v, n2_v = v, n1_v
			else:
				n2 = proj
				n2_v = v
	
	tolerance = 0.1
	if n1 - n2 < tolerance:
		return (n1_v + n2_v) / 2
	else:
		return n1_v