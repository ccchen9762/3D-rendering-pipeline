#include "Primitive.h"

const float PI = 3.1415926f / 180.0f;

//functions declaration 
vector<float> readNumbers(const string& command, int now);
vector<vector<float>> matrixMultiplication(const vector<vector<float>>& matrixA, const vector<vector<float>>& matrixB);
inline SpaceVector crossProduct(SpaceVector& u, SpaceVector& v) {
	return SpaceVector(u.getYVal() * v.getZVal() - u.getZVal() * v.getYVal(),
					   u.getZVal() * v.getXVal() - u.getXVal() * v.getZVal(),
					   u.getXVal() * v.getYVal() - u.getYVal() * v.getXVal());
}

inline float dotProduct(SpaceVector& u, SpaceVector& v) {
	return u.getXVal() * v.getXVal() + u.getYVal() * v.getYVal() + u.getZVal() * v.getZVal();
}

inline bool sortPointsY(Point& p1, Point& p2) {
	return p1.getYVal() > p2.getYVal();
}

inline bool sortPointsX(Point& p1, Point& p2) {
	return p1.getXVal() > p2.getXVal();
}

inline bool sortPointsDepth(Point& p1, Point& p2) {
	return p1.getDepth() > p2.getDepth();
}

//global variables
ifstream file;

int windowWidth = 0, windowHeight = 0;
float xRatio = 0, yRatio = 0;

bool reading = true;

//illumination
Color* ambient = nullptr;
Color* background = nullptr;
vector<Light> lights;

Point* eyePosition = nullptr;
Point* COIPosition = nullptr;	//center of interest

vector<ASC> ascList;

vector<float> viewportVertex;

//Transformation Matrix
vector<vector<float>> TM = { { 1, 0, 0, 0 }, { 0, 1, 0, 0 }, { 0, 0, 1, 0 }, { 0, 0, 0, 1 } };
//Eye Matrix
vector<vector<float>> mirrorM = { { -1, 0, 0, 0 }, { 0, 1, 0, 0 }, { 0, 0, 1, 0 }, { 0, 0, 0, 1 } };
vector<vector<float>> GRM;
vector<vector<float>> tiltM;
vector<vector<float>> EM;
vector<vector<float>> eyePositionM;
//Perspective Matrix
vector<vector<float>> PM = { { 1, 0, 0, 0 }, { 0, 1, 0, 0 }, { 0, 0, 1, 0 }, { 0, 0, 0, 1 } };

vector<Point> backgroundPoints;
vector<Point> objectPoints;
vector<Point> surfacePoints;

void initial() {
	glClearColor(0.0, 0.0, 0.0, 0.0);
	gluOrtho2D(-windowWidth / 2, windowWidth / 2, -windowHeight / 2, windowHeight / 2);
}

void display() {
	glClear(GL_COLOR_BUFFER_BIT);
	glBegin(GL_POINTS);
	//draw background
	if (background) {
		glColor3f(background->getRed(), background->getGreen(), background->getBlue());
		for (int i = 0; i < backgroundPoints.size(); i++)
			glVertex3f(backgroundPoints[i].getXVal(), backgroundPoints[i].getYVal(), 1.0f);
	}
	sort(objectPoints.begin(), objectPoints.end(), sortPointsDepth);
	glColor3f(1.0, 1.0, 1.0);
	for (int i = 0; i < objectPoints.size(); i++) {
		glColor3f(objectPoints[i].getColor().getRed(), objectPoints[i].getColor().getGreen(), objectPoints[i].getColor().getBlue());
		glVertex3f(objectPoints[i].getXVal(), objectPoints[i].getYVal(), 1.0f);
	}
		
	glEnd();
	glutSwapBuffers();
}

void doIdle() {
	string command = "";
	while (reading && getline(file, command)) {
		if (command[0] != '#') {	// command is not comment
			// read instruction
			string instruction = "";
			int pos = command.find(" ");
			if (pos == std::string::npos)
				instruction = command.substr(0);
			else
				instruction = command.substr(0, pos);
			cout << instruction << endl;
			// execute by instruction
			if (instruction == "ambient") {
				vector<float> nums = readNumbers(command, pos + 1);
				ambient = new Color(nums[0], nums[1], nums[2]);
			}
			else if (instruction == "background") {
				vector<float> nums = readNumbers(command, pos + 1);
				background = new Color(nums[0], nums[1], nums[2]);
			}
			else if (instruction == "light") {
				vector<float> nums = readNumbers(command, pos + 1);
				Light light(nums[1], nums[2], nums[3], nums[4], nums[5], nums[6]);
				if (static_cast<int>(nums[0]) - 1 < lights.size())
					lights[static_cast<int>(nums[0]) - 1] = light;
				else
					lights.push_back(light);
			}
			else if (instruction == "scale") {
				vector<float> nums = readNumbers(command, pos + 1);
				vector<vector<float>> scaleMatrix = { { nums[0], 0, 0, 0 }, { 0, nums[1], 0, 0 }, { 0, 0, nums[2], 0 }, { 0, 0, 0, 1 } };
				TM = matrixMultiplication(scaleMatrix, TM);
			}
			else if (instruction == "rotate") {
				vector<float> nums = readNumbers(command, pos + 1);
				//rotate around x
				vector<vector<float>> rotateMatrix;
				rotateMatrix = { {1, 0, 0, 0},
										{ 0, cos(nums[0] * PI), ((-1) * sin(nums[0] * PI)), 0 },
										{ 0, sin(nums[0] * PI),  cos(nums[0] * PI), 0 },
										{ 0, 0, 0, 1 } };
				TM = matrixMultiplication(rotateMatrix, TM);
				//rotate around y
				rotateMatrix = { { cos(nums[1] * PI), 0, sin(nums[1] * PI), 0 },
										{ 0, 1, 0, 0 },
										{ ((-1) * sin(nums[1] * PI)), 0, cos(nums[1] * PI), 0 },
										{ 0, 0, 0, 1 } };
				TM = matrixMultiplication(rotateMatrix, TM);
				//rotate around z
				rotateMatrix = { { cos(nums[2] * PI), ((-1) * sin(nums[2] * PI)), 0, 0 },
										{ sin(nums[2] * PI),  cos(nums[2] * PI), 0, 0 },
										{ 0, 0, 1, 0 },
										{ 0, 0, 0, 1 } };
				TM = matrixMultiplication(rotateMatrix, TM);
			}
			else if (instruction == "translate") {
				vector<float> nums = readNumbers(command, pos + 1);
				vector<vector<float>> translateMatrix = { { 1, 0, 0, nums[0] }, { 0, 1, 0, nums[1] }, { 0, 0, 1, nums[2] }, { 0, 0, 0, 1 } };
				TM = matrixMultiplication(translateMatrix, TM);
			}
			else if (instruction == "observer") {
				vector<float> nums = readNumbers(command, pos + 1);
				eyePosition = new Point(nums[0], nums[1], nums[2]);
				COIPosition = new Point(nums[3], nums[4], nums[5]);
				//EM
				eyePositionM = { { 1, 0, 0, (-1) * nums[0] },
											{ 0, 1, 0, (-1) * nums[1] },
											{ 0, 0, 1, (-1) * nums[2] },
											{ 0, 0, 0, 1 } };
				float total = 0;
				SpaceVector vector3(nums[3] - nums[0], nums[4] - nums[1], nums[5] - nums[2]);
				SpaceVector vector1(vector3.getZVal(), 0, (-1) * vector3.getXVal());
				SpaceVector vector2 = crossProduct(vector3, vector1);
				vector3.normalization();
				vector1.normalization();
				vector2.normalization();
				GRM = { { vector1.getXVal(), vector1.getYVal(), vector1.getZVal(), 0 },
							{ vector2.getXVal(), vector2.getYVal(), vector2.getZVal(), 0 },
							{ vector3.getXVal(), vector3.getYVal(), vector3.getZVal(), 0 },
							{ 0, 0, 0, 1 } };
				tiltM = { {cos(nums[6] * PI), sin(nums[6] * PI), 0, 0},
							{ (-1) * sin(nums[6] * PI), cos(nums[6] * PI), 0, 0},
							{0, 0, 1, 0},
							{0, 0, 0, 1} };
				EM = matrixMultiplication(GRM, eyePositionM);
				EM = matrixMultiplication(mirrorM, EM);
				EM = matrixMultiplication(tiltM, EM);
				//PM
				PM = { {1, 0, 0, 0},
							{0 ,1, 0, 0},
							{0, 0,  (nums[8] / (nums[8] - nums[7])) * tan(nums[9] * PI), ((nums[7] * nums[8]) / (nums[7] - nums[8])) * tan(nums[9] * PI)},
							{0, 0,  tan(nums[9] * PI), 0} };
			}
			else if (instruction == "viewport") {
				viewportVertex = readNumbers(command, pos + 1);
				PM[1][1] = (viewportVertex[1] - viewportVertex[0]) / (viewportVertex[3] - viewportVertex[2]);
				viewportVertex[0] *= windowWidth / 2;
				viewportVertex[1] *= windowWidth / 2;
				viewportVertex[2] *= windowHeight / 2;
				viewportVertex[3] *= windowHeight / 2;
				xRatio = (viewportVertex[1] - viewportVertex[0]) / 2;
				yRatio = (viewportVertex[3] - viewportVertex[2]) / 2;
				//push points
				backgroundPoints.clear();
				backgroundPoints.reserve((viewportVertex[1] - viewportVertex[0] + 1) * (viewportVertex[3] - viewportVertex[2] + 1));
				for (int i = viewportVertex[0]; i <= viewportVertex[1]; i++)
					for (int j = viewportVertex[2]; j <= viewportVertex[3]; j++)
						backgroundPoints.push_back(Point(i, j, 1.0f));
			}
			else if (instruction == "object") {
				//	read file name
				while (command[pos] == ' ')
					pos++;
				string ascName = command.substr(pos, command.find(" ", pos) - pos);
				pos += ascName.size();

				//	read object color and kd ks N	
				vector<float> nums = readNumbers(command, pos + 1);
				ascList.push_back(ASC(nums[0], nums[1], nums[2], nums[3], nums[4], nums[5]));

				//open asc file and read vertex and plane amount
				ifstream ascFile(ascName);
				int vertexAmount = 0;
				int surfaceAmount = 0;
				do {
					getline(ascFile, command);
				} while (command[0] == '\0');
				nums = readNumbers(command, 0);
				vertexAmount = nums[0];
				surfaceAmount = nums[1];
				ascList.back().reserveVector(vertexAmount, surfaceAmount);

				//read Vertices
				for (int i = 0; i < vertexAmount; i++) {
					getline(ascFile, command);
					vector<float> nums = readNumbers(command, 0);
					ascList.back().addVertices(Point(nums[0], nums[1], nums[2]));
				}

				// TM x ascMatrix
				ascList.back().setMatrix(matrixMultiplication(TM, ascList.back().getMatrix()));
				ascList.back().setWorldMatrix();	//backup matrix after transformation

				//read surfaces
				vector <int> surface;
				for (int i = 0; i < surfaceAmount; i++) {
					getline(ascFile, command);
					vector<float> nums = readNumbers(command, 0);
					int vertexes = nums[0];
					surface.reserve(vertexes);
					for (int j = 0; j < vertexes; j++)
						surface.push_back(nums[j + 1] - 1);
					ascList.back().addSurface(surface);
					surface.clear();
				}
			}
			else if (instruction == "display") {
				objectPoints.clear();
				//  matrix calculation
				int ascAmount = ascList.size();
				for (int i = 0; i < ascList.size(); i++) {	//asc amount
					ascList[i].resetMatrix();		// reset to world space
					ascList[i].setMatrix(matrixMultiplication(EM, ascList[i].getMatrix()));
					ascList[i].setMatrix(matrixMultiplication(PM, ascList[i].getMatrix()));
					
					int surfaceAmount = ascList[i].getSurfaces().size();
					for (int j = 0; j < surfaceAmount; j++) { //asc's surface amount
						cout << "here";
						// get surface normal vector, (third - second) cross (first - second)
						SpaceVector surfaceVector1(ascList[i].getWorldMatrix()[0][ascList[i].getSurfaces()[j][2]] - ascList[i].getWorldMatrix()[0][ascList[i].getSurfaces()[j][1]],
												   ascList[i].getWorldMatrix()[1][ascList[i].getSurfaces()[j][2]] - ascList[i].getWorldMatrix()[1][ascList[i].getSurfaces()[j][1]],
												   ascList[i].getWorldMatrix()[2][ascList[i].getSurfaces()[j][2]] - ascList[i].getWorldMatrix()[2][ascList[i].getSurfaces()[j][1]]);

						SpaceVector surfaceVector2(ascList[i].getWorldMatrix()[0][ascList[i].getSurfaces()[j][0]] - ascList[i].getWorldMatrix()[0][ascList[i].getSurfaces()[j][1]],
												   ascList[i].getWorldMatrix()[1][ascList[i].getSurfaces()[j][0]] - ascList[i].getWorldMatrix()[1][ascList[i].getSurfaces()[j][1]],
												   ascList[i].getWorldMatrix()[2][ascList[i].getSurfaces()[j][0]] - ascList[i].getWorldMatrix()[2][ascList[i].getSurfaces()[j][1]]);
						//2 cross 1
						SpaceVector normalVector = crossProduct(surfaceVector2, surfaceVector1);
						normalVector.normalization();
						
						//central point of surface
						float totalX = 0.0f, totalY = 0.0f, totalZ = 0.0f;
						int vertexAmount = ascList[i].getSurfaces()[j].size();
						for (int k = 0; k < vertexAmount; k++) {		//surface's vertex amount
							totalX += ascList[i].getWorldMatrix()[0][ascList[i].getSurfaces()[j][k]];
							totalY += ascList[i].getWorldMatrix()[1][ascList[i].getSurfaces()[j][k]];
							totalZ += ascList[i].getWorldMatrix()[2][ascList[i].getSurfaces()[j][k]];
						}
						totalX /= vertexAmount;
						totalY /= vertexAmount;
						totalZ /= vertexAmount;

						Point central(totalX, totalY, totalZ);

						// lights
						Color diffuseLight(0, 0, 0);
						Color specularLight(0, 0, 0);
						for (int k = 0;  k < lights.size(); k++) {		//lights amount
							SpaceVector lightVector(lights[k].getXVal() - central.getXVal(),
													lights[k].getYVal() - central.getYVal(),
													lights[k].getZVal() - central.getZVal());
							lightVector.normalization();

							float N_dot_L = dotProduct(lightVector, normalVector);
							if (N_dot_L >= 0) {
								diffuseLight.setRed(diffuseLight.getRed() + ascList[i].getKdVal() * lights[k].getColor().getRed() * N_dot_L);
								diffuseLight.setGreen(diffuseLight.getGreen() + ascList[i].getKdVal() * lights[k].getColor().getGreen() * N_dot_L);
								diffuseLight.setBlue(diffuseLight.getBlue() + ascList[i].getKdVal() * lights[k].getColor().getBlue() * N_dot_L);
							}
							
							//reflection vector
							SpaceVector RVector(N_dot_L * normalVector.getXVal() * 2.0f - lightVector.getXVal(),
												N_dot_L * normalVector.getYVal() * 2.0f - lightVector.getYVal(),
												N_dot_L * normalVector.getZVal() * 2.0f - lightVector.getZVal());
							
							SpaceVector VVector(eyePosition->getXVal() - central.getXVal(),
												eyePosition->getYVal() - central.getYVal(),
												eyePosition->getZVal() - central.getZVal());
							VVector.normalization();

							float cosB = dotProduct(RVector, VVector);
							if (cosB > 0) {
								specularLight.setRed(ascList[i].getKsVal() * lights[k].getColor().getRed() * pow(cosB, ascList[i].getNVal()));
								specularLight.setGreen(ascList[i].getKsVal()* lights[k].getColor().getGreen()* pow(cosB, ascList[i].getNVal()));
								specularLight.setBlue(ascList[i].getKsVal()* lights[k].getColor().getBlue()* pow(cosB, ascList[i].getNVal()));
							}
						}
						
						// processing plane
						vertexAmount = ascList[i].getSurfaces()[j].size();
						for (int k = 0; k < vertexAmount; k++) {
							//check clipping
							bool clipping = false, trivial = false, reverse = false;
							float clippingC1 = 0, clippingC2 = 0;
							int plusMinus = 1, nextK = (k == ascList[i].getSurfaces()[j].size() - 1) ? 0 : k + 1;
							// 6 equation to determine clipping
							for (int m = 0; m < 2; m++) {
								for (int n = 0; n < 3; n++) {
									//W + X && W - X && W + Y && W - Y  && Z && W-Z
									if (!trivial) {
										float C1 = 0.0f, C2 = 0.0f;
										if (n == 2 && m == 0) {
											C1 = plusMinus * ascList[i].getMatrix()[n][ascList[i].getSurfaces()[j][k]];
											C2 = plusMinus * ascList[i].getMatrix()[n][ascList[i].getSurfaces()[j][nextK]];
										}
										else {
											C1 = ascList[i].getMatrix()[3][ascList[i].getSurfaces()[j][k]] +
												plusMinus * ascList[i].getMatrix()[n][ascList[i].getSurfaces()[j][k]];
											C2 = ascList[i].getMatrix()[3][ascList[i].getSurfaces()[j][nextK]] +
												plusMinus * ascList[i].getMatrix()[n][ascList[i].getSurfaces()[j][nextK]];
										}

										if (C1 >= 0 && C2 >= 0)
											continue;
										else if (C1 < 0 && C2 < 0) {
											trivial = true;
											cout << "trivial";
											break;
										}
										else {
											clippingC1 = C1;
											clippingC2 = C2;
											if (C2 >= 0)
												reverse = true;
											clipping = true;
										}
									}
								}
								plusMinus = -1;
							}

							if (!trivial) {
								double pointDepth1 = 0;
								double pointDepth2 = 0;
								if (clipping) {
									float coeff = clippingC1 / (clippingC1 - clippingC2);
									float clippingX = ascList[i].getMatrix()[0][ascList[i].getSurfaces()[j][k]] +
										coeff * (ascList[i].getMatrix()[0][ascList[i].getSurfaces()[j][nextK]] - ascList[i].getMatrix()[0][ascList[i].getSurfaces()[j][k]]);
									float clippingY = ascList[i].getMatrix()[1][ascList[i].getSurfaces()[j][k]] +
										coeff * (ascList[i].getMatrix()[1][ascList[i].getSurfaces()[j][nextK]] - ascList[i].getMatrix()[1][ascList[i].getSurfaces()[j][k]]);
									float	clippingZ = ascList[i].getMatrix()[2][ascList[i].getSurfaces()[j][k]] +
										coeff * (ascList[i].getMatrix()[2][ascList[i].getSurfaces()[j][nextK]] - ascList[i].getMatrix()[2][ascList[i].getSurfaces()[j][k]]);
									float	clippingW = ascList[i].getMatrix()[3][ascList[i].getSurfaces()[j][k]] +
										coeff * (ascList[i].getMatrix()[3][ascList[i].getSurfaces()[j][nextK]] - ascList[i].getMatrix()[3][ascList[i].getSurfaces()[j][k]]);
									//perspective divide
									clippingX /= clippingW;
									clippingY /= clippingW;

									//C2 >= 0
									if (reverse) {
										Line addLine((clippingX + 1) * xRatio + viewportVertex[0],
													 (clippingY + 1) * yRatio + viewportVertex[2],
													 static_cast<double>(clippingZ) / static_cast<double>(clippingW),
													 (ascList[i].getMatrix()[0][ascList[i].getSurfaces()[j][nextK]] / ascList[i].getMatrix()[3][ascList[i].getSurfaces()[j][nextK]] + 1) * xRatio + viewportVertex[0],
													 (ascList[i].getMatrix()[1][ascList[i].getSurfaces()[j][nextK]] / ascList[i].getMatrix()[3][ascList[i].getSurfaces()[j][nextK]] + 1) * yRatio + viewportVertex[2],
													 static_cast<double>(ascList[i].getMatrix()[2][ascList[i].getSurfaces()[j][nextK]]) / static_cast<double>(ascList[i].getMatrix()[3][ascList[i].getSurfaces()[j][nextK]]));
										addLine.drawLine(surfacePoints);
									}
									//C1 >= 0
									else {
										Line addLine((ascList[i].getMatrix()[0][ascList[i].getSurfaces()[j][k]] / ascList[i].getMatrix()[3][ascList[i].getSurfaces()[j][k]] + 1)* xRatio + viewportVertex[0],
													 (ascList[i].getMatrix()[1][ascList[i].getSurfaces()[j][k]] / ascList[i].getMatrix()[3][ascList[i].getSurfaces()[j][k]] + 1)* yRatio + viewportVertex[2],
													 static_cast<double>(ascList[i].getMatrix()[2][ascList[i].getSurfaces()[j][k]]) / static_cast<double>(ascList[i].getMatrix()[3][ascList[i].getSurfaces()[j][k]]),
													 (clippingX + 1)* xRatio + viewportVertex[0],
													 (clippingY + 1)* yRatio + viewportVertex[2],
													 static_cast<double>(clippingZ) / static_cast<double>(clippingW));
										addLine.drawLine(surfacePoints);
									}
								}
								else {	//no clipping
									Line addLine((ascList[i].getMatrix()[0][ascList[i].getSurfaces()[j][k]] / ascList[i].getMatrix()[3][ascList[i].getSurfaces()[j][k]] + 1)* xRatio + viewportVertex[0],
												 (ascList[i].getMatrix()[1][ascList[i].getSurfaces()[j][k]] / ascList[i].getMatrix()[3][ascList[i].getSurfaces()[j][k]] + 1)* yRatio + viewportVertex[2],
												 static_cast<double>(ascList[i].getMatrix()[2][ascList[i].getSurfaces()[j][k]]) / static_cast<double>(ascList[i].getMatrix()[3][ascList[i].getSurfaces()[j][k]]),
												 (ascList[i].getMatrix()[0][ascList[i].getSurfaces()[j][nextK]] / ascList[i].getMatrix()[3][ascList[i].getSurfaces()[j][nextK]] + 1)* xRatio + viewportVertex[0],
												 (ascList[i].getMatrix()[1][ascList[i].getSurfaces()[j][nextK]] / ascList[i].getMatrix()[3][ascList[i].getSurfaces()[j][nextK]] + 1)* yRatio + viewportVertex[2],
												 static_cast<double>(ascList[i].getMatrix()[2][ascList[i].getSurfaces()[j][nextK]]) / static_cast<double>(ascList[i].getMatrix()[3][ascList[i].getSurfaces()[j][nextK]]));
									addLine.drawLine(surfacePoints);
								}
							}
						}
						

						
						//add viewport lines (clipping surface repair) bad
						int leftFirst = viewportVertex[3];
						int leftSecond = viewportVertex[2];
						int rightFirst = viewportVertex[3];
						int rightSecond = viewportVertex[2];
						int upFirst = viewportVertex[1];
						int upSecond = viewportVertex[0];
						int downFirst = viewportVertex[1];
						int downSecond = viewportVertex[0];
						double depthFirst = 0;
						double depthSecond = 0;
						bool left = false, right = false, down = false, up = false;
						for (int k = 0; k < surfacePoints.size(); k++) {
							if (surfacePoints[k].getXVal() == viewportVertex[0]) {
								left = true;
								if (surfacePoints[k].getYVal() > leftSecond) {
									leftSecond = surfacePoints[k].getYVal();
									depthSecond = surfacePoints[k].getDepth();
								}
								else if (surfacePoints[k].getYVal() <leftFirst ) {
									leftFirst = surfacePoints[k].getYVal();
									depthFirst = surfacePoints[k].getDepth();
								}
							}
							else if (surfacePoints[k].getXVal() == viewportVertex[1]) {
								right = true;
								if (surfacePoints[k].getYVal() > rightSecond) {
									rightSecond = surfacePoints[k].getYVal();
									depthSecond = surfacePoints[k].getDepth();
								}
								else if (surfacePoints[k].getYVal() < rightFirst) {
									rightFirst = surfacePoints[k].getYVal();
									depthFirst = surfacePoints[k].getDepth();
								}
							}
							else if (surfacePoints[k].getYVal() == viewportVertex[2]) {
								down = true;
								if (surfacePoints[k].getXVal() > downSecond) {
									downSecond = surfacePoints[k].getXVal();
									depthSecond = surfacePoints[k].getDepth();
								}
								else if (surfacePoints[k].getXVal() < downFirst) {
									downFirst = surfacePoints[k].getXVal();
									depthFirst = surfacePoints[k].getDepth();
								}
							}
							else if (surfacePoints[k].getYVal() == viewportVertex[3]) {
								up = true;
								if (surfacePoints[k].getXVal() > upSecond) {
									upSecond = surfacePoints[k].getXVal();
									depthSecond = surfacePoints[k].getDepth();
								}
								else if (surfacePoints[k].getXVal() < upFirst) {
									upFirst = surfacePoints[k].getXVal();
									depthFirst = surfacePoints[k].getDepth();
								}
							}
						}

						if (left) {
							for (int a = leftFirst; a <= leftSecond; a++)
								surfacePoints.push_back(Point(viewportVertex[0],
															  static_cast<float>(a),
															  0.0f,
															  (depthFirst - depthSecond)* (a - leftFirst) / (leftSecond - leftFirst) + depthSecond));
						}
						else if (right) {
							for (int a = rightFirst; a <= rightSecond; a++)
								surfacePoints.push_back(Point(viewportVertex[1],
															  static_cast<float>(a),
															  0.0f,
															  (depthFirst - depthSecond)* (a - leftFirst) / (leftSecond - leftFirst) + depthSecond));
						}
						else if (down) {
							for (int a = downFirst; a <= downSecond; a++)
								surfacePoints.push_back(Point(static_cast<float>(a),
															  viewportVertex[2],
															  0.0f,
															  (depthFirst - depthSecond) * (a - leftFirst) / (leftSecond - leftFirst) + depthSecond));
						}
						else if (up) {
							for (int a = upFirst; a <= upSecond; a++)
								surfacePoints.push_back(Point(static_cast<float>(a),
															  viewportVertex[3],
															  0.0f,
															  (depthFirst - depthSecond) * (a - leftFirst) / (leftSecond - leftFirst) + depthSecond));
						}

						// fill surface
						sort(surfacePoints.begin(), surfacePoints.end(), sortPointsY);
						int len = surfacePoints.size();
						float xLeft = surfacePoints[0].getXVal();
						float xRight = surfacePoints[0].getXVal();
						double depthLeft = surfacePoints[0].getDepth();
						double depthRight = surfacePoints[0].getDepth();
						for (int m = 1; m < len; m++) {
							if (surfacePoints[m].getYVal() != surfacePoints[m - 1].getYVal()) {
								for (int n = roundf(xLeft) + 1; n < roundf(xRight); n++) {
									surfacePoints.push_back(Point(n,
																  roundf(surfacePoints[m - 1].getYVal()),
																  0.0f,
																  (depthRight - depthLeft) * (n - xLeft) / (xRight - xLeft) + depthLeft));
								}
								xLeft = surfacePoints[m].getXVal();
								xRight = surfacePoints[m].getXVal();
								depthLeft = surfacePoints[m].getDepth();
								depthRight = surfacePoints[m].getDepth();
							}
							else {
								if (surfacePoints[m].getXVal() < xLeft) {
									xLeft = surfacePoints[m].getXVal();
									depthLeft = surfacePoints[m].getDepth();
								}
								else if (surfacePoints[m].getXVal() > xRight) {
									xRight = surfacePoints[m].getXVal();
									depthRight = surfacePoints[m].getDepth();
								}
							}
						}
						//////////////////////////////   determine color  /////////////////////////////
						for (float a = 0; a < surfacePoints.size(); a++) {
							surfacePoints[a].setColor(Color((ambient->getRed() + diffuseLight.getRed()) * ascList[i].getColor().getRed() + specularLight.getRed(),
															(ambient->getGreen() + diffuseLight.getGreen()) * ascList[i].getColor().getGreen() + specularLight.getGreen(),
															(ambient->getBlue() + diffuseLight.getBlue()) * ascList[i].getColor().getBlue() + specularLight.getBlue()));
							surfacePoints[a].colorThreshold();
							objectPoints.push_back(surfacePoints[a]);
						}
						surfacePoints.clear();
					}
				}

				reading = false;
			}
			else if (instruction == "reset") {
				TM = { { 1, 0, 0, 0 }, { 0, 1, 0, 0 }, { 0, 0, 1, 0 }, { 0, 0, 0, 1 } };
			}
			else if (instruction == "end") {
				file.close();
				exit(0);
			}
		}
	}
	//update screen
	glutPostRedisplay();
}

void keyboard(unsigned char key, int x, int y) {
	switch (key) {
	case ' ':
		reading = true;
		break;
	case 'q':
		exit(0);
		break;
	}
}

int main(int argc, char* argv[]) {
	//open file
	string fileName = argv[1];
	file.open(fileName);
	if (file.is_open())
		cout << "opening " + fileName << endl;
	else {
		cout << "file does not exist" << endl;
		exit(0);
	}

	// read window width & height 
	string command = "";
	getline(file, command);
	int split = command.find(" ");
	windowWidth = stoi(command.substr(0, split));
	windowHeight = stoi(command.substr(split + 1));

	//settings
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
	glutInitWindowSize(windowWidth, windowHeight);
	glutInitWindowPosition(200, 100);
	glutCreateWindow("CG");
	glutDisplayFunc(&display);
	glutIdleFunc(doIdle);
	glutKeyboardFunc(keyboard);
	initial();
	glutMainLoop();
	return 0;
}