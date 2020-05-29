import numpy as np 
import shape as sh

import math

from shape import ConvexPolygon, Circle, edges
from vec import magnitude, sqr_magnitude, normalized, orthogonal

def line_vs_line(line1, line2):
    """
    Returns the point at which two lines intersect (or None if they don't)
    """
    p1 = np.array(line1[0]).astype(float)
    p2 = np.array(line1[1]).astype(float)

    w1 = np.array(line2[0]).astype(float)
    w2 = np.array(line2[1]).astype(float)
    
    xdiff = (p1[0] - p2[0], w1[0] - w2[0])
    ydiff = (p1[1] - p2[1], w1[1] - w2[1])

    div = np.linalg.det([xdiff, ydiff])
    if div == 0: return None
    
    d = (np.linalg.det(line1), np.linalg.det(line2))
    x = np.linalg.det([d, xdiff]) / div
    y = np.linalg.det([d, ydiff]) / div
    return np.array([x, y])

def is_between(a, b, c):
    """
    Checks if c is between a and b
    """
    if a > b: 
        a, b = b, a 
    return a < c < b

def is_point_on_ray_extension(ray, k):
    """
    Checks if the point k lies on the extension of the ray (the part of the line that isn't on top of the ray)
    """
    b, d = ray[0], ray[0] + ray[1]
    return is_between(k[0], d[0], b[0]) or is_between(k[1], d[1], b[1])

def ray_vs_segment(ray, segment):
    """
    Returns the point at which a ray and a segment intersect (or None if they don't).
    This doesn't pay attention to the ends of the segment.
    """
    b, d = ray[0], ray[1]
    k = line_vs_line([b, b + d], segment)

    if k is None: return None
    if is_point_on_ray_extension(ray, k): 
        return None
    
    a, b = segment[0], segment[1]
    m = min(a[0], b[0])
    n = max(a[0], b[0])
    if k[0] < m or math.isclose(k[0], m) or k[0] > n or math.isclose(k[0], n):
        return None
    m = min(a[1], b[1])
    n = max(a[1], b[1])
    if k[1] < m or math.isclose(k[1], m) or k[1] > n or math.isclose(k[1], n):
        return None
    return k


def point_in_circle(p, center, radius):
    """
    Returns whether a point is inside a given circle (p is in local space)
    """
    v = np.array(p).astype(float) - center
    return sqr_magnitude(v) < radius ** 2

def point_in_polygon(p, vertices, es = None):
    """
    Returns whether a point is inside a given convex polygon (p is in local space)
    """
    p = np.array(p).astype(float)

    if es is None: es = edges(vertices)
    u = es[:, 0] - p
    v = es[:, 1] - p
    return np.greater(np.cross(u, v), 0).all()

def point_in_aabb(p, aabb):
    """
    Returns whether a point is inside a given AABB (p is in local space)
    """
    p = np.array(p).astype(float)

    b1 = abs(aabb[0][0] - p[0]) <= aabb[1][0]
    b2 = abs(aabb[0][1] - p[1]) <= aabb[1][1]
    return b1 and b2

def aabb_vs_aabb(aabb1, aabb2):
    """
    Returns whether two AABBs are colliding.
    Each AABB is represented by a center and half-width/height.  
    """
    c1, c2 = aabb1[0], aabb2[0]
    e1, e2 = aabb1[1], aabb2[1]

    b1 = abs(c1[0] - c2[0]) <= (e1[0] + e2[0])
    b2 = abs(c1[1] - c2[1]) <= (e1[1] + e2[1])
    return b1 and b2

def point_in_shape(p, shape):
    """
    Generalized version of point_in_... for shapes
    """
    if not point_in_aabb(p, shape.aabb):
        return None
    if shape.type == sh.Type.CIRCLE:
        return point_in_circle(p, [0, 0], shape.radius)
    else:
        return point_in_polygon(p, None, shape.edges)

def is_separating_axis_polygons(axis, v1, v2):
    """
    Returns True and None if "axis" is a separating axis of v1 and v2 (v1 and v2 are vertices).
    Return False and the the push vector otherwise.
    """
    m1, n1 = float("Inf"), float("-Inf")
    m2, n2 = float("Inf"), float("-Inf")

    for v in v1:
        proj = np.dot(v, axis)
        m1 = min(m1, proj)
        n1 = max(n1, proj)

    for v in v2:
        proj = np.dot(v, axis)
        m2 = min(m2, proj)
        n2 = max(n2, proj)

    if n1 >= m2 and n2 >= m1:
        d = min(n2 - m1, n1 - m2)

        # Push a bit more than needed so the shapes do not overlap in future tests due to float precision
        d_over_axis_squared = d / np.dot(axis, axis) + 1e-10
        return False, d_over_axis_squared * axis
    else:
        return True, None

def is_separating_axis_polygon_and_circle(axis, v1, center, radius):
    """
    Returns True and None if "axis" is a separating axis of v1 and a circle (v1 are vertices).
    Return False and the the push vector otherwise.
    """
    m1, n1 = float("Inf"), float("-Inf")

    for v in v1:
        proj = np.dot(v, axis)
        m1 = min(m1, proj)
        n1 = max(n1, proj)

    proj = np.dot(center, axis)
    m2 = proj - radius
    n2 = proj + radius

    if n1 >= m2 and n2 >= m1:
        d = min(n2 - m1, n1 - m2)

        # Push a bit more than needed so the shapes do not overlap in future tests due to float precision
        d_over_axis_squared = d / np.dot(axis, axis) + 1e-10
        return False, d_over_axis_squared * axis
    else:
        return True, None

def polygon_vs_polygon_push_vector(polygon_a, polygon_b):
    """
    Returns the minimum push vector of two polygons (None if they don't intersect).
    "polygon_a" and "polygon_b" must be instances of ConvexPolygon.
    """
    axes = np.concatenate((polygon_a.normals, polygon_b.normals), axis = 0)

    push_vectors = []
    for axis in axes:
        separating, pv = is_separating_axis_polygons(axis, polygon_a.vertices, polygon_b.vertices)
        if separating:
            return None # The polygons don't overlap if there exists at least one axis of separation
        push_vectors.append(pv)

    mpv = min(push_vectors, key = (lambda v: sqr_magnitude(v)))

    d = polygon_b.centroid - polygon_a.centroid
    if np.dot(d, mpv) > 0:
        mpv = -mpv
    return -mpv

def polygon_vs_circle_push_vector(polygon, circle):
    """
    Return the minimum push vector of a polygon and a circle (None if they don't intersect).
    "polygon" and "circle" must be instances of ConvexPolygon and Circle respectively.
    """
    closest, closest_dist = None, float("Inf")
    for v in polygon.vertices:
        u = v - circle.centroid
        d = sqr_magnitude(u)
        if d < closest_dist:
            closest = u
            closest_dist = d
    axes = np.concatenate((polygon.normals, [normalized(closest)]), axis = 0)

    push_vectors = []
    for axis in axes:
        separating, pv = is_separating_axis_polygon_and_circle(axis, polygon.vertices, circle.centroid, circle.radius)
        if separating:
            return None # The polygon and circle don't overlap if there exists at least one axis of separation
        push_vectors.append(pv)

    mpv = min(push_vectors, key = (lambda v: sqr_magnitude(v)))

    d = circle.centroid - polygon.centroid
    if np.dot(d, mpv) > 0:
        mpv = -mpv
    return -mpv

def push_vector(shape_a, shape_b):
    """
    Return the minimum push vector of a two shapes. (None if they don't intersect).
    """
    a_circle, b_circle = shape_a.type == sh.Type.CIRCLE, shape_b.type == sh.Type.CIRCLE
    
    if a_circle and b_circle:
        a_to_b = shape_b.centroid - shape_a.centroid
        r = (shape_a.radius + shape_b.radius)
        if sqr_magnitude(a_to_b) > r ** 2:
            return None
        d = magnitude(a_to_b)
        if d != 0:
            return a_to_b / d * (r - d)
        else:
            return np.array([1.0, 0.0]) * max(shape_a.radius, shape_b.radius)
    swap = False
    if a_circle != b_circle:
        if a_circle: 
            shape_a, shape_b = shape_b, shape_a
            swap = True

        result = polygon_vs_circle_push_vector(shape_a, shape_b)
        if result is not None and swap: result = -result
        return result

    return polygon_vs_polygon_push_vector(shape_a, shape_b)
