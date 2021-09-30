#include "Primitive.h"

Color::Color() {
	red = 0, green = 0, blue = 0;
}

Color::Color(const float r, const float g, const float b) {
	red = r, green = g, blue = b;
}

void Point::colorThreshold() {
	if (color.getRed() > 1.0f)
		color.setRed(1.0f);
	if (color.getGreen() > 1.0f)
		color.setGreen(1.0f);
	if (color.getBlue() > 1.0f)
		color.setBlue(1.0f);
}

Light::Light(const float r, const float g, const float b, const float x, const float y, const float z) : color(r, g, b) {
	xVal = x, yVal = y, zVal = z;
}

Point::Point(const float x, const float y, const float z) : color() {
	xVal = x, yVal = y, zVal = z, depth=0;
}

Point::Point(const float x, const float y, const float z, const double d) : color() {
	xVal = x, yVal = y, zVal = z, depth = d;
}

Point::Point(const float r, const float g, const float b, const float x, const float y, const float z) : color(r, g, b) {
	xVal = x, yVal = y, zVal = z, depth=0;
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

Line::Line(const float x1, const float y1, const double d1, const float x2, const float y2, const double d2) {
	xStart = x1, yStart = y1, depthStart=d1, xEnd = x2, yEnd = y2, depthEnd=d2, slope = 0, vertical = false;
	if (xStart == xEnd)
		vertical = true;
	else
		slope = (yEnd - yStart) / (xEnd - xStart);
}

void Line::drawLine(vector<Point>& drawList) {
	if (vertical) {
		if (yStart > yEnd) {
			swap(yStart, yEnd);
			swap(depthStart, depthEnd);
		}
		//for (float plotY = yStart; plotY <= yEnd; plotY++)
		for (int plotY = round(yStart); plotY <= round(yEnd); plotY++)
			drawList.push_back(Point(xStart,
									 plotY,
									 0.0f,
									 (depthEnd - depthStart) * (static_cast<double>(plotY) - static_cast<double>(yStart)) / (static_cast<double>(yEnd) - static_cast<double>(yStart)) + depthStart));
	}
	else {
		int slopeState = 0;
		if (slope >= -1 && slope < 0) {
			xStart *= -1;
			xEnd *= -1;
			slopeState = 1;
		}
		else if (slope > 1) {
			swap(xStart, yStart);
			swap(xEnd, yEnd);
			slopeState = 2;
		}
		else if (slope < -1) {
			xStart *= -1;
			xEnd *= -1;
			swap(xStart, yStart);
			swap(xEnd, yEnd);
			slopeState = 3;
		}

		if (xStart > xEnd) {
			swap(xStart, xEnd);
			swap(yStart, yEnd);
			swap(depthStart, depthEnd);
		}

		float constA = yEnd - yStart;
		float constB = xStart - xEnd;
		float dVal = 2 * constA + constB;
		//float plotY = yStart;
		//for (float plotX = xStart; plotX <= xEnd; plotX++) {
		int plotY = round(yStart);
		for (int plotX = round(xStart); plotX <= round(xEnd); plotX++) {
			if (dVal <= 0)
				dVal += 2 * constA;
			else {
				plotY += 1;
				dVal += 2 * (constA + constB);
			}
			//draw the line based on input slope
			double coeff = (static_cast<double>(plotX) - static_cast<double>(xStart)) / (static_cast<double>(xEnd) - static_cast<double>(xStart));
			switch (slopeState) {
			case 0:
				drawList.push_back(Point(plotX, plotY, 0.0f, (depthEnd - depthStart) * coeff + depthStart));
				break;
			case 1:
				drawList.push_back(Point(plotX * (-1), plotY, 0.0f, (depthEnd - depthStart) * coeff + depthStart));
				break;
			case 2:
				drawList.push_back(Point(plotY, plotX, 0.0f, (depthEnd - depthStart) * coeff + depthStart));
				break;
			case 3:
				drawList.push_back(Point(plotY * (-1), plotX, 0.0f, (depthEnd - depthStart) * coeff + depthStart));
				break;
			}
		}
	}
}

ASC::ASC(const float r, const float g, const float b, const float kd, const float ks, const float n) : color(r, g, b) {
	kdVal = kd, ksVal = ks, nVal = n;
	ascMatrix = { { }, { }, { }, { } };
	ascVertices = {};
	ascSurfaces = {};
}

void ASC::addVertices(const Point& p) {
	ascVertices.push_back(p);
	addMatrix();
}

void ASC::addSurface(const vector<int>& vertices) {
	ascSurfaces.push_back(vertices);
}

void ASC::reserveVector(const int vertexAmount, const int surfaceAmount) {
	ascMatrix[0].reserve(vertexAmount);
	ascMatrix[1].reserve(vertexAmount);
	ascMatrix[2].reserve(vertexAmount);
	ascMatrix[3].reserve(vertexAmount);
	ascVertices.reserve(vertexAmount);
	ascSurfaces.reserve(surfaceAmount);
}

