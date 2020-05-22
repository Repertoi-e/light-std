import lstdgraphics as g
import numpy as np

def init():
	'''
	Called once when the application starts
	'''
	pass

def reload(state):
	'''
	Called when the script is reloaded
	'''
	g.state(state)

def frame():
	'''
	Called each frame from C++ side. Import 'lstdgraphics' module to draw primitives.
	'''
	pos = np.array([50, 50]);
	size = np.array([40, 40]);
	g.rect(pos, pos + size, color = 0xffff00ff);

