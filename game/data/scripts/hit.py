import numpy as np 

from shape import ConvexPolygon, Circle, edges

from vec import sqr_magnitude

# Returns the point at which two lines intersect (or None if they don't)
def line_vs_line(line1, line2):
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

# Checks if c is between a and b
def is_between(a, b, c):
    if a > b: 
        a, b = b, a 
    return a < c < b

# Checks if the point k lies on the extension of the ray (the part of the line that isn't on top of the ray)
def is_point_on_ray_extension(ray, k):
    b, d = ray[0], ray[0] + ray[1]
    return is_between(k[0], d[0], b[0]) or is_between(k[1], d[1], b[1])

# Returns the point at which a ray and a segment intersect (or None if they don't).
# This doesn't pay attention to the ends of the segment.
def ray_vs_segment(ray, segment):
    b, d = ray[0], ray[1]
    k = line_vs_line([b, b + d], segment)

    if k is None: return None
    if is_point_on_ray_extension(ray, k): 
        return None
    
    a, b = segment[0], segment[1]
    m = min(a[0], b[0])
    n = max(a[0], b[0])
    if np.less(k[0], m) or np.isclose(k[0], m) or np.greater(k[0], n) or np.isclose(k[0], n):
        return None
    m = min(a[1], b[1])
    n = max(a[1], b[1])
    if np.less(k[1], m) or np.isclose(k[1], m) or np.greater(k[1], n) or np.isclose(k[1], n):
        return None
    return k


# Returns whether a point is inside a given circle (p is in local space)
def point_in_circle(p, center, radius):
    v = np.array(p).astype(float) - center
    return sqr_magnitude(v) < radius ** 2

# Returns whether a point is inside a given convex polygon (p is in local space)
def point_in_polygon(p, vertices, es = None):
    p = np.array(p).astype(float)

    if es is None: es = edges(vertices)
    u = es[:, 0] - p
    v = es[:, 1] - p
    return np.greater(np.cross(u, v), 0).all()

# Returns whether a point is inside a given AABB (p is in local space)
def point_in_aabb(p, aabb):
    p = np.array(p).astype(float)
    return np.greater(p, aabb[0]).all() and np.less(p, aabb[1]).all()

def point_in_shape(p, shape):
    if isinstance(shape, Circle):
        return point_in_circle(p, [0, 0], shape.radius)
    elif isinstance(shape, ConvexPolygon):
        return point_in_polygon(p, None, shape.edges)