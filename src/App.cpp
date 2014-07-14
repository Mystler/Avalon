#include "App.h"
#include "ShaderProgram.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <SDL2/SDL_image.h>

static const float vertices[] = {
    -0.5f, 0.5f, -1.f, 0.f, 0.f, // Top-left
    0.5f, 0.5f, 1.f, 1.f, 0.f, // Top-right
    0.5f, -0.5f, 0.f, 1.f, 1.f, // Bottom-right
    -0.5f, -0.5f, 1.f, 0.f, 1.f  // Bottom-left
};

static const GLuint indices[] = {
    0, 1, 2,
    2, 3, 0,
    1, 3, 0,
    1, 3, 2
};

static const float screenVerts[] = {
    -1.0f, 1.0f, 0.0f, 1.0f,
    1.0f, 1.0f, 1.0f, 1.0f,
    1.0f, -1.0f, 1.0f, 0.0f,

    1.0f, -1.0f, 1.0f, 0.0f,
    -1.0f, -1.0f, 0.0f, 0.0f,
    -1.0f, 1.0f, 0.0f, 1.0f
};

App::App()
 : fRunning(true), m_sceneShader(nullptr), m_postShader(nullptr)
{
}

int App::onExecute()
{
    if (!onInit()) {
        onCleanup();
        return -1;
    }
    SDL_Event event;
    while (fRunning) {
        while (SDL_PollEvent(&event))
            onEvent(&event);
        onLoop();
        onRender();
    }
    onCleanup();
    return 0;
}

bool App::onInit()
{
    // SDL Setup
    if (SDL_Init(SDL_INIT_VIDEO) < 0 || IMG_Init(IMG_INIT_JPG) < 0)
        return false;

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 8);

    // Create the window
    fMainWindow = SDL_CreateWindow("SDL + OpenGL Test", SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED, 640, 480, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
    if (!fMainWindow)
        return false;

    // Create the OpenGL Context
    fGLContext = SDL_GL_CreateContext(fMainWindow);
    SDL_GL_SetSwapInterval(1);
    glewExperimental = true;
    if (glewInit() != GLEW_OK)
        return false;
    SDL_Log("SDL-Error: %s", SDL_GetError());
    SDL_Log("GLEW-Error: %s", glewGetErrorString(glGetError()));
    SDL_Log("GL Version: %s", glGetString(GL_VERSION));
    if (!GLEW_VERSION_3_2)
        return false;

    // Create VAO
    glGenVertexArrays(1, &m_sceneVAO);
    glBindVertexArray(m_sceneVAO);

    // Load object buffers
    GLuint vertBuf;
    glGenBuffers(1, &vertBuf);
    glBindBuffer(GL_ARRAY_BUFFER, vertBuf);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    GLuint elemBuf;
    glGenBuffers(1, &elemBuf);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elemBuf);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Create shaders
    m_sceneShader = new ShaderProgram();
    if (!m_sceneShader->load(GL_VERTEX_SHADER, "shaders/v_default.glsl"))
        SDL_Log("Error loading vertex shader: %s", m_sceneShader->errorMsg());
    if (!m_sceneShader->load(GL_FRAGMENT_SHADER, "shaders/f_default.glsl"))
        SDL_Log("Error loading fragment shader: %s", m_sceneShader->errorMsg());
    if (!m_sceneShader->link()) {
        SDL_Log("Error linking shader program: %s", m_sceneShader->errorMsg());
        return false;
    }
    m_sceneShader->use();

    // Configure VS
    GLint posAttrib = m_sceneShader->attribLocation("a_pos");
    glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 0);
    glEnableVertexAttribArray(posAttrib);
    GLint texCoordAttrib = m_sceneShader->attribLocation("a_uv");
    glVertexAttribPointer(texCoordAttrib, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(texCoordAttrib);

    // Load object textures
    glGenTextures(1, &m_textures[1]);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_textures[1]);
    SDL_Surface* texture = IMG_Load("M:/l4gmod1.jpg"); // Random image from my HDD for testing
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texture->w, texture->h, 0, GL_RGB, GL_UNSIGNED_BYTE, texture->pixels);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    static const float borderCol[] = {1.f, 0.f, 0.f, 1.f};
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderCol);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    SDL_FreeSurface(texture);
    glUniform1i(m_sceneShader->uniformLocation("u_tex1"), 0);
    glGenerateMipmap(GL_TEXTURE_2D);
    glGenTextures(1, &m_textures[2]);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_textures[2]);
    texture = IMG_Load("M:/DrizzleErrors.jpg"); // Random image from my HDD for testing
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texture->w, texture->h, 0, GL_RGB, GL_UNSIGNED_BYTE, texture->pixels);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderCol);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    SDL_FreeSurface(texture);
    glUniform1i(m_sceneShader->uniformLocation("u_tex2"), 1);
    glGenerateMipmap(GL_TEXTURE_2D);
    IMG_Quit();

    glm::mat4 view = glm::lookAt(glm::vec3(2.f, 2.f, 2.f), glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.0f, 0.0f, 1.0f));
    glUniformMatrix4fv(m_sceneShader->uniformLocation("u_view"), 1, GL_FALSE, glm::value_ptr(view));

    glm::mat4 proj = glm::perspective(45.0f, 640.f / 480.f, .5f, 10.f);
    glUniformMatrix4fv(m_sceneShader->uniformLocation("u_proj"), 1, GL_FALSE, glm::value_ptr(proj));

    // Post processing setup
    glGenVertexArrays(1, &m_postVAO);
    glBindVertexArray(m_postVAO);

    GLuint postBuf;
    glGenBuffers(1, &postBuf);
    glBindBuffer(GL_ARRAY_BUFFER, postBuf);
    glBufferData(GL_ARRAY_BUFFER, sizeof(screenVerts), screenVerts, GL_STATIC_DRAW);

    m_postShader = new ShaderProgram();
    if (!m_postShader->load(GL_VERTEX_SHADER, "shaders/v_post.glsl"))
        SDL_Log("Error loading vertex shader: %s", m_postShader->errorMsg());
    if (!m_postShader->load(GL_FRAGMENT_SHADER, "shaders/f_post.glsl"))
        SDL_Log("Error loading fragment shader: %s", m_postShader->errorMsg());
    if (!m_postShader->link()) {
        SDL_Log("Error linking post shader program: %s", m_postShader->errorMsg());
        return false;
    }
    m_postShader->use();

    posAttrib = m_sceneShader->attribLocation("a_pos");
    glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glEnableVertexAttribArray(posAttrib);
    texCoordAttrib = m_sceneShader->attribLocation("a_uv");
    glVertexAttribPointer(texCoordAttrib, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(texCoordAttrib);

    glGenFramebuffers(1, &m_frameBuf);
    glBindFramebuffer(GL_FRAMEBUFFER, m_frameBuf);
    glGenTextures(1, &m_textures[0]);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_textures[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 640, 480, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_textures[0], 0);
    GLuint rboDepth;
    glGenRenderbuffers(1, &rboDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 640, 480);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rboDepth);

    glUniform1i(m_sceneShader->uniformLocation("u_tex"), 0);

    return true;
}

void App::onEvent(SDL_Event* event)
{
    if (event->type == SDL_QUIT)
        fRunning = false;
}

void App::onLoop()
{
}

void App::onRender()
{
    // Render to FBO
    glBindFramebuffer(GL_FRAMEBUFFER, m_frameBuf);
    glBindVertexArray(m_sceneVAO);
    glEnable(GL_DEPTH_TEST);
    m_sceneShader->use();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_textures[1]);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_textures[2]);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Draw scene
    glm::quat rot = glm::angleAxis<float>(SDL_GetTicks() / 10.f, glm::vec3(0.f, 0.f, 1.f));
    glm::mat4 modelMatrix = glm::mat4_cast(rot);

    GLint uniTrans = m_sceneShader->uniformLocation("u_model");
    glUniformMatrix4fv(uniTrans, 1, GL_FALSE, glm::value_ptr(modelMatrix));

    glDrawElements(GL_TRIANGLES, sizeof(indices) / sizeof(GLuint), GL_UNSIGNED_INT, 0);

    // Post processing
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindVertexArray(m_postVAO);
    glDisable(GL_DEPTH_TEST);
    m_postShader->use();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_textures[0]);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    SDL_GL_SwapWindow(fMainWindow);
}

void App::onCleanup()
{
    glDeleteFramebuffers(1, &m_frameBuf);
    if (m_postShader)
        delete m_postShader;
    if (m_sceneShader)
        delete m_sceneShader;
    SDL_GL_DeleteContext(fGLContext);
    SDL_DestroyWindow(fMainWindow);
    SDL_Quit();
}
