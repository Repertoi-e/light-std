import lstdgraphics as g

import numpy as np

from vec import normalized

#
# This file implements basic shapes which our demo uses.
# As convention we consider the vertices of polygons to be ordered counter-clockwise (this affects calculations).
#

class Shape:
    def __init__(self): pass

class Circle(Shape):
    def __init__(self, radius, color = 0x4254f5):
        self.radius = float(radius)
        self.area = radius ** 2 * np.pi
        self.color = color

def edges(vertices):
    result = np.empty((len(vertices), 2, 2))
    a = vertices[0]
    for i in range(1, len(vertices) + 1):
        b = vertices[i % len(vertices)] # At the end we cycle back to the first vertex
        result[i - 1] = np.array([a, b])
        a = b
    return result

def shoelace(es):
    area = 0
    for e in es:
        area += 0.5 * np.cross(e[0], e[1])
    return area

def centroid(es):
    result = np.array([0.0, 0.0])
    
    area_sum = 0.0
    for e in es:
        tri_area = 0.5 * np.cross(e[0], e[1])
        area_sum += tri_area
        result += (e[0] + e[1]) / 3 * tri_area
    return result / area_sum

class ConvexPolygon(Shape):
    def __init__(self, vertices, color = 0x4254f5):
        self.vertices = np.array(vertices).astype(float)
        self.normals = np.empty((len(vertices), 2)).astype(float)
        self.edges = np.empty((len(vertices), 2, 2)).astype(float)

        es = edges(self.vertices)

        self.area = shoelace(es)
        self.vertices -= centroid(es)

        # Recalculate edges after translating the vertices
        self.edges = edges(self.vertices)

        for i, e in enumerate(self.edges):
            a, b = e[0], e[1]
            n = [(b[1] - a[1]), -(b[0] - a[0])]
            self.normals[i] = normalized(n)

        self.color = color

def random_convex_polygon(n):
    x = np.sort(np.random.uniform(0, 1, n))
    y = np.sort(np.random.uniform(0, 1, n))
    min_x, max_x = x[0], x[-1]
    min_y, max_y = y[0], y[-1]

    def chains(min_a, max_a, a):
        r = np.array([])
        last_top, last_bot = min_a, min_a
        for i in range(1, len(a) - 1):
            t = a[i]
            if np.random.random_sample() < 0.5:
                r = np.append(r, t - last_top)
                last_top = t
            else:
                r = np.append(r, last_bot - t)
                last_bot = t
        r = np.append(r, max_a - last_top)
        r = np.append(r, last_bot - max_a)
        return r

    x_vec = chains(min_x, max_x, x)
    y_vec = chains(min_y, max_y, y)
    np.random.shuffle(y_vec)

    vec = list(zip(x_vec, y_vec))
    vec.sort(key = lambda v: np.arctan2(v[1], v[0]))

    vertex = np.array([0.0, 0.0])
    m = np.array([0.0, 0.0])
    vertices = []
    for v in vec:
        vertices.append(vertex)
        vertex = vertex + v
        m = np.minimum(m, vertex)
    return np.array(vertices) - (np.array([min_x, min_y]) - m)

# Returns a centered rectangle with half_width, half_height extents
def make_rect(half_width, half_height, color = 0x4254f5):
    vertices = [
        [-half_width, -half_height],
        [half_width, -half_height],
        [half_width, half_height],
        [-half_width, half_height]
    ]
    return ConvexPolygon(vertices, color = color)