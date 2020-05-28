import lstdgraphics as g

import numpy as np 
import shape

import cProfile, pstats

from drawing import draw_shape, dot
from shape import random_convex_polygon
from body import Body, apply_force, set_static
from hit import point_in_shape

from vec import clamp_magnitude

import data_demo_grabbing as data

gravity = 9.8
drag = 0.7

bodies = [None] * 1000
num_bodies = 0

triangle, floor = None, None

def add_body(b):
	global num_bodies
	bodies[num_bodies] = b
	num_bodies += 1

def load(state):
	'''
	Called from C++ side. Sets the state which 'lstdgraphics' uses for drawing.
	'''
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
	add_body(triangle)

	rect = shape.make_rect(100, 0.2, 0x42f5d7)
	floor = Body(rect, 1, static = True)
	floor.pos = np.array([10.0, 10.0])
	add_body(floor)

def unload():
	'''
	Called when the script is unloaded
	'''
	global bodies, num_bodies
	bodies = [None] * 1000
	num_bodies = 0

def mouse_click(x, y, rightButton):
	if rightButton:
		global num_bodies
		vs = random_convex_polygon(np.random.randint(3, 8)) * 3
		poly = shape.ConvexPolygon(vs, np.random.randint(0x1000000))
		b = Body(poly, 10)
		b.pos = np.copy(data.mouse)
		add_body(b)
		return

	if not data.mouse_line:
		data.mouse_line = True
		data.mouse_start = np.array([x, y])

	for i in range(num_bodies):
		b = bodies[i]
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

def frame(dt):
	'''
	Called each frame from C++ side. Use 'lstdgraphics' module to draw primitives.
	We also have some helper functions in 'drawing.py' (which also use 'lstdgraphics').
	'''
	floor_y = floor.pos[1]
	for i in range(num_bodies):
		b = bodies[i]
		if not b.static:
			acc = b.force / b.mass - np.array([0, -1]) * gravity
			b.vel += acc * dt
			b.pos += b.vel * dt

			ang_acc = b.torque / b.rot_inertia
			b.ang_vel += ang_acc * dt
			b.rot += b.ang_vel * dt

			# Apply air drag
			b.vel -= clamp_magnitude(b.vel, 1) * drag * dt
			b.ang_vel -= clamp_magnitude(b.ang_vel, 1) * drag * dt

			b.force = [0.0, 0.0]
			b.torque = 0

			if b.pos[1] > floor_y:
				b.pos[1] = floor_y
				b.vel[1] = 0.0

		c, s = np.cos(b.rot), np.sin(b.rot)
		R = np.array([
			[c, -s, 0], 
			[s, c, 0],
			[0, 0, 1]])
		T = np.array([
			[1, 0, b.pos[0]], 
			[0, 1, b.pos[1]], 
			[0, 0, 1]]).astype(float)

		b.model_mat = T @ R
		b.inv_model_mat = np.linalg.inv(b.model_mat)

		draw_shape(b.model_mat, b.shape, thickness = 3)

	if data.mouse_line and data.mouse_start is not None and data.mouse is not None:
		g.line(data.mouse_start, data.mouse, color = 0xffe62b)
