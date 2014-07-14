#ifndef _App_h_
#define _App_h_

#include <GL/glew.h>
#include <SDL2/SDL.h>

class ShaderProgram;

class App {
public:
    App();
    int onExecute();
private:
    bool onInit();
    void onEvent(SDL_Event* event);
    void onLoop();
    void onRender();
    void onCleanup();

    bool fRunning;
    SDL_Window *fMainWindow;
    SDL_GLContext fGLContext;
    GLuint m_sceneVAO, m_postVAO;
    GLuint m_frameBuf;
    ShaderProgram* m_sceneShader, *m_postShader;

    // TEMP
    GLuint m_textures[3];
};

#endif
