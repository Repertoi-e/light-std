#pragma once

#include "vec.h"

LSTD_BEGIN_NAMESPACE

//
// Shapes
//

template <typename T, s64 Dim>
struct hyperplane;

template <typename T, s64 Dim>
struct line {
    using VectorT = vec<T, Dim>;
    VectorT Direction, Base;

    // :MathTypesNoInit By default we dont-init but you can call a constructor with a scalar value of 0 to zero-init. We do this to save on performance.
    line() {}

    line(const VectorT &base, const VectorT &direction) : Direction(direction), Base(base) {
        assert(is_normalized(direction));
    }

    // Constructs a line through both points.
    line(const VectorT &p1, const VectorT &p2) : line(p1, safe_normalize(p2 - p1)) {}

    // A 2D plane and line are equivalent, converts representation. Only for 2D.
    line(const hyperplane<T, 2> &plane);

    // Returns the point at distance from the base point along direction
    VectorT point_at(T dist) const { return Base + dist * Direction; }
};

template <typename T, s64 Dim>
struct line_segment {
    using VectorT = vec<T, Dim>;
    VectorT P1, P2;

    line_segment() : P1(0), P2(0) { P2[0] = 1; }
    line_segment(const VectorT &base, const VectorT &direction, T length) {
        P1 = base;
        P2 = base + direction * length;
    }
    line_segment(const VectorT &p1, const VectorT &p2) : P1(p1), P2(p2) {}

    T length() const { return len(P2 - P1); }
    VectorT interpolate(T t) const { return t * P2 + (T(1) - t) * P1; }

    explicit operator line<T, Dim>() const { return line<T, Dim>(P1, P2); }
};

template <typename T, s64 Dim>
struct ray : protected line<T, Dim> {
    // Inhreitance is protected to deny casting.
    // Casting is bad 'cause we don't want to implicit cast a ray to a line and _intersect()_ it with a plane.

    using line<T, Dim>::line;
    using line<T, Dim>::point_at;

    explicit operator line<T, Dim>() const { return (line<T, Dim>) *this; }
};

template <typename T, s64 Dim>
struct hyperplane {
    using VectorT = vec<T, Dim>;
    VectorT Normal;
    T Scalar;

    hyperplane() : Normal(0), Scalar(0) { normal(0) = 1; }
    hyperplane(const VectorT &base, const VectorT &normal) : normal(normal) {
        assert(IsNormalized(normal));
        scalar = Dot(normal, base);
    }
    hyperplane(const VectorT &normal, T scalar) : normal(normal), scalar(scalar) { assert(IsNormalized(normal)); }
    hyperplane(const line<T, 2> &line) {
        static_assert(Dim == 2, "Plane dimension must be two, which is a line.");
        normal = {-line.Direction()(1), line.Direction()(0)};
        scalar = Dot(normal, line.Base());
    }

    const VectorT &Normal() const { return normal; }
    T Scalar() const { return scalar; }

    template <bool Packed>
    T Distance(const vec<T, Dim, Packed> &point) {
        return Dot(point, normal) - scalar;
    }
};

template <typename T, s64 Dim>
line<T, Dim>::line(const hyperplane<T, 2> &plane) {
    static_assert(Dim == 2, "line dimension must be two, since it a plane in 2 dimensional space.");

    // Intersect plane's line with line through origo perpendicular to plane to find suitable base
    T a = plane.Normal()(0);
    T b = plane.Normal()(1);
    T d = plane.Scalar();
    T div = (a * a + b * b);
    base = {a * d / div, b * d / div};
    direction = {b, -a};
}

template <typename T>
struct Triangle3D {
   public:
    Triangle3D() {}
    Triangle3D(const vec<T, 3, false> &a, const vec<T, 3, false> &b, const vec<T, 3, false> &c) : a(a), b(b), c(c) {}
    vec<T, 3, false> a, b, c;  // Corners of the traingle.
};

//------------------------------------------------------------------------------
// Intersections
//------------------------------------------------------------------------------

template <typename T, typename U>
struct Intersection;

template <typename T, typename U>
auto Intersect(const T &t, const U &u) {
    return Intersection<T, U>(t, u);
}

// Plane-line intersection
template <typename T, s64 Dim>
struct Intersection<hyperplane<T, Dim>, line<T, Dim>> {
   protected:
    using PlaneT = hyperplane<T, Dim>;
    using LineT = line<T, Dim>;
    using VectorT = vec<T, Dim>;

   public:
    Intersection(const PlaneT &plane, const LineT &line);

    bool Intersecting() const { return !is_inf(param); }
    VectorT Point() const { return line.PointAt(param); }
    T LineParameter() const { return param; }

   private:
    LineT line;
    T param;
};

template <typename T, s64 Dim>
struct Intersection<line<T, Dim>, hyperplane<T, Dim>> : public Intersection<hyperplane<T, Dim>, line<T, Dim>> {
    using PlaneT = hyperplane<T, Dim>;
    using LineT = line<T, Dim>;

   public:
    Intersection(const LineT &line, const PlaneT &plane)
        : Intersection<hyperplane<T, Dim>, line<T, Dim>>(plane, line) {}
};

// Plane-line segment intersection
template <typename T, s64 Dim>
struct Intersection<hyperplane<T, Dim>, line_segment<T, Dim>> {
    using PlaneT = hyperplane<T, Dim>;
    using LineT = line_segment<T, Dim>;
    using VectorT = vec<T, Dim>;

   public:
    Intersection(const PlaneT &plane, const LineT &line) {
        lineSegment = line;
        auto intersection = Intersect(plane, line.line());
        param = intersection.LineParameter() / line.Length();
    }

    bool Intersecting() const { return T(0) <= param && param <= T(1); }
    VectorT Point() const { return lineSegment.Interpol(param); }
    T InterpolParameter() const { return param; }
    T LineParameter() const { return param * lineSegment.Length(); }

   private:
    LineT lineSegment;
    T param;
};

template <typename T, s64 Dim>
struct Intersection<line_segment<T, Dim>, hyperplane<T, Dim>>
    : public Intersection<hyperplane<T, Dim>, line_segment<T, Dim>> {
    using PlaneT = hyperplane<T, Dim>;
    using LineT = line_segment<T, Dim>;

   public:
    Intersection(const LineT &line, const PlaneT &plane)
        : Intersection<hyperplane<T, Dim>, line_segment<T, Dim>>(plane, line) {}
};

template <typename T, s64 Dim>
Intersection<hyperplane<T, Dim>, line<T, Dim>>::Intersection(const PlaneT &plane, const LineT &line) {
    this->line = line;

    // We have to solve the system of linear equations for x,y,z,t
    //	|d |	|a b c 0|	|x|
    //	|px|	|1 0 0 p|	|y|
    //	|py|  =	|0 1 0 q| *	|z|
    //	|pz|	|0 0 1 r|	|t|
    //   b    =     A     *  x
    // where [px,py,pz] + t*[p,q,r] = [x,y,z] is the line's equation
    // and ax + by + cz + d = 0 is the plane's equation

    vec<T, Dim + 1> b;
    vec<T, Dim + 1> A_inv_t;

    // Fill up 'b'
    b = -plane.Scalar() | line.Base();

    // Fill up 'A_inv_t', which is the last line of A^-1, used to calculate 't'
    A_inv_t = 1 | plane.Normal();

    // Compute result of the equation
    T scaler = Dot(line.Direction(), plane.Normal());
    T x_t = Dot(A_inv_t, b);
    T t = x_t / scaler;
    param = -t;
}

// 2D line intersection
// with lines
template <typename T>
struct Intersection<line<T, 2>, line<T, 2>> {
    using LineT = line<T, 2>;

   public:
    Intersection(const LineT &l1, const LineT &l2) {
        line2 = l2;
        auto intersection = Intersect(hyperplane<T, 2>(l1), l2);
        param2 = intersection.LineParameter();
        param1 = is_inf(param2) ? numeric_info<T>::infinity() : Length(intersection.Point() - l1.Base());
    }

    bool Intersecting() const { return !is_inf(param1); }
    T LineParameter1() const { return param1; }
    T LineParameter2() const { return param2; }
    vec<T, 2> Point() const { return line2.PointAt(param2); }

   private:
    T param1, param2;
    LineT line2;
};

// with hyperplanes
template <typename T>
struct Intersection<hyperplane<T, 2>, hyperplane<T, 2>> : public Intersection<line<T, 2>, line<T, 2>> {
   public:
    Intersection(const hyperplane<T, 2> &p1, const hyperplane<T, 2> &p2)
        : Intersection<line<T, 2>, line<T, 2>>(p1, p2) {}
};

// line segments
template <typename T>
struct Intersection<line_segment<T, 2>, line_segment<T, 2>> {
   public:
    Intersection(const line_segment<T, 2> &l1, const line_segment<T, 2> &l2) {
        lineSegment1 = l1;
        lineSegment2 = l2;
        auto intersection = Intersect(l1.line(), l2.line());
        if (intersection.Intersecting()) {
            param1 = intersection.LineParameter1() / l1.Length();
            param2 = intersection.LineParameter2() / l2.Length();
        } else {
            param1 = param2 = numeric_info<T>::infinity();
        }
    }

    bool Intersecting() const { return T(0) <= param1 && param2 <= T(1); }
    vec<T, 2> Point() const { return lineSegment1.Interpol(param1); }
    T InterpolParameter1() const { return param1; }
    T InterpolParameter2() const { return param2; }
    T LineParameter1() const { return param1 * lineSegment1.Length(); }
    T LineParameter2() const { return param2 * lineSegment2.Length(); }

   private:
    T param1;
    T param2;
    line_segment<T, 2> lineSegment1;
    line_segment<T, 2> lineSegment2;
};

// line segment vs line2d
template <typename T>
struct Intersection<line_segment<T, 2>, line<T, 2>> {
   public:
    Intersection(const line_segment<T, 2> &line1, const line<T, 2> &line2) {
        auto inter = Intersect(line1.line(), line2);
        if (inter.Intersecting() && inter.LineParameter1() < line1.Length()) {
            param1 = inter.LineParameter1();
            param2 = inter.LineParameter2();
        } else {
            param1 = param2 = numeric_info<T>::infinity();
        }
    }

    bool Intersecting() const { return !isinf(param1); }
    vec<T, 2> Point() const { return line1.line().PointAt(param1); }
    T LineParameter1() const { return param1; }
    T InterpolParameter1() const { return param1 / line1.Length(); }
    T LineParameter2() const { return param2; }

   private:
    T param1;
    T param2;
    line_segment<T, 2> line1;
};

template <typename T>
struct Intersection<line<T, 2>, line_segment<T, 2>> : private Intersection<line_segment<T, 2>, line<T, 2>> {
   public:
    Intersection(const line<T, 2> &line1, const line_segment<T, 2> &line2)
        : Intersection<line_segment<T, 2>, line<T, 2>>(line2, line1) {}

    using Intersection<line_segment<T, 2>, line<T, 2>>::Intersecting;
    using Intersection<line_segment<T, 2>, line<T, 2>>::Point;

    T LineParameter1() const { return Intersection<line_segment<T, 2>, line<T, 2>>::LineParameter2(); }
    T InterpolParameter2() const { return Intersection<line_segment<T, 2>, line<T, 2>>::InterpolParameter1(); }
    T LineParameter2() const { return Intersection<line_segment<T, 2>, line<T, 2>>::LineParameter1(); }
};

// ray-triangle intersection (M�ller-Trumbore algorithm)
template <typename T>
struct Intersection<ray<T, 3>, Triangle3D<T>> {
    using VectorT = vec<T, 3, false>;

   public:
    Intersection(const ray<T, 3> &ray, const Triangle3D<T> &triangle);

    bool IsIntersecting() const { return intersecting; }
    VectorT Point() const { return point; }

    template <typename U>
    U Interpolate(const U &a, const U &b, const U &c) const;

    T GetT() const { return t; }
    T GetU() const { return u; }
    T GetV() const { return v; }

   private:
    T t, u, v;
    bool intersecting;
    VectorT point;
};

template <typename T>
Intersection<ray<T, 3>, Triangle3D<T>>::Intersection(const ray<T, 3> &ray, const Triangle3D<T> &triangle) {
    constexpr T EPSILON = T(0.00000001);

    VectorT edge1 = triangle.b - triangle.a;
    VectorT edge2 = triangle.c - triangle.a;

    VectorT h = Cross(ray.Direction(), edge2);
    T a = Dot(edge1, h);

    if (abs(a) < EPSILON) {
        intersecting = false;
        return;
    }

    T f = T(1) / a;
    VectorT s = ray.Base() - triangle.a;
    u = f * Dot(s, h);

    if (u < T(0) || u > T(1)) {
        intersecting = false;
        return;
    }

    VectorT q = Cross(s, edge1);
    v = f * Dot(ray.Direction(), q);

    if (v < 0.0 || u + v > 1.0) {
        intersecting = false;
        return;
    }

    t = f * Dot(edge2, q);
    intersecting = t > EPSILON;
    if (intersecting) {
        point = ray.PointAt(t);
    }
}

template <typename T>
template <typename U>
U Intersection<ray<T, 3>, Triangle3D<T>>::Interpolate(const U &a, const U &b, const U &c) const {
    T w = T(1) - u - v;
    return u * b + v * c + w * a;
}

template <typename T, s64 Dim, s64 Order>
struct BezierCurve {
    static_assert(Order >= 1, "Bezier curve must have order n>=1.");

   public:
    using VectorT = vec<T, Dim, false>;

    VectorT operator()(T t) const { return EvalInterpolRecurse(t); }

   protected:
    VectorT EvalInterpolRecurse(T t) const;

   public:
    stack_array<VectorT, Order + 1> p;
};

template <typename T, s64 Dim, s64 Order>
auto BezierCurve<T, Dim, Order>::EvalInterpolRecurse(T t) const -> VectorT {
    stack_array<VectorT, Order + 1> reduction = p;

    T u = T(1) - t;

    for (s64 i = Order; i >= 1; --i) {
        for (s64 j = 1; j <= i; ++j) {
            reduction[j - 1] = u * reduction[j - 1] + t * reduction[j];
        }
    }

    return reduction[0];
}

LSTD_END_NAMESPACE
