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

//global variables
ifstream file;

int windowWidth = 0, windowHeight = 0;
float xRatio = 0, yRatio = 0;

bool reading = true;

//illumination
Color* ambient = nullptr;
Color* background = nullptr;
vector<Light> lights;

Point* eyePosition;
Point* COIPosition;	//center of interest

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
		for (float i = 0; i < backgroundPoints.size(); i++)
			glVertex3f(backgroundPoints[i].getXVal(), backgroundPoints[i].getYVal(), 1.0f);
	}
	glEnd();
	glutSwapBuffers();
}

void doIdle() {
	string command = "";
	while (reading && getline(file, command)) {
		if (command[0] != '#') {	//not comment
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
	/* make the screen update */
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