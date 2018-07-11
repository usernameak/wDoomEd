#include "math_util.h"

#include <cmath>

using namespace WDEdMathUtil;

/* MathStuff::linesIntersect
 * Checks for an intersection between two lines l1 and l2.  Returns
 * true if they intersect and sets out to the intersection point
 *******************************************************************/
bool WDEdMathUtil::linesIntersect(LineSegment l1, LineSegment l2, Point& out)
{
	// First, simple check for two parallel horizontal or vertical lines
	if ((l1.x1() == l1.x2() && l2.x1() == l2.x2()) || (l1.y1() == l1.y2() && l2.y1() == l2.y2()))
		return false;

	// Second, check if the lines share any endpoints
	if ((l1.x1() == l2.x1() && l1.y1() == l2.y1()) ||
			(l1.x2() == l2.x2() && l1.y2() == l2.y2()) ||
			(l1.x1() == l2.x2() && l1.y1() == l2.y2()) ||
			(l1.x2() == l2.x1() && l1.y2() == l2.y1()))
		return false;

	// Third, check bounding boxes
	if (fmax(l1.x1(), l1.x2()) < fmin(l2.x1(), l2.x2()) ||
			fmax(l2.x1(), l2.x2()) < fmin(l1.x1(), l1.x2()) ||
			fmax(l1.y1(), l1.y2()) < fmin(l2.y1(), l2.y2()) ||
			fmax(l2.y1(), l2.y2()) < fmin(l1.y1(), l1.y2()))
		return false;

	// Fourth, check for two perpendicular horizontal or vertical lines
	if (l1.x1() == l1.x2() && l2.y1() == l2.y2())
	{
		out.x = l1.x1();
		out.y = l2.y1();
		return true;
	}
	if (l1.y1() == l1.y2() && l2.x1() == l2.x2())
	{
		out.x = l2.x1();
		out.y = l1.y1();
		return true;
	}

	// Not a simple case, do full intersection calculation

	// Calculate some values
	double a1 = l1.y2() - l1.y1();
	double a2 = l2.y2() - l2.y1();
	double b1 = l1.x1() - l1.x2();
	double b2 = l2.x1() - l2.x2();
	double c1 = (a1 * l1.x1()) + (b1 * l1.y1());
	double c2 = (a2 * l2.x1()) + (b2 * l2.y1());
	double det = a1*b2 - a2*b1;

	// Check for no intersection
	if (det == 0)
		return false;

	// Calculate intersection point
	out.x = (b2*c1 - b1*c2) / det;
	out.y = (a1*c2 - a2*c1) / det;

	// Round to nearest 3 decimal places
	out.x = floor(out.x * 1000.0 + 0.5) / 1000.0;
	out.y = floor(out.y * 1000.0 + 0.5) / 1000.0;

	// Check that the intersection point is on both lines
	if (fmin(l1.x1(), l1.x2()) <= out.x && out.x <= fmax(l1.x1(), l1.x2()) &&
		fmin(l1.y1(), l1.y2()) <= out.y && out.y <= fmax(l1.y1(), l1.y2()) &&
		fmin(l2.x1(), l2.x2()) <= out.x && out.x <= fmax(l2.x1(), l2.x2()) &&
		fmin(l2.y1(), l2.y2()) <= out.y && out.y <= fmax(l2.y1(), l2.y2()))
		return true;

	// Intersection point does not lie on both lines
	return false;
}


double WDEdMathUtil::lineSide(Point point, LineSegment line) {
	return (point.x - line.x1()) * line.height() - (point.y - line.y1()) * line.width();
}

double WDEdMathUtil::angle2DRad(Point p1, Point p2, Point p3)
{
	// From: http://stackoverflow.com/questions/3486172/angle-between-3-points
	// modified not to bother converting to degrees
	Point ab(p2.x - p1.x, p2.y - p1.y);
	Point cb(p2.x - p3.x, p2.y - p3.y);

	// dot product
	double dot = (ab.x * cb.x + ab.y * cb.y);

	// length square of both vectors
	double abSqr = ab.x * ab.x + ab.y * ab.y;
	double cbSqr = cb.x * cb.x + cb.y * cb.y;

	// square of cosine of the needed angle
	double cosSqr = dot * dot / abSqr / cbSqr;

	// this is a known trigonometric equality:
	// cos(alpha * 2) = [ cos(alpha) ]^2 * 2 - 1
	double cos2 = 2 * cosSqr - 1;

	// Here's the only invocation of the heavy function.
	// It's a good idea to check explicitly if cos2 is within [-1 .. 1] range
	double alpha2 =
		(cos2 <= -1) ? M_PI :
		(cos2 >= 1) ? 0 :
		acosf(cos2);

	//double rslt = alpha2 / 2;
	//double rs = rslt * rad2deg;
	double rs = alpha2 / 2;

	// Now revolve the ambiguities.
	// 1. If dot product of two vectors is negative - the angle is definitely
	// above 90 degrees. Still we have no information regarding the sign of the angle.

	// NOTE: This ambiguity is the consequence of our method: calculating the cosine
	// of the double angle. This allows us to get rid of calling sqrt.
	if (dot < 0)
		rs = M_PI - rs;

	// 2. Determine the sign. For this we'll use the Determinant of two vectors.
	double det = (ab.x * cb.y - ab.y * cb.x);
	if (det < 0)
		rs = (2*M_PI) - rs;

	return rs;
}

double WDEdMathUtil::distance(Point p1, Point p2)
{
	return sqrt((p2.x-p1.x)*(p2.x-p1.x) + (p2.y-p1.y)*(p2.y-p1.y));
}

