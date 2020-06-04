import lstdgraphics as g

import numpy as np 
import shape

import math

import cProfile, pstats

from drawing import draw_shape
from shape import random_convex_polygon
from body import Body, ensure_transformed_shape, apply_force, apply_impulse, set_static
from hit import point_in_shape, aabb_vs_aabb, minimum_translation_vector, get_contact_points, get_average_contact_point

from vec import clamp_magnitude, magnitude, sqr_magnitude, normalized, dot_mat

import data_grabbing as data
import data_editor as editor

gravity = 9.8
drag = 0.7

bodies = []
triangle, floor = None, None

def load(state):
	"""
	Called from C++ side. Sets the state which "lstdgraphics" uses for drawing.
	"""
	g.state(state,
		editor_spawn_shape_type = True,
		editor_impulse_resolution = True,
		editor_calculate_contact_points = True,
		editor_show_contact_points = True
	)

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

def editor_variable(var, value): 
	s = '"' + value + '"' if isinstance(value, str) else str(value)
	exec("editor." + var + " = " + s)

def unload():
	"""
	Called when the script is unloaded
	"""
	bodies.clear()

def spawn_shape(pos):
	sh = None
	if editor.shape_spawn_type == "polygon":
		vs = random_convex_polygon(np.random.randint(3, 8)) * 5
		sh = shape.ConvexPolygon(vs, np.random.randint(0x1000000))
	else:
		sh = shape.Circle(np.random.randint(10) * 0.05 + 1)
	b = Body(sh, 10)
	b.pos = pos
	bodies.append(b)

def mouse_click(x, y, rightButton):
	if rightButton:
		spawn_shape(np.copy(data.mouse))
		return

	if not data.mouse_line:
		data.mouse_line = True
		data.mouse_start = np.array([x, y])

	for b in bodies:
		mouse_in_local_space = dot_mat(b.inv_model_mat, data.mouse)

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
			acc = b.force * b.inv_mass - np.array([0, 1]) * gravity
			b.vel += acc * dt
			b.pos += b.vel * dt

			ang_acc = b.torque * b.inv_rot_inertia
			b.ang_vel += ang_acc * dt
			b.rot += b.ang_vel * dt

			# Apply air drag
			b.vel -= clamp_magnitude(b.vel, 1) * drag * dt
			b.ang_vel -= clamp_magnitude(b.ang_vel, 1) * drag * dt

			b.force = np.array([0.0, 0.0])
			b.torque = 0

			b.dirty_transform = True
		# Ensure we have a cached a transformed shape
		ensure_transformed_shape(b)
							
		b.contacts = None
	
	for i in range(len(bodies)):
		a = bodies[i]
		for j in range(i + 1, len(bodies)):
			b = bodies[j]

			# Broad phase (cheaper to calculate)
			if not aabb_vs_aabb(a.transformed_shape.aabb, b.transformed_shape.aabb):
				continue

			# Narrow phase
			mtv = minimum_translation_vector(a.transformed_shape, b.transformed_shape)
			if mtv is not None:
				mta, overlap = mtv
				
				d = b.transformed_shape.centroid - a.transformed_shape.centroid
				if dot(d, mta) < 0:
					mta = -mta

				normal = normalized(mta)
				# There are rare cases where the collision normal is zero,
				# because normalized() returns [0, 0] if the input vector 
				# has near zero length (to avoid division by zero).
				if math.isclose(sqr_magnitude(normal), 0):
					continue

				if editor.impulse_resolution:					
					rv = b.vel - a.vel
					vel_along_normal = dot(rv, normal)

					j = vel_along_normal / (a.inv_mass + b.inv_mass)
					impulse = j * normal

					mass_sum = a.mass + b.mass
					impulse_a = (a.mass / mass_sum) * impulse
					impulse_b = -(b.mass / mass_sum) * impulse

					contacts_a = [a.transformed_shape.centroid]
					contacts_b = [b.transformed_shape.centroid]
					if editor.calculate_contact_points:
						contacts_a = get_contact_points(a.transformed_shape, mta)
						contacts_b = get_contact_points(b.transformed_shape, -mta)
					a.contacts = contacts_a
					b.contacts = contacts_b

					for c in contacts_a:
						apply_impulse(a, impulse_a / len(contacts_a), point = c - a.transformed_shape.centroid)
					for c in contacts_b:
						apply_impulse(b, impulse_b / len(contacts_b), point = c - b.transformed_shape.centroid)

					if editor.positional_correction:
						correction = overlap / (a.inv_mass + b.inv_mass) * 0.2 * normal
						a.pos -= a.inv_mass * correction
						b.pos += b.inv_mass * correction
				else:
					t = overlap / (a.inv_mass + b.inv_mass) * normal
					a.pos += a.inv_mass * t
					b.pos -= b.inv_mass * t

	for b in bodies:
		draw_shape(b.transformed_shape, thickness = 3)
		if editor.show_contact_points and b.contacts is not None:
			for c in b.contacts:
				g.circle_filled(c, 0.1, num_segments = 3, color = 0x35fc03)
		#draw_shape(b.transformed_shape, thickness = 3, aabb = b.transformed_shape.aabb)

	if data.mouse_line and data.mouse_start is not None and data.mouse is not None:
		g.line(data.mouse_start, data.mouse, color = 0xffe62b)

	#profile.disable
	#ps = pstats.Stats(profile).sort_stats("cumulative")
	#ps.print_stats()
