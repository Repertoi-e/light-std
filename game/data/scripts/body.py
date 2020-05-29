import numpy as np
import shape as sh

import copy

from shape import edges
from vec import sqr_magnitude, dot, normalized

class Body:
    def __init__(self, shape, density, restitution = 0, static = False):
        self.pos = np.array([0, 0]).astype(float)
        self.rot = 0.0
        
        self.vel = np.array([0, 0]).astype(float)
        self.ang_vel  = 0.0

        self.dirty_transform = True
        self.model_mat = np.identity(3)
        self.inv_model_mat = np.identity(3)

        self.force = np.array([0, 0]).astype(float)
        self.torque = 0.0

        self.shape = shape
        self.transformed_shape = copy.deepcopy(shape)

        if not static:
            self.mass = density * shape.area
            self.inv_mass = 1 / self.mass

            if shape.type == sh.Type.CIRCLE:
                self.rot_inertia = 0.5 * self.mass * (shape.radius ** 2)
                self.inv_rot_inertia = 1 / self.rot_inertia
            else:
                self.rot_inertia = 0
                for e in shape.edges:
                    a, b = e[0], e[1]
                    tri_mass = density * 0.5 * np.abs(np.cross(a, b))
                    tri_inertia = tri_mass * (sqr_magnitude(a) + sqr_magnitude(b) + np.dot(a, b)) / 6
                    self.rot_inertia += tri_inertia
                    self.inv_rot_inertia = 1 / self.rot_inertia
        else:
            self.mass = 0.0
            self.inv_mass = 0.0
            self.rot_inertia = 0.0
            self.inv_rot_inertia = 0.0

        self.restitution = restitution
        self.static = static

def calculate_body_model_mat(body):
    c, s = np.cos(body.rot), np.sin(body.rot)
    R = np.array([
        [c, -s, 0], 
        [s, c, 0],
        [0, 0, 1]])
    T = np.array([
        [1, 0, body.pos[0]], 
        [0, 1, body.pos[1]], 
        [0, 0, 1]]).astype(float)
    body.model_mat = T @ R
    body.inv_model_mat = np.linalg.inv(body.model_mat)

def ensure_transformed_shape(body):
    if body.dirty_transform:
        calculate_body_model_mat(body)
        t = body.transformed_shape
        if body.shape.type == sh.Type.CIRCLE:
            t.centroid = body.pos
            t.aabb = np.array([t.centroid, [t.radius, t.radius]])
        else:
            transformed = []

            mx, nx = float("Inf"), float("-Inf")
            my, ny = float("Inf"), float("-Inf")
            for v in body.shape.vertices:
                d = dot(body.model_mat, v)
                mx = min(d[0], mx)
                my = min(d[1], my)
                nx = max(d[0], nx)
                ny = max(d[1], ny)
                transformed.append(d)
            t.vertices = transformed
            t.edges = edges(transformed)
            for i, e in enumerate(t.edges):
                a, b = e[0], e[1]
                n = [(b[1] - a[1]), -(b[0] - a[0])]
                t.normals[i] = normalized(n)

            t.centroid = dot(body.model_mat, [0, 0])

            hx = (nx - mx) / 2
            hy = (ny - my) / 2
            t.aabb = np.array([[mx + hx, my + hy], [abs(hx), abs(hy)]])
        body.dirty_transform = False

def apply_force(body, force, point = [0.0, 0.0]):
    body.force += force
    body.torque += np.cross(point, force)

def apply_impulse(body, impulse, point = [0.0, 0.0]):
    """
    Apply impulse to a body by taking into account it's mass
    """
    body.vel += impulse * body.inv_mass
    body.ang_vel += np.cross(point, impulse) * body.inv_rot_inertia

def set_static(body, static):
    if body.static == static: return
    if not body.static:
        body.vel = np.array([0, 0]).astype(float)
        body.ang_vel = 0.0
        body.force = np.array([0, 0]).astype(float)
        body.torque = 0.0
    body.static = static