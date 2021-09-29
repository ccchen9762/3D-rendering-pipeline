#include "Primitive.h"

const float PI = 3.1415926f / 180.0f;

ifstream file;

int windowWidth = 0, windowHeight = 0;

void initial() {
	glClearColor(0.0, 0.0, 0.0, 0.0);
}

void display() {
	glutSwapBuffers();
}

void keyboard(unsigned char key, int x, int y) {
	switch (key) {
	case 'r':		//reset transformation matrices
	case 'R':
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
	int split = 0;
	getline(file, command);
	windowWidth = stoi(command.substr(split, command.find(" ", split)));
	split = command.find(" ") + 1;
	windowHeight = stoi(command.substr(split, command.find(" ", split)));

	//
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
	glutInitWindowSize(windowWidth, windowHeight);
	glutInitWindowPosition(200, 100);
	glutCreateWindow("CG");
	glutDisplayFunc(&display);
	glutKeyboardFunc(keyboard);
	initial();
	glutMainLoop();
	return 0;
}