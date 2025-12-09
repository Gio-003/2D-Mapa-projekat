#include "./Header/Input.h"
#include <GLFW/glfw3.h>
#include <algorithm>
#include <cmath>
#include <vector>

std::vector<MeasurePoint> measurePoints;
float measureDistance = 0.0f;
double totalDistance = 0.0;
const double mapScaleFactor = 10000.0; // Faktor konverzije UV koordinata u "metre"

// --- Globals ---
float camX = 0.5f, camY = 0.5f; //kamera podesena na centar mape
float camSpeed = 0.0015f;
const float zoomVal = 0.1f; // Povecano zumiranje (manji broj = veci zoom) za "walking mode" (10/100 mape)
int screenWidth = 800, screenHeight = 800;
MeasurePoint mouseToXY(double mx, double my)
{
    // Normalizacija da bi se pozicija izražena u pikselima namapirala na OpenGL-ov prozor sa opsegom (-1, 1)

    MeasurePoint p;
    p.x = static_cast<float>(mx) / static_cast<float>(screenWidth);
    p.y = 1.0f - (static_cast<float>(my) / static_cast<float>(screenHeight));
    return p;
}
AppMode mode = MODE_WALKING;
void processKeyboardInput(GLFWwindow* window) {

    static bool rWasPressed = false;
    //if (key == GLFW_KEY_S && action == GLFW_PRESS) {
    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
    {
        if (!rWasPressed) {
            mode = (mode == MODE_WALKING ? MODE_MEASURING : MODE_WALKING);
        }
        rWasPressed = true;
    }
    else {
        rWasPressed = false;
    }
    if (mode == MODE_WALKING) {
        float oldX = camX;
        float oldY = camY;
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) camY += camSpeed;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) camY -= camSpeed;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) camX -= camSpeed;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) camX += camSpeed;


        // Ogranicavanje kretanja unutar mape
        float halfZoom = zoomVal * 0.5f;
        camY = std::max(halfZoom, std::min(camY, 1.0f - halfZoom));
        camX = std::max(halfZoom, std::min(camX, 1.0f - halfZoom));



        // --- Racunanje predjene distance ---
        float dx = camX - oldX;
        float dy = camY - oldY;
        // Pitagorina teorema
        double segment = std::sqrt(dx * dx + dy * dy);
        totalDistance += segment * mapScaleFactor;
    }
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}
void processMouse(GLFWwindow* window) {
    static bool leftWasDown = false;

    bool leftNow = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;

    if (leftNow && !leftWasDown) {

        double mx, my;
        glfwGetCursorPos(window, &mx, &my);

        // Kao sa vezbi
        float x = (float(mx) / screenWidth) * 2.0f - 1.0f;
        float y = 1.0f - (float(my) / screenHeight) * 2.0f;

        // Klik na switch ikonicu
        if (x > 0.80f && x < 0.97f && y > 0.80f && y < 0.97f) {
            mode = (mode == MODE_WALKING ? MODE_MEASURING : MODE_WALKING);
        }
        else {
            if (mode == MODE_MEASURING) {

                int idx = findPointIndexNear(mx, my);

                if (idx != -1) {
                    removeMeasurePoint(idx);
                }
                else {
                    addMeasurePoints(mouseToXY(mx, my));
                }
            }
        }
    }

    leftWasDown = leftNow;   // pamti stanje za sledeći frejm
}
int findPointIndexNear(double mx, double my)
{
    const float radius = 15.0f; // Radijus oko kursora za detekciju
    for (int i = 0; i < measurePoints.size(); ++i)
    {
        float px = measurePoints[i].x * screenWidth;
        float py = (1.0f - measurePoints[i].y) * screenHeight; // Invert Y za ekran
        float dist = std::sqrt((px - mx) * (px - mx) + (py - my) * (py - my));//Euklidska distanca zapamti
        if (dist < radius) return i;
    }
    return -1;
}
void addMeasurePoints(MeasurePoint p)
{
    if (measurePoints.empty())
    {
        measurePoints.push_back(p);
        return;
    }
    MeasurePoint lastPoint = measurePoints.back();
    float dx = p.x - lastPoint.x;
    float dy = p.y - lastPoint.y;
    measureDistance += std::sqrt(dx * dx + dy * dy) * mapScaleFactor;
    measurePoints.push_back(p);

}
void removeMeasurePoint(int index)
{
    if (index < 0 || index >= measurePoints.size()) return;
    if (index > 0)
    {
        MeasurePoint lastPoint = measurePoints[index - 1];
        MeasurePoint removedPoint = measurePoints[index];
        float dx = removedPoint.x - lastPoint.x;
        float dy = removedPoint.y - lastPoint.y;
        measureDistance -= std::sqrt(dx * dx + dy * dy) * mapScaleFactor;
    }
    if (index < measurePoints.size() - 1)
    {
        MeasurePoint next = measurePoints[index + 1];
        MeasurePoint curr = measurePoints[index];
        float dx = next.x - curr.x;
        float dy = next.y - curr.y;
        measureDistance -= std::sqrt((dx * dx) + (dy * dy)) * mapScaleFactor;
    }
    if (index > 0 && index < measurePoints.size() - 1)
    {
        MeasurePoint prev = measurePoints[index - 1];
        MeasurePoint next = measurePoints[index + 1];
        float dx = next.x - prev.x;
        float dy = next.y - prev.y;
        measureDistance += std::sqrt((dx * dx) + (dy * dy)) * mapScaleFactor;
    }
    measurePoints.erase(measurePoints.begin() + index);
}
/*void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    static bool leftWasDown = false;

    if (button == GLFW_MOUSE_BUTTON_LEFT)
    {
        if (action == GLFW_PRESS && !leftWasDown)
        {
            double mx, my;
            glfwGetCursorPos(window, &mx, &my);

            // Pretvaranje u NDC
            float x = (float(mx) / screenWidth) * 2.0f - 1.0f;
            float y = 1.0f - (float(my) / screenHeight) * 2.0f;

            // Klik na ikonicu
            if (x > 0.80f && x < 0.97f && y > 0.80f && y < 0.97f)
            {
                mode = (mode == MODE_WALKING ? MODE_MEASURING : MODE_WALKING);
            }
            else
            {
                if (mode == MODE_MEASURING)
                {
                    int idx = findPointIndexNear(mx, my);

                    if (idx != -1)
                        removeMeasurePoint(idx);
                    else
                        addMeasurePoints(mouseToXY(mx, my));
                }
            }
        }

        // update previous state
        if (action == GLFW_PRESS) leftWasDown = true;
        if (action == GLFW_RELEASE) leftWasDown = false;
    }
}
*/
/*void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    static bool rWasPressed = false;

    // --- R toggle ---
    if (key == GLFW_KEY_R)
    {
        if (action == GLFW_PRESS && !rWasPressed)
        {
            mode = (mode == MODE_WALKING ? MODE_MEASURING : MODE_WALKING);
            rWasPressed = true;
        }
        else if (action == GLFW_RELEASE)
        {
            rWasPressed = false;
        }
        return;
    }

    // --- ESC zatvaranje ---
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, true);
        return;
    }

    // --- Kretanje (samo u WALKING modu) ---
    if (mode == MODE_WALKING)
    {
        float oldX = camX;
        float oldY = camY;

        if (action == GLFW_PRESS || action == GLFW_REPEAT)
        {
            if (key == GLFW_KEY_W) camY += camSpeed;
            if (key == GLFW_KEY_S) camY -= camSpeed;
            if (key == GLFW_KEY_A) camX -= camSpeed;
            if (key == GLFW_KEY_D) camX += camSpeed;
        }

        // Granice
        float halfZoom = zoomVal * 0.5f;
        camY = std::max(halfZoom, std::min(camY, 1.0f - halfZoom));
        camX = std::max(halfZoom, std::min(camX, 1.0f - halfZoom));

        // Distance
        float dx = camX - oldX;
        float dy = camY - oldY;
        double segment = std::sqrt(dx * dx + dy * dy);
        totalDistance += segment * mapScaleFactor;
    }
}
*/