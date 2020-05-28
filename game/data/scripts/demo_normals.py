import lstdgraphics as g

import numpy as np 
import shape

from body import Body
from drawing import draw_shape 

import constants

bodies = list()

# Called from C++ side. Sets the state which 'lstdgraphics' uses for drawing.
def load(state):
	g.state(state)

	vertices = [
		[1, -1],
		[-1, -1],
		[0, 1]
	]
	poly = shape.ConvexPolygon(vertices, 0xed37d8)
	b = Body(poly, 10)
	b.pos += [4, 4]
	bodies.append(b)

	rect = shape.make_rect(2, 1, 0x42f5d7)
	b = Body(rect, 1)
	b.pos += [8, 4]
	bodies.append(b)

	circle = shape.Circle(1.3, 0x61eb34)
	b = Body(circle, 1)
	b.pos += [13, 4]
	bodies.append(b)
	
	constants.gravity = 0

def unload():
	'''
	Called when the script is unloaded
	'''
	pass

def frame(dt):
	'''
	Called each frame from C++ side. Use 'lstdgraphics' module to draw primitives.
	We also have some helper functions in 'drawing.py' (which also use 'lstdgraphics').
	'''
	global bodies

	for b in bodies:
		if not b.static:
			acc = b.force / b.mass - np.array([0, -1]) * constants.gravity
			b.vel += acc * dt
			b.pos += b.vel * dt

			ang_acc = b.torque / b.rot_inertia
			b.ang_vel += ang_acc * dt
			b.rot += b.ang_vel * dt

			b.force = np.array([0, 0]).astype(float)
			b.torque = 0.0

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

		draw_shape(b.model_mat, b.shape, normals_color = 0xff0000, thickness = 3)
		


