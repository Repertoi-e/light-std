import lstdgraphics as g

import numpy as np 
import shape

import math

import cProfile, pstats

from collision import collide
from drawing import draw_shape
from shape import random_convex_polygon
from body import Body, ensure_transformed_shape, apply_force, apply_impulse, set_static
from hit import point_in_shape

from vec import clamp_magnitude, magnitude, sqr_magnitude, normalized, dot, dot_mat, orthogonal

import data_grabbing as data
import data_editor as editor

gravity = 9.8
drag = 0.7

bodies = []

bullet = None
timer = 0

def launch_bullet():
	global bullet
	bullet.pos = np.array([-10.0, -5.0])
	bullet.vel = np.array([0.0, 0.0])
	apply_impulse(bullet, np.array([80.0, 0.0]))

def load(state):
	"""
	Called from C++ side. Sets the state which "lstdgraphics" uses for drawing.
	"""
	g.state(state,
		editor_draw_aabb = True,
		editor_positional_correction = True,
		editor_debug_intersections = True,
		editor_iterations = True
	)

	global bullet
	circle = shape.Circle(0.2, 0xed37d8)
	bullet = Body(circle, 10)
	launch_bullet()
	bodies.append(bullet)

	rect = shape.make_rect(100, 0.2, 0x42f5d7)
	floor = Body(rect, 1, static = True)
	floor.pos = np.array([10.0, -10.0])
	bodies.append(floor)

	rect = shape.make_rect(0.2, 5, 0x42f5d7)
	thin_wall = Body(rect, 1, static = True)
	thin_wall.pos = np.array([13.0, -5.0])
	bodies.append(thin_wall)

	rect = shape.make_rect(10, 5, 0x42f5d7)
	thick_wall = Body(rect, 1, static = True)
	thick_wall.pos = np.array([27.0, -5.0])
	bodies.append(thick_wall)

def editor_variable(var, value): 
	s = '"' + value + '"' if isinstance(value, str) else str(value)
	exec("editor." + var + " = " + s)

def unload():
	"""
	Called when the script is unloaded
	"""
	bodies.clear()

def mouse_click(x, y, rightButton):
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

def cross_scalar(s, v):
	return [-s * v[1], s * v[0]]

def frame(dt):
	"""
	Called each frame from C++ side. Use "lstdgraphics" module to draw primitives.
	We also have some helper functions in "drawing.py" (which also use "lstdgraphics").
	"""

	global timer
	timer += dt

	if timer > 3:
		launch_bullet()
		timer = 0 

	#profile = cProfile.Profile()
	#profile.enable()
	
	allowed_penetration = 0.01
	position_bias_factor = 0.2 if editor.positional_correction else 0.0

	for i in range(len(bodies)):
		ib = bodies[i]

		# Ensure we have a cached a transformed shape
		ensure_transformed_shape(ib)

		# Draw body
		draw_shape(ib.transformed_shape, thickness = 3, aabb = ib.transformed_shape.aabb if editor.draw_aabb else None)

		if not ib.static:
			acc = ib.force * ib.inv_mass - np.array([0, 1]) * gravity
			ib.vel += acc * dt

			ang_acc = ib.torque * ib.inv_rot_inertia
			ib.ang_vel += ang_acc * dt

			# Apply air drag
			ib.vel -= clamp_magnitude(ib.vel, 1) * drag * dt
			ib.ang_vel -= clamp_magnitude(ib.ang_vel, 1) * drag * dt

		for j in range(i + 1, len(bodies)):
			jb = bodies[j]
			if ib.static and jb.static: continue

			a_index, b_index = i, j
			a, b = ib, jb
			if a_index > b_index:
				a_index, b_index = b_index, a_index
				a, b = b, a

			contacts = collide(a, b)

			# Precompute constant stuff for each iteration
			for c in contacts:
				c.r1 = c.pos - a.pos
				c.r2 = c.pos - b.pos

				rn1 = dot(c.r1, c.normal)
				rn2 = dot(c.r2, c.normal)
				k = a.inv_mass + b.inv_mass
				k += a.inv_rot_inertia * (sqr_magnitude(c.r1) - rn1 ** 2) + b.inv_rot_inertia * (sqr_magnitude(c.r2) - rn2 ** 2)
				c.inv_k = 1 / k
				
				c.bias = -position_bias_factor * 1 / dt * min(0, -c.collision_depth + allowed_penetration)
			
			# Perform iterations
			for _ in range(editor.iterations):
				for c in contacts:
					# Relative velocity at each contact
					rel_vel = b.vel + cross_scalar(b.ang_vel, c.r2) - a.vel - cross_scalar(a.ang_vel, c.r1)
					vel_along_normal = dot(rel_vel, c.normal)
					
					j = max(c.inv_k * (-vel_along_normal + c.bias), 0)
					impulse = j * c.normal

					apply_impulse(a, -impulse, c.r1)
					apply_impulse(b, impulse, c.r2)
		
		if not ib.static:
			ib.pos += ib.vel * dt
			ib.rot += ib.ang_vel * dt

			ib.force = np.array([0.0, 0.0])
			ib.torque = 0
			
			ib.dirty_transform = True

	if data.mouse_line and data.mouse_start is not None and data.mouse is not None:
		g.line(data.mouse_start, data.mouse, color = 0xffe62b)

	#profile.disable
	#ps = pstats.Stats(profile).sort_stats("cumulative")
	#ps.print_stats()
