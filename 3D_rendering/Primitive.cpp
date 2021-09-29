#include "Primitive.h"

Color::Color() {
	red = 0, green = 0, blue = 0;
}

Color::Color(float r, float g, float b) {
	red = r, green = g, blue = b;
}

Light::Light(const float x, const float y, const float z, const Color& c) {
	xVal = x, yVal = y, zVal = z, color = c;
}

Point::Point(const float x, const float y, const float z, const Color& c) {
	xVal = x, yVal = y, zVal = z, color = c;
}

SpaceVector::SpaceVector(const float x, const float y, const float z) {
	xVal = x, yVal = y, zVal = z;
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

