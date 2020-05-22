import numpy as np
import lstdgraphics as g

from vec import normalized

#
# This file implements basic shapes which our demo uses.
# As convention we consider the vertices of polygons to be ordered counter-clockwise (this affects calculations).
#

class Shape:
    def __init__(self, color):
        self.color = color
        self.recache_constants()

    def recache_constants(self):
        '''
        After changing shape properties, this method recalculates constants (e.g. center of mass)
        '''
        pass

class Circle(Shape):
    def __init__(self, radius, color = 0xffff00ff):
        self.radius = radius
        super().__init__(color)

    def recache_constants(self):
        self.centroid = np.array([0, 0]) # A circle's center of mass is its center :D

class ConvexPolygon(Shape):
    def __init__(self, vertices, color = 0xffff00ff):
        self.vertices = np.array(vertices)
        self.normals = np.empty((len(vertices), 2))
        super().__init__(color)

    def recache_constants(self):
        self.centroid = np.array([0, 0])
        
        # Save the previous vertex
        a = self.vertices[0]
        for i in range(1, len(self.vertices) + 1):
            b = self.vertices[i % len(self.vertices)] # At the end we cycle back to the first vertex

            n = [-(b[1] - a[1]), (b[0] - a[0])]
            n = normalized(n)

            self.normals[i - 1] = n

            a = b
