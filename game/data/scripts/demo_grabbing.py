import lstdgraphics as g

import numpy as np 
import shape

from drawing import draw_shape, dot
from body import Body, apply_force, set_static
from hit import point_in_aabb

import data_demo_grabbing as data

gravity = 9.8

bodies = list()
triangle, floor = None, None

def load(state):
	'''
	Called from C++ side. Sets the state which 'lstdgraphics' uses for drawing.
	'''
	g.state(state)

	global triangle, floor
	vertices = [
		[1, -1],
		[-1, -1],
		[0, 1]
	]
	poly = shape.ConvexPolygon(vertices, 0xed37d8)
	triangle = Body(poly, 10)
	triangle.pos = [10, 0]
	bodies.append(triangle)

	rect = shape.make_rect(100, 0.2, 0x42f5d7)
	floor = Body(rect, 1, static = True)
	floor.pos = [10, 10]
	bodies.append(floor)

def unload():
	'''
	Called when the script is unloaded
	'''
	pass

def mouse_click(x, y, rightButton):
	if rightButton: return

	if not data.mouse_line:
		data.mouse_line = True
		data.mouse_start = np.array([x, y])

	for b in bodies:
		mouse_in_local_space = dot(b.inv_model_mat, data.mouse)

		if point_in_aabb(mouse_in_local_space, b.AABB):
			data.grabbed = b

			c, s = np.cos(b.rot), np.sin(b.rot)
			R = np.array([
				[c, -s, 0], 
				[s,  c, 0],
				[0,  0, 1]])
			data.grabbed_offset = dot(R, mouse_in_local_space)

			data.grabbed_was_static = b.static
			
			# We null all forces and stop moving the object while grabbing it
			set_static(b, True)
			break

def mouse_release(rightButton):
	if rightButton: return

	if data.mouse_line:
		data.mouse_line = False

	if data.grabbed is not None:
		if not data.grabbed_was_static:
			set_static(data.grabbed, False)
		
		F = (data.mouse - np.array(data.mouse_last)) * 5000
		
		apply_force(data.grabbed, F, data.grabbed_offset)
		data.grabbed = None


def mouse_move(x, y):
	data.mouse_last = data.mouse
	data.mouse = np.array([x, y])
	if data.grabbed is not None:
		data.grabbed.pos = data.mouse - data.grabbed_offset

def frame(dt):
	'''
	Called each frame from C++ side. Use 'lstdgraphics' module to draw primitives.
	We also have some helper functions in 'drawing.py' (which also use 'lstdgraphics').
	'''
	global bodies

	for b in bodies:
		if not b.static:
			acc = b.force / b.mass - np.array([0, -1]) * gravity
			b.vel += acc * dt
			b.pos += b.vel * dt

			ang_acc = b.torque / b.rot_inertia
			b.ang_vel += ang_acc * dt
			b.rot += b.ang_vel * dt

			b.force = [0.0, 0.0]
			b.torque = 0

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

	if data.mouse_line:
		g.line(data.mouse_start, data.mouse, color = 0xffe62b)

	floor_upper_edge = floor.pos[1] + floor.AABB[0][1]
	if triangle.pos[1] > floor_upper_edge:
		triangle.pos[1] = floor_upper_edge
		triangle.vel[1] = 0

