#include "Primitive.h"

Color::Color() {
	red = 0, green = 0, blue = 0;
}

Color::Color(const float r, const float g, const float b) {
	red = r, green = g, blue = b;
}

Light::Light(const float r, const float g, const float b, const float x, const float y, const float z) : color(r, g, b) {
	xVal = x, yVal = y, zVal = z;
}

Point::Point(const float x, const float y, const float z) : color() {
	xVal = x, yVal = y, zVal = z;
}

Point::Point(const float r, const float g, const float b, const float x, const float y, const float z) : color(r, g, b) {
	xVal = x, yVal = y, zVal = z;
}

SpaceVector::SpaceVector(const float x, const float y, const float z) {
	xVal = x, yVal = y, zVal = z;
}

void SpaceVector::normalization() {
	float scalar = sqrt(pow(xVal, 2) + pow(yVal, 2) + pow(zVal, 2));
	xVal /= scalar;
	yVal /= scalar;
	zVal /= scalar;
}

Line::Line(const float x1, const float y1, const float x2, const float y2) {
	xStart = x1, yStart = y1, xEnd = x2, yEnd = y2, slope = 0, vertical = false;
	if (xStart == xEnd)
		vertical = true;
	else
		slope = (yEnd - yStart) / (xEnd - xStart);
}

void Line::drawLine(vector<Point>& pointList) {

}
