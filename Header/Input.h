#ifndef INPUT_H
#define INPUT_H


#include <vector>
#include <GLFW/glfw3.h>


// Tipovi koji se koriste za input/measure
enum AppMode {
	MODE_WALKING,
	MODE_MEASURING
};


struct MeasurePoint {
	float x;
	float y;
};


// Globals (definirane u Input.cpp)
extern std::vector<MeasurePoint> measurePoints;
extern float measureDistance;
extern double totalDistance;
extern const double mapScaleFactor;


extern AppMode mode;


extern float camX;
extern float camY;
extern float camSpeed;
extern const float zoomVal;


extern int screenWidth;
extern int screenHeight;


// Funkcije vezane za interakciju sa misem i tastaturom
void processInput(GLFWwindow* window);
void processMouse(GLFWwindow* window);
int findPointIndexNear(double mx, double my);
void addMeasurePoints(MeasurePoint p);
void removeMeasurePoint(int index);
MeasurePoint mouseToXY(double mx, double my);


#endif // INPUT_H