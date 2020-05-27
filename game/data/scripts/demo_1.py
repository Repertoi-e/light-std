import lstdgraphics as g
import numpy as np 

from drawing import draw_shape, t_vert
import shape

from constants import *

from body import Body

from hit import point_in_aabb

bodies = list()

# Called from C++ side. Sets the state which 'lstdgraphics' uses for drawing.
def load(state):
	g.state(state)

	vertices = [
		[1, -1],
		[-1, -1],
		[0, 1]
	]
	poly = shape.ConvexPolygon(vertices, 0xffff00ff)
	b = Body(poly, 10, static = True)
	b.pos = [10, 0]
	bodies.append(b)

	rect = shape.make_rect(6, 1, 0xff00ffff)
	b = Body(rect, 1, static = True)
	b.pos = [10, 10]
	bodies.append(b)

def unload():
	bodies.clear()

test_line = False
mouse_pos_start = None
mouse_pos = None

def mouse_click(x, y, rightButton):
	global test_line, mouse_pos_start
	if not test_line and not rightButton:
		test_line = True
		mouse_pos_start = [x, y]
	
	global bodies
	for b in bodies:
		p = t_vert(b.inv_model_mat, np.array([x, y]) / pixels_per_meter)
		if point_in_aabb(p, b.AABB):
			print("Hit!")

def mouse_release(rightButton):
	global test_line, mouse_pos_start
	if test_line and not rightButton:
		test_line = False

def mouse_move(x, y):
	global mouse_pos
	mouse_pos = [x, y]

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
			[0, 0, 1]]).astype(float)
		T = np.array([
			[1, 0, b.pos[0]], 
			[0, 1, b.pos[1]], 
			[0, 0, 1]]).astype(float)

		b.model_mat = np.matmul(T, R)
		b.inv_model_mat = np.linalg.inv(b.model_mat)

		draw_shape(b.model_mat, b.shape, thickness = 3)

	global test_line, mouse_pos, mouse_pos_start

	if test_line:
		g.line(mouse_pos_start, mouse_pos, color = 0xffd8eb34)

