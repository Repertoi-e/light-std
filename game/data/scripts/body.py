import numpy as np
import shape as sh

from vec import sqr_magnitude

from constants import *

class Body:
    def __init__(self, shape, density, static = False):
        self.pos = np.array([0, 0]).astype(float)
        self.rot = 0.0
        
        self.vel = np.array([0, 0]).astype(float)
        self.ang_vel  = 0.0

        self.model_mat = np.identity(3)
        self.inv_model_mat = np.identity(3)

        self.force = np.array([0, 0]).astype(float)
        self.torque = 0.0

        self.shape = shape
        self.static = static

        if isinstance(shape, sh.Circle):
            self.AABB = np.array([[-shape.radius, -shape.radius], [shape.radius, shape.radius]])
        elif isinstance(shape, sh.ConvexPolygon):
            x = shape.vertices[:, 0]
            y = shape.vertices[:, 1]
            mx = np.min(x)
            nx = np.max(x)
            my = np.min(y)
            ny = np.max(y)
            self.AABB = np.array([[mx, my], [nx, ny]])

        if not static:
            self.mass = density * shape.area

            if isinstance(shape, sh.Circle):
                self.rot_inertia = 0.5 * self.mass * (shape.radius ** 2)
            elif isinstance(shape, sh.ConvexPolygon):
                self.rot_inertia = 0
                for e in shape.edges:
                    a, b = e[0], e[1]
                    tri_mass = density * 0.5 * np.abs(np.cross(a, b))
                    tri_inertia = tri_mass * (sqr_magnitude(a) + sqr_magnitude(b) + np.dot(a, b)) / 6
                    self.rot_inertia += tri_inertia
        else:
            self.mass = float('Inf')
            self.rot_inertia = float('Inf')


def apply_force(body, force, point = [0.0, 0.0]):
    if not body.static: 
        body.force += force
        body.torque += np.cross(point, force)

def apply_impulse(body, impulse, point = [0.0, 0.0]):
    if not body.static: 
        body.vel += impulse * (1 / body.mass)
        body.ang_vel += np.cross(point, impulse) * (1 / body.rot_inertia)

def set_static(body, static):
    if body.static == static: return
    if not body.static:
        body.vel = [0.0, 0.0]
        body.ang_vel = 0.0
        body.force = [0.0, 0.0]
        body.torque = 0.0
    body.static = static