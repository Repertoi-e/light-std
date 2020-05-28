import lstdgraphics as g

import numpy as np

from vec import normalized

from constants import *

#
# This file implements basic shapes which our demo uses.
# As convention we consider the vertices of polygons to be ordered clockwise (this affects calculations).
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
    return abs(area)

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
            n = [-(b[1] - a[1]), (b[0] - a[0])]
            self.normals[i] = normalized(n)

        self.color = color


# Returns a centered rectangle with half_width, half_height extents
def make_rect(half_width, half_height, color = 0x4254f5):
    vertices = [
        [-half_width, -half_height],
        [-half_width, half_height],
        [half_width, half_height],
        [half_width, -half_height]
    ]
    return ConvexPolygon(vertices, color = color)