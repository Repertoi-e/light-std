import lstdgraphics as g
import numpy as np 

from drawing import draw_shape, t_vert
import shape

from constants import *

from body import Body, apply_force, set_static

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
	b = Body(poly, 10)
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
mouse_pos, mouse_pos_last = None, None
grabbed_body = None
body_offset = None
body_rot = None
was_static = False

def mouse_click(x, y, rightButton):
	if rightButton: return

	global test_line, mouse_pos_start
	if not test_line and not rightButton:
		test_line = True
		mouse_pos_start = [x, y]
	
	global body_offset, grabbed_body, was_static, body_rot
	for b in bodies:
		p = t_vert(b.inv_model_mat, np.array([x, y]) / pixels_per_meter)
		if point_in_aabb(p, b.AABB):
			body_offset = p
			c, s = np.cos(b.rot), np.sin(b.rot)
			body_rot = np.array([
				[c, -s, 0], 
				[s, c, 0],
				[0, 0, 1]]).astype(float)
			grabbed_body = b
			was_static = b.static
			set_static(b, True)
			break

def mouse_release(rightButton):
	if rightButton: return

	global test_line, mouse_pos_start
	if test_line and not rightButton:
		test_line = False

	global grabbed_body
	if grabbed_body:
		if not was_static:
			set_static(grabbed_body, False)
		F = (np.array(mouse_pos_last) - mouse_pos) * 100
		apply_force(grabbed_body, -F, body_offset)
		grabbed_body = None


def mouse_move(x, y):
	global mouse_pos, mouse_pos_last
	mouse_pos_last = mouse_pos
	mouse_pos = [x, y]
	if grabbed_body:
		grabbed_body.pos = np.array([x, y]) / pixels_per_meter - t_vert(body_rot, body_offset)

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

