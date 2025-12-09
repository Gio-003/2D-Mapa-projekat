#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "../Header/Util.h" 
#include "../Header/Create.h"
#include "../Header/Input.h"
#include <cstdio>
#include <thread>
#include <chrono>
#include <string>
#include <cmath>


// Globals (buffers, textures, shaders, uniform locations)
GLuint vaoFull, vaoNDC;
GLuint mapTex, pinTex, infoTex, digitsTex, iconWalkingTex, iconMeasuringTex, dotTex;
GLuint mapShader, texShader, lineShader;
// Uniform locations
//primer loc_tex_pos = glGetUniformLocation(texShader, "uPos");
GLint loc_map_uTex, loc_map_camCenter, loc_map_zoom;
GLint loc_tex_uTex, loc_tex_pos, loc_tex_size, loc_tex_alpha;
//scale = koliko veliki deo teksture koristiš
//offset=gde taj isečeni deo počinje
GLint loc_tex_uvScale, loc_tex_uvOffset;

const double targetFrameTime = 1.0 / 75.0; // 75 fps

GLFWwindow* initWindow() {
  //Funckije kao sa vezbi za formiranje prozora sa prikaz u fullscreen
     // Inicijalizacija GLFW i postavljanje na verziju 3 sa programabilnim pajplajnom
    if (!glfwInit()) return nullptr;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);
    //sirina i visina monitora
    screenWidth = mode->width; screenHeight = mode->height; // Fullscreen
    GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight, "Projekat 01 Mapa", monitor, NULL);
    if (!window) return nullptr;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(0);
    //Inicijalizacija GLEW
    if (glewInit() != GLEW_OK) return nullptr;
    //Naglaseno da program koristi alfa kanal za providnost
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    return window;
}
//Kada korisnik promeni veličinu prozora, parametri w i h su nove dimenzije
void framebufferCallback(GLFWwindow* window, int w, int h) {
    screenWidth = w; screenHeight = h;
    glViewport(0, 0, w, h); 
}
//Za sve draw funkcije koje nisu drawDistanceNumber vratimo Scale i Offset na standardne vrednosti
void resetUVProps() {
    glUniform2f(loc_tex_uvScale, 1.0f, 1.0f);
    glUniform2f(loc_tex_uvOffset, 0.0f, 0.0f);
}
//Crtanje linija u rezimu merenja razdaljine
void drawMeasureLines()
{
    if (measurePoints.size() < 2) return;

    glUseProgram(lineShader);//sejder za linije
    std::vector<float> verts;
    verts.reserve(measurePoints.size() * 2);

    for (auto& p : measurePoints)
    {
        // Normalizacija da bi se pozicija izražena u pikselima namapirala na OpenGL-ov prozor sa opsegom (-1, 1)
        float x = p.x * 2.0f - 1.0f;
        float y = p.y * 2.0f - 1.0f;
        verts.push_back(x);
        verts.push_back(y);
    }

    GLuint vbo, vao;
    glGenBuffers(1, &vbo);
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(float), verts.data(), GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
    //Debljina linije
    glLineWidth(4.0f);
    //Crtanje linija kao sa vezbi
    glDrawArrays(GL_LINE_STRIP, 0, measurePoints.size());

    // Cleanup
    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);
}
//crtanje tacaka u rezimu merenja razdaljina
void drawMeasureDots()
{
    glUseProgram(texShader);
    glBindTexture(GL_TEXTURE_2D, dotTex); // dot.png

    for (auto& p : measurePoints)
    {
        // Normalizacija da bi se pozicija izražena u pikselima namapirala na OpenGL-ov prozor sa opsegom (-1, 1)
        float x = p.x * 2.0f - 1.0f;
        float y = p.y * 2.0f - 1.0f;

        //Postavljamo poziciju,velicinu i alpha ,scale i offset za tacke
        glUniform2f(loc_tex_pos, x, y);
        glUniform2f(loc_tex_size, 0.03f, 0.05f);
        glUniform1f(loc_tex_alpha, 1.0f);
        resetUVProps();
        // Podešavanje da se crta koristeći vertekse četvorougla
        glBindVertexArray(vaoNDC);
        // Crtaju se trouglovi tako da formiraju četvorougao
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    }
}


// Funkcija za iscrtavanje mape
void drawMap() {
    glUseProgram(mapShader);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, mapTex);
    glUniform1i(loc_map_uTex, 0);
    if (mode == MODE_WALKING) {
        glUniform2f(loc_map_camCenter, camX, camY);//centar mape
        glUniform1f(loc_map_zoom, zoomVal);//zoomVal prikaz 10/100 mape
    }
    else {
        // U measure mode prikazujemo celu mapu
        glUniform2f(loc_map_camCenter, 0.5f, 0.5f);//centar mape
        glUniform1f(loc_map_zoom, 1.0f);  // vidi celu mapu
    }
    glBindVertexArray(vaoFull);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);// Crtaju se trouglovi tako da formiraju četvorougao
}
//Funckija za Pin u rezimu hodanja
void drawPin() {
    glUseProgram(texShader);
    resetUVProps(); // Pin koristi celu teksturu tj scale i offset vracamo na standardne vrednosti

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, pinTex);
    glUniform1i(loc_tex_uTex, 0);

    float aspect = (float)screenWidth / screenHeight;
    float pinH = 0.24f;
    float pinW = 0.12f;

    glUniform2f(loc_tex_size, pinW * aspect * 0.5f, pinH * 0.5f * aspect * 0.5f);
    glUniform2f(loc_tex_pos, 0.0f, 0.0f); // Centar ekrana
    glUniform1f(loc_tex_alpha, 1.0f);

    glBindVertexArray(vaoNDC);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

void drawModeIcon(unsigned int modeTex) {
    // Iscrtavanje ikonice "Walking Mode" u gornjem desnom uglu
    glUseProgram(texShader);
    resetUVProps();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, modeTex);
    glUniform1i(loc_tex_uTex, 0);

    float aspect = (float)screenWidth / screenHeight;
    float iconSize = 0.25f;

    // Pozicija: Gornji desni ugao
    float posX = 0.85f;
    float posY = 0.85f;
    //proizvoljan izbor velicine
    glUniform2f(loc_tex_size, (iconSize) * 0.5f * aspect * 0.5f, iconSize * 0.5f * aspect * 0.5f);
    glUniform2f(loc_tex_pos, posX, posY);
    glUniform1f(loc_tex_alpha, 1.0f);

    glBindVertexArray(vaoNDC);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

// Funkcija koja ispisuje broj koristeci teksturu sa ciframa 0-9
void drawDistanceNumbers() {
    glUseProgram(texShader);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, digitsTex);
    glUniform1i(loc_tex_uTex, 0);
    glUniform1f(loc_tex_alpha, 1.0f);

    int distInt = (mode == MODE_WALKING)
        ? (int)totalDistance
        : (int)measureDistance;

    std::string s = std::to_string(distInt);

    float aspect = (float)screenWidth / screenHeight;
    float charHeight = 0.1f;
    float charWidth = charHeight * aspect * 0.5f; // Procena sirine broja

    // Pocetna pozicija (donji levi ugao)
    float startX = -0.9f;
    float startY = -0.8f;

    // Sprite sheet podesavanja (tekstura ima 10 cifara horizontalno)
    // Svaka cifra zauzima 1/10 sirine teksture (0.1)
    glUniform2f(loc_tex_uvScale, 0.1f, 1.0f);

    for (int i = 0; i < s.length(); ++i) {
        char c = s[i];
        int digit = c - '0'; // ASCII trik: '3' - '0' = int 3

        // Ako nije broj (npr 'm'), preskoci
        if (digit < 0 || digit > 9) continue;

        // Racunamo offset: ako je broj 3, offset je 0.3
        glUniform2f(loc_tex_uvOffset, (float)digit * 0.1f, 0.0f);

        // Pozicija kvad-a
        glUniform2f(loc_tex_size, charWidth * 0.4f, charHeight * 0.5f);
        glUniform2f(loc_tex_pos, startX + (i * charWidth), startY);

        glBindVertexArray(vaoNDC);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    }
}
void drawInfoBox() {
    glUseProgram(texShader);
    resetUVProps();
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, infoTex);
    glUniform1i(loc_tex_uTex, 0);

    float aspect = (float)screenWidth / screenHeight;

    // --- PODESAVANJE VELICINE ---
    float width = 1.0f;
    float height = 1.0f;
    // --- PODESAVANJE POZICIJE (DONJI DESNI UGAO) ---
    float margin = 0.05f; // Odmak od ivice ekrana

    // X: Desna ivica (1.0) minus margina minus polovina sirine slike
    float x = 1.0f - margin - (width * 0.2f);

    // Y: Donja ivica (-1.0) plus margina plus polovina visine slike
    float y = -1.0f + margin + (height * 0.2f);

    // Saljemo u shader
    glUniform2f(loc_tex_size, width * 0.7f * aspect * 0.5f, height * 0.7f * aspect * 0.5f);
    glUniform2f(loc_tex_pos, x, y);
    //Podesavanje providnosti
    glUniform1f(loc_tex_alpha, 0.9f);

    glBindVertexArray(vaoNDC);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}



int main() {
    GLFWwindow* window = initWindow();
    if (!window) return endProgram("Window init failed.");
    // glfwSetFramebufferSizeCallback(window, framebufferCallback);

    GLFWcursor* cursor = loadImageToCursor("Resources/compas1.png");
    if (cursor) glfwSetCursor(window, cursor);

    vaoFull = createFullscreenQuad();
    vaoNDC = createNDCQuad();

    // Ucitavanje tekstura(funkcija dodata u util.h jer je funckija sa vezbi)
    preprocessTexture(mapTex, "Resources/map.jpg");
    preprocessTexture(pinTex, "Resources/ping.png");
    preprocessTexture(infoTex, "Resources/nesto.png.PNG");
    preprocessTexture(digitsTex, "Resources/digits.png");
    preprocessTexture(iconWalkingTex, "Resources/walking.png");
    preprocessTexture(iconMeasuringTex, "Resources/lenjir.png");
    preprocessTexture(dotTex, "Resources/dot1.png");


    mapShader = createShader("Shaders/map.vert", "Shaders/map.frag");
    texShader = createShader("Shaders/tex.vert", "Shaders/tex.frag");
    lineShader = createShader("Shaders/line.vert", "Shaders/line.frag");


    // Map shader uniforms
    loc_map_uTex = glGetUniformLocation(mapShader, "uTexture");
    loc_map_camCenter = glGetUniformLocation(mapShader, "uCamCenter");
    loc_map_zoom = glGetUniformLocation(mapShader, "uZoom");

    // Tex shader uniforms
    loc_tex_uTex = glGetUniformLocation(texShader, "uTexture");
    loc_tex_pos = glGetUniformLocation(texShader, "uPos");
    loc_tex_size = glGetUniformLocation(texShader, "uSize");
    loc_tex_alpha = glGetUniformLocation(texShader, "uAlpha");
    loc_tex_uvScale = glGetUniformLocation(texShader, "uUVScale");
    loc_tex_uvOffset = glGetUniformLocation(texShader, "uUVOffset");

    // Inicijalizacija uniformi da ne bi bile 0
   // glUseProgram(texShader);
   // glUniform2f(loc_tex_uvScale, 1.0f, 1.0f);
  //  glUniform2f(loc_tex_uvOffset, 0.0f, 0.0f);

    while (!glfwWindowShouldClose(window)) {
        double frameStart = glfwGetTime();

        processKeyboardInput(window);

        glClearColor(0.12f, 0.14f, 0.17f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        drawMap();
        if (mode == MODE_WALKING) {

            drawModeIcon(iconWalkingTex);
            drawPin();
            drawInfoBox();
            drawDistanceNumbers();
        }
        else {
            if (mode == MODE_MEASURING)
            {
                drawModeIcon(iconMeasuringTex);
                drawInfoBox();
                drawMeasureLines();
                drawMeasureDots();
                drawDistanceNumbers();
            }


        }
        glfwSwapBuffers(window);
        glfwPollEvents();
        double const FRAME_TIME = 75.0f;
        // Mouse toggle mode
        processMouse(window);
        double elapsed = glfwGetTime() - frameStart;
        while (elapsed < targetFrameTime)
        {
            elapsed= glfwGetTime() - frameStart;
        }
    }


    glDeleteVertexArrays(1, &vaoFull);
    glDeleteVertexArrays(1, &vaoNDC);
    glDeleteProgram(mapShader);
    glDeleteProgram(texShader);
    if (cursor) glfwDestroyCursor(cursor);
    glDeleteTextures(1, &mapTex);
    glDeleteTextures(1, &pinTex);
    glDeleteTextures(1, &infoTex);
    glDeleteTextures(1, &digitsTex);

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}