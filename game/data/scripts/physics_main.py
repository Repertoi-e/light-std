import lstdgraphics as g
import numpy as np 

from drawing import draw_shape 
import shape

poly = None

def reload(state):
	'''
	Called when the script is reloaded
	'''
	g.state(state)

	global poly

	vertices = [
		[-20, -20],
		[0, 20],
		[20, -20]
	]
	poly = shape.ConvexPolygon(vertices, 0xffff00ff)


def frame():
	'''
	Called each frame from C++ side. Import 'lstdgraphics' module to draw primitives.
	'''
	global poly

	pos = np.array([50, 50])
	draw_shape(pos, poly, normals_color = 0xffffffff)

