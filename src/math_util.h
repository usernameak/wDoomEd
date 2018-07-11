#pragma once

namespace WDEdMathUtil {
    struct Point {
        float x, y;
        inline void set(float nx, float ny) {x = nx; y = ny;}
        Point(float x, float y) : x(x), y(y) {}
        Point() : x(0), y(0) {}
    };
    struct LineSegment {
        Point p1, p2;
        inline float x1() {return p1.x;}
        inline float x2() {return p2.x;}
        inline float y1() {return p1.y;}
        inline float y2() {return p2.y;}
        LineSegment(float x1, float y1, float x2, float y2) : p1(x1, y1), p2(x2, y2) {}
        LineSegment() {}
        LineSegment(Point p1, Point p2) : p1(p1), p2(p2) {}
        double width()
        {
            return p2.x - p1.x;
        }

        double height()
        {
            return p2.y - p1.y;
        }

    };
    struct BoundingBox
    {
        Point	min;
        Point	max;

        BoundingBox() { reset(); }

        void reset()
        {
            min.set(0, 0);
            max.set(0, 0);
        }

        void extend(double x, double y)
        {
            // Init bbox if it has been reset last
            if (min.x == 0 && min.y == 0 && max.x == 0 && max.y == 0)
            {
                min.set(x, y);
                max.set(x, y);
                return;
            }

            // Extend to fit the point [x,y]
            if (x < min.x)
                min.x = x;
            if (x > max.x)
                max.x = x;
            if (y < min.y)
                min.y = y;
            if (y > max.y)
                max.y = y;
        }

        bool point_within(double x, double y)
        {
            return (x >= min.x && x <= max.x && y >= min.y && y <= max.y);
        }
        bool contains(Point point)
        {
            return point_within(point.x, point.y);
        }

        bool is_within(Point bmin, Point bmax)
        {
            return (min.x >= bmin.x && max.x <= bmax.x && min.y >= bmin.y && max.y <= bmax.y);
        }

        bool is_valid()
        {
            return ((max.x - min.x > 0) && (max.y - min.y) > 0);
        }

        Point size()
        {
            return Point(max.x - min.x, max.y - min.y);
        }

        double width()
        {
            return max.x - min.x;
        }

        double height()
        {
            return max.y - min.y;
        }

        Point mid()
        {
            return Point(mid_x(), mid_y());
        }

        double mid_x()
        {
            return min.x + ((max.x - min.x) * 0.5);
        }

        double mid_y()
        {
            return min.y + ((max.y - min.y) * 0.5);
        }

        LineSegment left_side()
        {
            return LineSegment(min.x, min.y, min.x, max.y);
        }

        LineSegment right_side()
        {
            return LineSegment(max.x, min.y, max.x, max.y);
        }

        LineSegment bottom_side()
        {
            return LineSegment(min.x, max.y, max.x, max.y);
        }

        LineSegment top_side()
        {
            return LineSegment(min.x, min.y, max.x, min.y);
        }
    };
    bool linesIntersect(LineSegment l1, LineSegment l2, Point& out);
    double lineSide(Point point, LineSegment line);
    double angle2DRad(Point p1, Point p2, Point p3);
    double distance(Point p1, Point p2);
}