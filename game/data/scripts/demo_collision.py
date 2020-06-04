import lstdgraphics as g

import numpy as np 
import shape

import math

import cProfile, pstats

from drawing import draw_shape
from shape import random_convex_polygon
from body import Body, ensure_transformed_shape, apply_force, apply_impulse, set_static
from hit import point_in_shape, aabb_vs_aabb, minimum_translation_vector, get_contact_points, get_average_contact_point

from vec import clamp_magnitude, magnitude, sqr_magnitude, normalized, dot, dot_mat, orthogonal

import data_grabbing as data
import data_editor as editor

gravity = 9.8
drag = 0.7

bodies = []

# We make a class because we add lots of dynamic properties and we want to have readable names!
class Contact:
	def __init__(self, pos):
		self.pos = pos

def collision_find_best_edge(vertices, mtv):
    """
    Returns the edge furthest along "mtv" and most perpendicular to "mtv" and the furthest vertex.
    """
    max_proj = float("-Inf")
    i_vertex, vertex = None, None
    for i, v in enumerate(vertices):
        proj = dot(v, mtv)
        if proj > max_proj:
            i_vertex = i
            vertex = v
            max_proj = proj

    r_vertex = vertices[(i_vertex + 1) % len(vertices)]
    l_vertex = vertices[(i_vertex - 1) % len(vertices)]

    # Both of these must point to "vertex"!
    r = normalized(vertex - r_vertex)
    l = normalized(vertex - l_vertex)

    if dot(r, mtv) < dot(l, mtv):
        # Right edge is better
        return [vertex, r_vertex], vertex
    else:
        # Left edge is better
        return [l_vertex, vertex], vertex

def dot_edge(edge, v):
	return dot(edge[1] - edge[0], v)

def collision_clip_segment(p1, p2, n, offset):
	"""
	Clips the segment's end points if they are past "offset" along "n".
	"""
	clipped = []
	d1 = dot(p1, n) 
	d2 = dot(p2, n)
	
	# If either point is past "offset" along "n" we keep it
	if d1 <= offset:
		clipped.append(p1)
	if d2 <= offset:
		clipped.append(p2)

	d1 -= offset
	d2 -= offset
	
    # Check if they are on opposing sides.
    # If you can visualize this in your head, if they are not on opposing sides, that means that
    # the segment is entirely outside of our view. In that case we return an empty array.
	if d1 * d2 < 0:
		e = p2 - p1
		# Compute the coeff along "e"
		u = d1 / (d1 - d2)
		# The new position is "u * e" but we add the original point in order to convert it into the proper space
		clipped.append(e * u + p1)
	return clipped

def collision_clip_points(ref, ref_max_v, inc):
    ref_v = normalized(ref[1] - ref[0])
        
    # Clip the incident edge by the first vertex of the reference edge
    o1 = -dot(ref_v, ref[0])
    clipped = collision_clip_segment(inc[0], inc[1], -ref_v, o1)

    # If we don't have two points then fail
    if len(clipped) != 2: 
        return []
    
    # Clip what's left on the incident edge by the second vertex of the reference edge
    o2 = dot(ref_v, ref[1])
    clipped = collision_clip_segment(clipped[0], clipped[1], ref_v, o2)

    # If we don't have two points then fail
    if len(clipped) != 2: 
        return []

    ref_normal = -orthogonal(ref_v)
    
    # Clip the final points against the reference edge
    ref_max_v_depth = dot(ref_normal, ref_max_v)
    
    c0_depth = dot(ref_normal, clipped[0]) - ref_max_v_depth
    c1_depth = dot(ref_normal, clipped[1]) - ref_max_v_depth
    
    contacts = [Contact(clipped[0]), Contact(clipped[1])]
    contacts[0].collision_depth = c0_depth
    contacts[1].collision_depth = c1_depth
    
    if c1_depth < 0:
        contacts.pop(1)
    if c0_depth < 0:
        contacts.pop(0)
    return contacts

def collide(a, b):
	"""
	Returns a list of contact points or empty if "a" and "b" are not colliding.
	"""
	if not aabb_vs_aabb(a.transformed_shape.aabb, b.transformed_shape.aabb):
		return []

	mtv = minimum_translation_vector(a.transformed_shape, b.transformed_shape)
	if mtv is not None:	  
		d = b.transformed_shape.centroid - a.transformed_shape.centroid
		if dot(d, mtv) < 0:
			mtv = -mtv
		
		normal = normalized(mtv)

		if editor.debug_intersections:
			g.line(b.transformed_shape.centroid, b.transformed_shape.centroid + mtv, color = 0xaa42f5, thickness = 3)
			g.circle_filled(b.transformed_shape.centroid + mtv, 0.1, color = 0xaa42f5, num_segments = 3)

		if a.transformed_shape.type == shape.Type.CIRCLE:
			furthest_point = a.transformed_shape.centroid + a.transformed_shape.radius * normal
			c = Contact(furthest_point)
			c.normal = normal
			c.collision_depth = magnitude(mtv)
			return [c]
		a_best_edge, a_max_v = collision_find_best_edge(a.transformed_shape.vertices, mtv)

		if b.transformed_shape.type == shape.Type.CIRCLE:
			furthest_point = b.transformed_shape.centroid + b.transformed_shape.radius * normal
			c = Contact(furthest_point)
			c.normal = normal
			c.collision_depth = magnitude(mtv)
			return [c]
		b_best_edge, b_max_v = collision_find_best_edge(b.transformed_shape.vertices, -mtv)
		
		if abs(dot_edge(a_best_edge, mtv)) < abs(dot_edge(b_best_edge, mtv)):
			ref = a_best_edge
			ref_max_v = a_max_v
			inc = b_best_edge
		else:
			ref = b_best_edge
			ref_max_v = b_max_v
			inc = a_best_edge

		if editor.debug_intersections:
			g.line(inc[0], inc[1], color = 0xf54254, thickness = 5)
			g.line(ref[0], ref[1], color = 0x6ff542, thickness = 5)
			g.circle_filled(ref_max_v, 0.1, color = 0x6ff542, num_segments = 3)

		contacts = collision_clip_points(ref, ref_max_v, inc)

		for c in contacts:
			c.normal = normal
		return contacts
	return []

def load(state):
	"""
	Called from C++ side. Sets the state which "lstdgraphics" uses for drawing.
	"""
	g.state(state,
		editor_spawn_shape_type = True,
		editor_draw_aabb = True,
		editor_positional_correction = True,
		editor_debug_intersections = True,
		editor_iterations = True
	)

	vertices = [
		[-1, -1],
		[1, -1],
		[0, 1]
	]
	poly = shape.ConvexPolygon(vertices, 0xed37d8)
	tri = Body(poly, 10)
	tri.pos = np.array([10.0, 0.0])
	bodies.append(tri)

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
	elif editor.shape_spawn_type == "circle":
		sh = shape.Circle(np.random.randint(10) * 0.05 + 1, np.random.randint(0x1000000))
	else:
		sh = shape.make_rect(np.random.randint(10) / 10 + 1, np.random.randint(10) / 10 + 1, np.random.randint(0x1000000))
	b = Body(sh, np.random.randint(10) + 5)
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
	if rightButton:
		return

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

	#if editor.show_contact_points:
	#	for c in contacts:
	#		g.circle_filled(c.pos, 0.1, num_segments = 3, color = 0x35fc03)

	if data.mouse_line and data.mouse_start is not None and data.mouse is not None:
		g.line(data.mouse_start, data.mouse, color = 0xffe62b)

	#profile.disable
	#ps = pstats.Stats(profile).sort_stats("cumulative")
	#ps.print_stats()
