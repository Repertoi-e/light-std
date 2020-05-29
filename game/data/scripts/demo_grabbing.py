import lstdgraphics as g

import numpy as np 
import shape

import math

import cProfile, pstats

from drawing import draw_shape
from shape import random_convex_polygon
from body import Body, ensure_transformed_shape, apply_force, apply_impulse, set_static
from hit import point_in_shape, aabb_vs_aabb, push_vector

from vec import clamp_magnitude, magnitude, sqr_magnitude, normalized, dot

import data_demo_grabbing as data

gravity = 9.8
drag = 0.7

bodies = []
triangle, floor = None, None

def load(state):
	"""
	Called from C++ side. Sets the state which "lstdgraphics" uses for drawing.
	"""
	g.state(state)

	global triangle, floor
	vertices = [
		[-1, -1],
		[1, -1],
		[0, 1]
	]
	poly = shape.ConvexPolygon(vertices, 0xed37d8)
	triangle = Body(poly, 10)
	triangle.pos = np.array([10.0, 0.0])
	bodies.append(triangle)

	rect = shape.make_rect(100, 0.2, 0x42f5d7)
	floor = Body(rect, 1, static = True)
	floor.pos = np.array([10.0, -10.0])
	bodies.append(floor)

def unload():
	"""
	Called when the script is unloaded
	"""
	bodies.clear()

def mouse_click(x, y, rightButton):
	if rightButton:
		sh = None
		if np.random.random_sample() < 0.5:
			vs = random_convex_polygon(np.random.randint(3, 8)) * 3
			sh = shape.ConvexPolygon(vs, np.random.randint(0x1000000))
		else:
			sh = shape.Circle(np.random.randint(10) * 0.2 + 1)
		b = Body(sh, 10)
		b.pos = np.copy(data.mouse)
		bodies.append(b)
		return

	if not data.mouse_line:
		data.mouse_line = True
		data.mouse_start = np.array([x, y])

	for b in bodies:
		mouse_in_local_space = dot(b.inv_model_mat, data.mouse)

		if point_in_shape(mouse_in_local_space, b.shape):
			data.grabbed = b
			data.grabbed_offset = data.mouse - b.pos
			data.grabbed_was_static = b.static
			
			# We null all forces and stop moving the object while grabbing it
			set_static(b, True)
			break

def mouse_release(rightButton):
	if rightButton: return

	if data.mouse_line:
		data.mouse_line = False

	if data.grabbed:
		if not data.grabbed_was_static:
			set_static(data.grabbed, False)
		
		F = None
		if data.mouse is None or data.mouse_last is None:
			F = [0, 0] 
		else:
			F = (data.mouse - np.array(data.mouse_last)) * 5000
		
		apply_force(data.grabbed, F, data.grabbed_offset)
		data.grabbed = None


def mouse_move(x, y):
	data.mouse_last = data.mouse
	data.mouse = np.array([x, y])
	if data.grabbed:
		data.grabbed.pos = data.mouse - data.grabbed_offset
		data.grabbed.dirty_transform = True

def frame(dt):
	"""
	Called each frame from C++ side. Use "lstdgraphics" module to draw primitives.
	We also have some helper functions in "drawing.py" (which also use "lstdgraphics").
	"""

	#profile = cProfile.Profile()
	#profile.enable()

	for b in bodies:
		if not b.static:
			# Integrate acceleration and velocity
			acc = b.force / b.mass - np.array([0, 1]) * gravity
			b.vel += acc * dt
			b.pos += b.vel * dt

			ang_acc = b.torque / b.rot_inertia
			b.ang_vel += ang_acc * dt
			b.rot += b.ang_vel * dt

			# Apply air drag
			b.vel -= clamp_magnitude(b.vel, 1) * drag * dt
			b.ang_vel -= clamp_magnitude(b.ang_vel, 1) * drag * dt

			b.force = np.array([0.0, 0.0])
			b.torque = 0

			b.dirty_transform = True
		ensure_transformed_shape(b)
	
	for i in range(len(bodies)):
		a = bodies[i]
		for j in range(i + 1, len(bodies)):
			b = bodies[j]

			# Broad phase (cheaper to calculate)
			if not aabb_vs_aabb(a.transformed_shape.aabb, b.transformed_shape.aabb):
				continue

			# Narrow phase
			pv = push_vector(a.transformed_shape, b.transformed_shape)
			if pv is not None:
				rv = b.vel - a.vel

				collision_normal = normalized(pv)
				vel_along_normal = np.dot(rv, collision_normal)

				if math.isclose(sqr_magnitude(collision_normal), 0):
					continue

				e = min(a.restitution, b.restitution)
				j = -(1 + e) * vel_along_normal
				j /= a.inv_mass + b.inv_mass
				
				mass_sum = a.mass + b.mass

				impulse = j * collision_normal
				apply_impulse(a, -(a.mass / mass_sum) * impulse)
				apply_impulse(b, (b.mass / mass_sum) * impulse)

				# Positional correction
				correction = magnitude(pv) / (a.inv_mass + b.inv_mass) * 0.2 * collision_normal
				a.pos -= a.inv_mass * correction
				b.pos += b.inv_mass * correction

	for b in bodies:
		draw_shape(b.transformed_shape, thickness = 3, aabb = b.transformed_shape.aabb)

	if data.mouse_line and data.mouse_start is not None and data.mouse is not None:
		g.line(data.mouse_start, data.mouse, color = 0xffe62b)

	#profile.disable
	#ps = pstats.Stats(profile).sort_stats("cumulative")
	#ps.print_stats()
