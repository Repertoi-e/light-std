import numpy as np
import shape as sh

import copy
import itertools

from shape import edges
from vec import sqr_magnitude, dot_mat, normalized, dot, cross

class Body:
	def __init__(self, shape, density, static = False):
		self.pos = np.array([0, 0]).astype(float)
		self.rot = 0.0
		
		self.vel = np.array([0, 0]).astype(float)
		self.ang_vel  = 0.0

		self.dirty_transform = True
		self.model_mat = np.identity(3)
		self.inv_model_mat = np.identity(3)

		self.force = np.array([0, 0]).astype(float)
		self.torque = 0.0

		self.shape = shape
		self.transformed_shape = copy.deepcopy(shape)

		if not static:
			self.mass = density * shape.area
			self.inv_mass = 1 / self.mass

			if shape.type == sh.Type.CIRCLE:
				self.rot_inertia = 0.5 * self.mass * (shape.radius ** 2)
				self.inv_rot_inertia = 1 / self.rot_inertia
			else:
				self.rot_inertia = 0
				for e in shape.edges:
					a, b = e[0], e[1]
					tri_mass = density * 0.5 * np.abs(cross(a, b))
					tri_inertia = tri_mass * (sqr_magnitude(a) + sqr_magnitude(b) + dot(a, b)) / 6
					self.rot_inertia += tri_inertia
					self.inv_rot_inertia = 1 / self.rot_inertia
		else:
			self.mass = 0.0
			self.inv_mass = 0.0
			self.rot_inertia = 0.0
			self.inv_rot_inertia = 0.0

		self.static = static

def calculate_body_model_mat(body):
	c, s = np.cos(body.rot), np.sin(body.rot)
	R = np.array([
		[c, -s, 0], 
		[s, c, 0],
		[0, 0, 1]])
	T = np.array([
		[1, 0, body.pos[0]], 
		[0, 1, body.pos[1]], 
		[0, 0, 1]]).astype(float)
	body.model_mat = T @ R
	body.inv_model_mat = np.linalg.inv(body.model_mat)

def edges_itertools(vertices):
    a, b = itertools.tee(vertices)
    next(b, None)
    yield from zip(a, b)
    yield [vertices[-1], vertices[0]]

def ensure_transformed_shape(body):
	if body.dirty_transform:
		body.dirty_transform = False
		
		calculate_body_model_mat(body)
		t = body.transformed_shape
		if body.shape.type == sh.Type.CIRCLE:
			t.centroid = body.pos
			t.aabb = np.array([t.centroid, [t.radius, t.radius]])
		else:
            # Since this method gets called a lot we do a lot of "cryptic" code here.
            # This basically does the same as __init__ in body but in a faster way (thanks to numpy magic).
            # I decided to keep optimized code away from methods that don't need to be fast (e.g initialization).
            # But this method gets called for *each* body *every* frame, so yeah..
			t.vertices = np.empty((len(body.shape.vertices), 2))

			mx, nx = float("Inf"), float("-Inf")
			my, ny = float("Inf"), float("-Inf")
			for i, v in enumerate(body.shape.vertices):
				d = dot_mat(body.model_mat, v)
				t.vertices[i] = d
				mx = min(d[0], mx)
				my = min(d[1], my)
				nx = max(d[0], nx)
				ny = max(d[1], ny)

			t.edges = np.array(list(edges_itertools(t.vertices)))

			edges_v = t.edges[:, 1] - t.edges[:, 0]
			orthogonal = np.flip(edges_v, 1) * [1, -1]
			magnitudes = np.sqrt(np.sum(np.square(orthogonal), axis = 1)).reshape(len(t.vertices), 1)
			t.normals = orthogonal / magnitudes 

			t.centroid = dot_mat(body.model_mat, [0, 0])

			hx = (nx - mx) / 2
			hy = (ny - my) / 2
			t.aabb = np.array([[mx + hx, my + hy], [abs(hx), abs(hy)]])

def apply_force(body, force, point = [0.0, 0.0]):
	"""
	Applies a force at a given point, using that point to calculate the torque.
	"""
	body.force += force
	body.torque += cross(point, force)

def apply_impulse(body, impulse, point = [0.0, 0.0]):
	"""
	Applies mpulse to a body and takes into account it's mass.
	Uses point to calculate the angular velocity.
	"""
	body.vel += impulse * body.inv_mass
	body.ang_vel += cross(point, impulse) * body.inv_rot_inertia

def set_static(body, static):
	if body.static == static: return
	if not body.static:
		body.vel = np.array([0, 0]).astype(float)
		body.ang_vel = 0.0
		body.force = np.array([0, 0]).astype(float)
		body.torque = 0.0
	body.static = static
