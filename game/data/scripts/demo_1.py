import lstdgraphics as g
import numpy as np 

from drawing import draw_shape 
import shape

from constants import *

from body import Body

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
	bodies.append(b)

	rect = shape.make_rect(2, 1, 0xff00ffff)
	b = Body(rect, 1, static = True)
	b.pos[0] = 2.5
	b.pos[1] = 2.5
	bodies.append(b)

def unload():
	bodies.clear()

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

		draw_shape(b.pos, b.shape, normals_color = 0xff0000ff, thickness = 3)


