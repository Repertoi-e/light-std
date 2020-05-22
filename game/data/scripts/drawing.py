import numpy as np
import lstdgraphics as g

import shape

#
# This file implements drawing of different types of objects
#

def draw_shape(pos, sh, normals_color = 0):
    '''
    Draws polygon with lines connecting each vertex.
    The color is stored in 'sh' .
    The shape is translated by 'pos'.
    Use 'normals_color' to specify the color of the normals (if 'sh' is a polygon).
    '''

    if isinstance(sh, shape.Circle):
        pass
    elif isinstance(sh, shape.ConvexPolygon):
        a = sh.vertices[0]
        for i in range(1, len(sh.vertices) + 1):
            b = sh.vertices[i % len(sh.vertices)] # At the end we cycle back to the first vertex

            if normals_color != 0:        
                mid = [(a[0] + b[0]) / 2, (a[1] + b[1]) / 2]
                n = sh.normals[i - 1] 
                g.line(mid, mid + n * 10, color = normals_color)
            g.line(a, b, color = sh.color)
            a = b
    else:
        print(sh)
