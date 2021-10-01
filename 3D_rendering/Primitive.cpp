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

Point2D::Point2D(const int x, const int y, const double d) : color() {
	xVal = x, yVal = y, depth = d;
}

void Point2D::colorThreshold() {
	if (color.getRed() > 1.0f)
		color.setRed(1.0f);
	if (color.getGreen() > 1.0f)
		color.setGreen(1.0f);
	if (color.getBlue() > 1.0f)
		color.setBlue(1.0f);
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

void Line::setSlope() {
	if (xStart == xEnd)
		vertical = true;
	else
		slope = (yEnd - yStart) / (xEnd - xStart);
}

void Line::swapPoints() {
	swap(xStart, xEnd);
	swap(yStart, yEnd);
	swap(depthStart, depthEnd);
}

void Line::drawLine(vector<Point2D>& drawList) {
	if (vertical) {
		if (yStart > yEnd) {
			swap(yStart, yEnd);
			swap(depthStart, depthEnd);
		}
		//for (float plotY = yStart; plotY <= yEnd; plotY++)
		for (int plotY = trunc(yStart); plotY <= trunc(yEnd); plotY++)
			drawList.push_back(Point2D(xStart,
									 plotY,
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

		int constA = trunc(yEnd) - trunc(yStart);
		int constB = trunc(xStart) - trunc(xEnd);
		int dVal = 2 * constA + constB;
		int plotY = trunc(yStart);
		for (int plotX = trunc(xStart); plotX <= trunc(xEnd); plotX++) {
			if (dVal <= 0)
				dVal += 2 * constA;
			else {
				plotY += 1;
				dVal += 2 * (constA + constB);
			}
			//draw lines based on slope
			double coeff = (static_cast<double>(plotX) - static_cast<double>(xStart)) / (static_cast<double>(xEnd) - static_cast<double>(xStart));
			if(slopeState==0)
				drawList.push_back(Point2D(plotX, plotY, (depthEnd - depthStart) * coeff + depthStart));
			else if(slopeState==1)
				drawList.push_back(Point2D(plotX * (-1), plotY, (depthEnd - depthStart) * coeff + depthStart));
			else if(slopeState==2)
				drawList.push_back(Point2D(plotY, plotX, (depthEnd - depthStart) * coeff + depthStart));
			else
				drawList.push_back(Point2D(plotY * (-1), plotX, (depthEnd - depthStart) * coeff + depthStart));
		}
	}
}

ASC::ASC(const float r, const float g, const float b, const float kd, const float ks, const float n) : color(r, g, b) {
	kdVal = kd, ksVal = ks, nVal = n;
	ascMatrix = { { }, { }, { }, { } };
	ascVertices = {};
	ascSurfaces = {};
}

void ASC::addMatrix() {
	ascMatrix[0].push_back(ascVertices.back().getXVal());
	ascMatrix[1].push_back(ascVertices.back().getYVal());
	ascMatrix[2].push_back(ascVertices.back().getZVal());
	ascMatrix[3].push_back(1);
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

