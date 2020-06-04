import lstdgraphics as g
import numpy as np

import math
import shape

from vec import cross, dot, magnitude, sqr_magnitude, normalized, orthogonal
from hit import point_in_shape, aabb_vs_aabb, minimum_translation_vector

import data_editor as editor

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

		if math.isclose(sqr_magnitude(mtv), 0):
			return

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
