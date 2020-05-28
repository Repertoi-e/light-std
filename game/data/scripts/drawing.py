import lstdgraphics as g

import numpy as np
import shape as sh

#
# Since there is no better place, here is a list of functions supported by 'lstdgraphics'
# line(p1, p2, color, thickness = 1.0)
# rect(min, max, color, rounding = 0.0, corner_flags = Corner.NONE, thickness = 1.0)
# rect_filled(min, max, color, rounding = 0.0, corner_flags = Corner.NONE)
# rect_filled_multi_color(min, max, color_ul, color_ur, color_dr, color_dl)
# quad(p1, p2, p3, p4, color, thickness = 1.0)
# quad_filled(p1, p2, p3, p4, color)
# triangle(p1, p2, p3, color, thickness = 1.0)
# triangle_filled(p1, p2, p3, color)
# circle(center, radius, color, num_segments = 12, thickness = 1.0)
# circle_filled(center, radius, color, num_segments = 12)
#
# Corner flags are in lstdgraphics.Corner:
#   NONE, TOP_LEFT, TOP_RIGHT, BOT_LEFT, BOT_RIGHT, TOP, BOT, LEFT, RIGHT, ALL
#

#
# This file implements drawing of different types of objects
#

def dot(mat, v):
    '''
    Helper function to multiply 2D vector by 3x3 transformation matrix
    '''
    return (mat @ [v[0], v[1], 1])[:2]

def draw_shape(model_mat, shape, normals_color = 0, thickness = 3, aabb = None):
    '''
    Draws polygon with lines connecting each vertex.
    The color is stored in 'shape' .
    The shape is transformed by 'model_mat'.
    Use 'normals_color' to specify the color of the normals (if 'shape' is a polygon).
    Use 'aabb' to draw a bounding box (also gets transformed by model_mat).
    '''

    if isinstance(shape, sh.Circle):
        center = dot(model_mat, [0, 0])
        g.circle(center, shape.radius, num_segments = 20, color = shape.color, thickness = thickness)
    elif isinstance(shape, sh.ConvexPolygon):
        a = dot(model_mat, shape.vertices[0])
        for i in range(1, len(shape.vertices) + 1):
            b = dot(model_mat, shape.vertices[i % len(shape.vertices)]) # At the end we cycle back to the first vertex

            if normals_color != 0:        
                mid = np.array([(a[0] + b[0]) / 2, (a[1] + b[1]) / 2])
                n = shape.normals[i - 1] 
                g.line(mid, (mid + n), color = normals_color, thickness = thickness)
            g.line(a, b, color = shape.color, thickness = thickness)
            
            if aabb is not None:
                m = dot(model_mat, aabb[0])
                n = dot(model_mat, aabb[1])
                g.rect(m, n, color = normals_color, rounding = 1.0, corner_flags = g.Corner.ALL)
            a = b
    else:
        print(shape)
