#pragma once

#include<iostream>
#include<vector>
#include<string>
#include<fstream>
#include<cmath>
#include<GL/glut.h>

using std::cin;
using std::cout;
using std::getline;
using std::endl;
using std::vector;
using std::string;
using std::ifstream;

class Color {
	float red, green, blue;

public:
	Color();
	Color(float r, float g, float b);
	void setRed(const float r) { red = r; }
	void setGreen(const float g) { green = g; }
	void setBlue(const float b) { blue = b; }
	float getRed() { return red; }
	float getGreen() { return green; }
	float getBlue() { return blue; }
};

class Light {
	float xVal, yVal, zVal;
	Color color;

public:
	Light(const float r, const float g, const float b, const float x, const float y, const float z);
	void setXVal(const float x) { xVal = x; }
	void setYVal(const float y) { yVal = y; }
	void setZVal(const float z) { zVal = z; }
	void setColor(const Color& c) { color = c; }
	float getXVal() { return xVal; }
	float getYVal() { return yVal; }
	float getZVal() { return zVal; }
	Color getColor() { return color; }
};

class Point {
private:
	float xVal, yVal, zVal;
	Color color;

public:
	Point(const float x, const float y, const float z);
	Point(const float r, const float g, const float b, const float x, const float y, const float z);
	void setXVal(const float x) { xVal = x; }
	void setYVal(const float y) { yVal = y; }
	void setZVal(const float z) { zVal = z; }
	void setColor(const Color& c) { color = c; }
	float getXVal() { return xVal; }
	float getYVal() { return yVal; }
	float getZVal() { return zVal; }
	Color getColor() { return color; }
};

class SpaceVector {
private:
	float xVal, yVal, zVal;

public:
	SpaceVector(const float x, const float y, const float z);
	void setXVal(const float x) { xVal = x; }
	void setYVal(const float y) { yVal = y; }
	void setZVal(const float z) { zVal = z; }
	float getXVal() { return xVal; }
	float getYVal() { return yVal; }
	float getZVal() { return zVal; }

	void normalization();
};

class Line {
private:
	float xStart, yStart, xEnd, yEnd, slope;
	bool vertical;

	void setSlope() {
		if (xStart == xEnd)
			vertical = true;
		else
			slope = (yEnd - yStart) / (xEnd - xStart);
	}

public:
	Line(const float x1, const float y1, const float x2, const float y2);
	void setXStart(const float x1) { xStart = x1, setSlope(); }
	void setYStart(const float y1) { yStart = y1, setSlope(); }
	void setXEnd(const float x2) { xEnd = x2, setSlope(); }
	void setYEnd(const float y2) { yEnd = y2, setSlope(); }
	float getXStart() { return xStart; }
	float getYStart() { return yStart; }
	float getXEnd() { return xEnd; }
	float getYEnd() { return yEnd; }
	float getSlope() { return slope; }
	bool isVertical() { return vertical; }

	void drawLine(vector<Point>& pointList);
};

class ASC {
private:
	float ksVal, kdVal, nVal;
	Color color;
	vector<vector<float>> ascMatrix;
	vector<Point> ascVertices;
	vector<vector<int>> ascSurfaces;

	void addMatrix() { 
		ascMatrix[0].push_back(ascVertices.back().getXVal());
		ascMatrix[1].push_back(ascVertices.back().getYVal());
		ascMatrix[2].push_back(ascVertices.back().getZVal());
		ascMatrix[3].push_back(1);
	}
public:
	ASC(const float r, const float g, const float b, const float kd, const float ks, const float n);
	void setKdVal(const float kd) { kdVal = kd; }
	void setKsVal(const float ks) { ksVal = ks; }
	void setNVal(const float n) { nVal = n; }
	void setColor(const Color& c) { color = c; }
	void setMatrix(const vector<vector<float>>& matrix) { ascMatrix = matrix; }
	float getKdVal() { return kdVal; }
	float getKsVal() { return ksVal; }
	float getNVal() { return nVal; }
	Color getColor() { return color; }
	vector<vector<float>> getMatrix() { return ascMatrix; }
	vector<Point> getVertices() { return ascVertices; }
	vector<vector<int>> getSurfaces() { return ascSurfaces; }

	void addVertices(const Point& p);
	void addSurface(const vector<int>& vertices);
	void reserveVector(const int vertexAmount, const int surfaceAmount);
};
