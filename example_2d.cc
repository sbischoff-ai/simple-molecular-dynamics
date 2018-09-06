#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES
#endif
#include <cmath>
#include <iostream>
#include <iomanip>
#include <cstdio>
#include "MDSim.h"
#include <GL/glut.h>
#include <GL/freeglut_ext.h>
//#define sprintf_s snprintf
using namespace std;

int plotMode = 0;
const double SQR_2_5 = pow(2.0, 0.2);
const double SIGMA_6 = pow(0.025, 6.0);

float* colorHue(float h) {	// h=0..1
	float rgb[3];
	int i = (1 - h) * 5;  float f = (1 - h) * 5 - i;
	switch ((i + 5) % 6) {
	case 0: rgb[0] = 1;   rgb[1] = f;   rgb[2] = 0;   break;
	case 1: rgb[0] = 1 - f; rgb[1] = 1;   rgb[2] = 0;   break;
	case 2: rgb[0] = 0;   rgb[1] = 1;   rgb[2] = f;   break;
	case 3: rgb[0] = 0;   rgb[1] = 1 - f; rgb[2] = 1;   break;
	case 4: rgb[0] = f;   rgb[1] = 0;   rgb[2] = 1;   break;
	case 5: rgb[0] = 1;   rgb[1] = 0;   rgb[2] = 1 - f; break;
	}
	return rgb;
}

int glutStuff(void(*timeStep)(int), void(*drawWindow)(), void(*reshapeWindow)(int, int),
	void(*keyboard)(unsigned char, int, int), int* argc_, char** argv_,
	void* windowData, const char* title){

	glutInit(argc_, argv_);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowSize(1024, 512);
	glutCreateWindow(title);
	glutReshapeFunc(reshapeWindow);
	glutDisplayFunc(drawWindow);
	glutTimerFunc(0, timeStep, 0);
	glutKeyboardFunc(keyboard);
	glutSetWindowData(windowData);
	glutMainLoop();
	return 0;
}

void draw(){
	MDSim* sim = (MDSim*)glutGetWindowData();
	void* font = GLUT_BITMAP_9_BY_15;
	int i, j;
	MDParticleListEntry* entry;

	glClearColor(0., 0., 0., 0.);
	glClear(GL_COLOR_BUFFER_BIT);

	// define display list for disks
	GLUquadric* q = gluNewQuadric();
	GLuint l = glGenLists(1);
	glNewList(l, GL_COMPILE);
	gluDisk(q, 0, 0.009, 16, 1);
	glEndList();

	// draw data using the display list
	for (entry = sim->particles->getFirst(); entry; entry = entry->getNext()) {
		for (i = -1; i <= 1; i++) for (j = -1; j <= 1; j++) if (entry->getThis()->r[1] + i < 1.3){
			if (i == 0 && j == 0) glColor4f(0.0f, 1.0f, 0.0f, 1.0f);
			else glColor4f(1.0f, 0.0f, 0.0f, 1.0f);
			glPushMatrix();
			glTranslated(entry->getThis()->r[1] + i, entry->getThis()->r[2] + j, 0);
			glCallList(l);
			glPopMatrix();
		}
	}

	// draw axes
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glBegin(GL_LINES); {
		glVertex2d(1.6, 0);  glVertex2d(1.6, 1);
		glVertex2d(1.6, 0);  glVertex2d(2.6, 0);
		// draw scale
		for (i = 1; i <= 10; i++){
			glVertex2d(1.6, i*0.1); glVertex2d(1.58, i*0.1);
			glVertex2d(1.6 + i*0.1, 0); glVertex2d(1.6 + i*0.1, -0.02);
		}
	} glEnd();

	// draw graph
	if (plotMode < 2){
		glBegin(GL_LINES); {
			graphData_t *graphData = sim->graphDataFirst->next;
			double t_ = 0;

			while (graphData->next) {
				switch (plotMode){
				case 0:
					glVertex2d(1.6 + t_, graphData->eKin / sim->graphDataFirst->eKin);
					t_ += (sim->getDt() / sim->getT()); graphData = graphData->next;
					glVertex2d(1.6 + t_, graphData->eKin / sim->graphDataFirst->eKin);
					break;
				case 1:
					double min = sim->getEPotMin();
					glVertex2d(1.6 + t_, (graphData->ePot-min) / (sim->graphDataFirst->ePot-min));
					t_ += (sim->getDt() / sim->getT()); graphData = graphData->next;
					glVertex2d(1.6 + t_, (graphData->ePot-min) / (sim->graphDataFirst->ePot-min));
				}
			}
		} glEnd();
	}

	if (plotMode == 2){
		glBegin(GL_QUADS); {
			mat radial = sim->getDirectionalDistribution();
			int hr = sim->getHistogramResolution();
			for (i = 0; i < hr; i++) for (j = 0; j < hr; j++){
				float* rgb = colorHue(radial[i+1][j+1]);
				glColor4f(rgb[0], rgb[1], rgb[2], 1.0f);
				glVertex2d(1.6 + (double)i / hr, (double)(j + 1.0) / hr); glVertex2d(1.6 + (double)i / hr, (double)j / hr);
				glVertex2d(1.6 + (double)(i + 1.0) / hr, (double)j / hr); glVertex2d( 1.6 +(double)(i + 1.0) / hr, (double)(j + 1.0) / hr);
			}
		}glEnd();
	}

	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	// write stuff
	glRasterPos2f(-0.2, -0.2);
	if (sim->pause) glutBitmapString(font, (unsigned char*)"pause");
	else glutBitmapString(font, (unsigned char*)"running");
	glRasterPos2f(-0.28, 1.25);
	char t_char[6]; sprintf_s(t_char, 6, "%04.3f", sim->getT());
	glutBitmapString(font, (unsigned char*)"t = ");
	glutBitmapString(font, (unsigned char*)t_char);
	glRasterPos2f(-0.28, 1.2);
	char temp_char[6]; sprintf_s(temp_char, 6, "%04.2f", sim->getThermostat()->getT());
	glutBitmapString(font, (unsigned char*)"T_soll = ");
	glutBitmapString(font, (unsigned char*)temp_char);
	glRasterPos2f(-0.28, 1.15);
	char nue_char[6]; sprintf_s(nue_char, 6, "%04.2f", sim->getThermostat()->getNue());
	glutBitmapString(font, (unsigned char*)"Rate = ");
	glutBitmapString(font, (unsigned char*)nue_char);

	// Graph labels
	glRasterPos2f(1.3, 1.06);
	char scale_char[10];
	switch (plotMode){
	case 0: glutBitmapString(font, (unsigned char*)"T_inst(t)\n");
		for (i = 10; i > 0; i--){
			glRasterPos2f(1.3, -0.01 + i * 0.1);
			sprintf_s(scale_char, 10, "%.2e", i*sim->graphDataFirst->eKin / (10.0*sim->particles->getLength()));
			glutBitmapString(font, (unsigned char*)scale_char);
		}break;
	case 1: glutBitmapString(font, (unsigned char*)"E_Pot(t)\n");
		double min = sim->getEPotMin();
		for (i = 10; i > 0; i--){
			glRasterPos2f(1.3, -0.01 + i * 0.1);
			double scale = min + (i*(sim->graphDataFirst->ePot - min) / 10.0);
			sprintf_s(scale_char, 10, "%2.2e", scale);
			glutBitmapString(font, (unsigned char*)scale_char);
		}
	}

	glutSwapBuffers();
}

void reshape(int w, int h){
	double xy = 0.5*(double)w / (h ? h : 1),
		lx = xy > 1 ? xy : 1, ly = 1 / xy > 1 ? 1 / xy : 1;
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);  glLoadIdentity();
	gluOrtho2D(0.5 - 0.8*lx, 0.5 + 2.4*lx, 0.5 - 0.8*ly, 0.5 + 0.8*ly);
}

// *************************************************************************************** ab hier weitermachen!!!!!

void keyboard(unsigned char key, int mx, int my){
	MDSim* sim = (MDSim*)glutGetWindowData();

	switch (key){
	case 27: glutLeaveMainLoop(); break;          // escape ends program
	case 112: sim->pause = !sim->pause; break;  // "p" toggles pause
	case 43: sim->getThermostat()->setT(sim->getThermostat()->getT()*SQR_2_5); break; // -/+ changes Temperature
	case 45: sim->getThermostat()->setT(sim->getThermostat()->getT() / SQR_2_5); break;
	case 114: sim->resetDirectionalDistribution(); break;					// "r" resets histogram
	case 103: plotMode = (plotMode == 0 ? 1 : (plotMode == 1 ? 2 : 0));	// "g" toggles graphs
	}
}

void step(int k){
	MDSim* sim = (MDSim*)glutGetWindowData();

	sim->getThermostat()->execute();
	sim->velocityVerletStep(true);
	if (!sim->pause) sim->updateGraphs();

	/*if (k == 100){
		mat histogram = sim->getRadialDistribution(true);
		int i, j;
		for (i = 0; i < histogram.get_cols(); i++) for (j = 0; j < histogram.get_rows(); j++)
			cout << fixed << setprecision(10) << i << " " << j << " " << histogram[i + 1][j + 1] << endl;
		cout << endl;
		k = 0;
	}
	else k++;*/

	glutTimerFunc(0, step, k);
	glutPostRedisplay();

}

vec f(vec r){
	double r2 = r*r;
	double r4 = r2*r2;
	double r8 = r4*r4;
	return 24.0 * SIGMA_6 * ((2 * SIGMA_6) / (r8*r4*r2) - 1 / r8) * r;
}

double pot(vec r){
	double r2 = r*r;
	double r4 = r2*r2;
	return 4.0 * SIGMA_6 * (SIGMA_6 / (r4*r4*r4) - 1 / (r4*r2));
}

int main(int argc, char* argv[]){	// Args: n, dt, nue, temp_start [, glut-Optionen]
	if (argc < 5) return 1;
	int n = atoi(argv[1]), i;
	double dt = atof(argv[2]), nue = atof(argv[3]), temp_start = atof(argv[4]);


	MDSim* sim = new MDSim(2, dt, f, pot, 0.15, 0.22, 10);
	for (i = 0; i < n; i++){
		sim->particles->addParticle(new MDParticle(2));
	}
	sim->getThermostat()->setT(temp_start);
	sim->getThermostat()->setNue(nue);
	double arr1[2] = { 1.0, 1.0 };
	sim->initSim(true, vec(arr1, 2),0, 0, 400, 0.15);

	return glutStuff(step, draw, reshape, keyboard, &argc, argv, sim, "MD - Basics");
}
