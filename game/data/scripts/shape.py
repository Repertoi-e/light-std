import lstdgraphics as g

import numpy as np

from vec import normalized, orthogonal, cross
from enum import Enum

#
# This file implements basic shapes which our demo uses.
# As convention we consider the vertices of polygons to be ordered counter-clockwise (this affects calculations).
#

class Type(Enum):
	CIRCLE = 1
	POLY = 2

class Shape:
	def __init__(self): pass

class Circle(Shape):
	def __init__(self, radius, color = 0x4254f5, center = [0, 0]):
		self.type = Type.CIRCLE
		self.radius = float(radius)

		self.area = radius ** 2 * np.pi
		self.centroid = np.array(center).astype(float)

		self.aabb = np.array([self.centroid, [self.radius, self.radius]])

		self.color = color

def edges(vertices):
	result = np.empty((len(vertices), 2, 2))
	a = vertices[0]
	for i in range(1, len(vertices) + 1):
		b = vertices[i % len(vertices)] # At the end we cycle back to the first vertex
		result[i - 1] = np.array([a, b])
		a = b
	return result

def shoelace(es):
	area = 0
	for e in es:
		area += 0.5 * cross(e[0], e[1])
	return area

def centroid(es):
	result = np.array([0.0, 0.0])
	
	area_sum = 0.0
	for e in es:
		tri_area = 0.5 * cross(e[0], e[1])
		area_sum += tri_area
		result += (e[0] + e[1]) / 3 * tri_area
	return result / area_sum

class ConvexPolygon(Shape):
	def __init__(self, vertices, color = 0x4254f5, recenter = True):
		self.type = Type.POLY

		self.vertices = np.array(vertices).astype(float)
		self.normals = np.empty((len(vertices), 2)).astype(float)
		self.edges = np.empty((len(vertices), 2, 2)).astype(float)

		es = edges(self.vertices)

		self.area = shoelace(es)
		self.centroid = centroid(es)
		
		if recenter: self.vertices -= self.centroid

		# Recalculate edges after translating the vertices
		self.edges = edges(self.vertices)

		for i, e in enumerate(self.edges):
			self.normals[i] = normalized(orthogonal(e[1] - e[0]))

		mx, nx = float("Inf"), float("-Inf")
		my, ny = float("Inf"), float("-Inf")
		
		for v in self.vertices:
			mx = min(v[0], mx)
			my = min(v[1], my)
			nx = max(v[0], nx)
			ny = max(v[1], ny)
		hx = (nx - mx) / 2
		hy = (ny - my) / 2
   
		self.aabb = np.array([[mx + hx, my + hy], [abs(hx), abs(hy)]])

		self.color = color

def random_convex_polygon(n):
	x = np.sort(np.random.uniform(0, 1, n))
	y = np.sort(np.random.uniform(0, 1, n))
	min_x, max_x = x[0], x[-1]
	min_y, max_y = y[0], y[-1]

	def chains(min_a, max_a, a):
		r = np.array([])
		last_top, last_bot = min_a, min_a
		for i in range(1, len(a) - 1):
			t = a[i]
			if np.random.random_sample() < 0.5:
				r = np.append(r, t - last_top)
				last_top = t
			else:
				r = np.append(r, last_bot - t)
				last_bot = t
		r = np.append(r, max_a - last_top)
		r = np.append(r, last_bot - max_a)
		return r

	x_vec = chains(min_x, max_x, x)
	y_vec = chains(min_y, max_y, y)
	np.random.shuffle(y_vec)

	vec = list(zip(x_vec, y_vec))
	vec.sort(key = lambda v: np.arctan2(v[1], v[0]))

	vertex = np.array([0.0, 0.0])
	m = np.array([0.0, 0.0])
	vertices = []
	for v in vec:
		vertices.append(vertex)
		vertex = vertex + v
		m = np.minimum(m, vertex)
	return np.array(vertices) - (np.array([min_x, min_y]) - m)

# Returns a centered rectangle with half_width, half_height extents
def make_rect(half_width, half_height, color = 0x4254f5):
	vertices = [
		[-half_width, -half_height],
		[half_width, -half_height],
		[half_width, half_height],
		[-half_width, half_height]
	]
	return ConvexPolygon(vertices, color = color)