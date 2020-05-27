import numpy as np
import lstdgraphics as g

from constants import pixels_per_meter
import shape as sh

#
# This file implements drawing of different types of objects
#

def t_vert(mat, v):
    return (mat @ [v[0], v[1], 1])[:2]

def draw_shape(model_mat, shape, normals_color = 0, thickness = 3, aabb = None):
    '''
    Draws polygon with lines connecting each vertex.
    The color is stored in 'sh' .
    The shape is translated by 'pos'.
    Use 'normals_color' to specify the color of the normals (if 'sh' is a polygon).
    '''

    if isinstance(shape, sh.Circle):
        pass
    elif isinstance(shape, sh.ConvexPolygon):
        a = t_vert(model_mat, shape.vertices[0])
        for i in range(1, len(shape.vertices) + 1):
            b = t_vert(model_mat, shape.vertices[i % len(shape.vertices)]) # At the end we cycle back to the first vertex

            if normals_color != 0:        
                mid = ([(a[0] + b[0]) / 2, (a[1] + b[1]) / 2]) * pixels_per_meter
                n = shape.normals[i - 1] * pixels_per_meter 
                g.line(mid, mid + n, color = normals_color, thickness = thickness)
            g.line(a * pixels_per_meter, b * pixels_per_meter, color = shape.color, thickness = thickness)
            
            if aabb is not None:
                m = t_vert(model_mat, aabb[0])
                n = t_vert(model_mat, aabb[1])
                g.rect(m * pixels_per_meter, n * pixels_per_meter, color = normals_color)
            a = b
    else:
        print(sh)
