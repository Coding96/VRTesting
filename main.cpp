/* 
 * File:   main.cpp
 * Author: edward
 *
 * Created on 13 May 2017, 16:22
 */
///////////////////////////////////
#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <openvr.h>
///////////////////////////////////
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
///////////////////////////////////
#include <cstdlib>
#include <stdio.h>
#include <string>
#include <vector>
#include <iostream>
///////////////////////////////////
#include "AssetLoader.h"
///////////////////////////////////

//global variables
vr::IVRChaperone *chaperone;
vr::IVRRenderModels *renderModels;
vr::IVRSystem *vrHMD;
vr::TrackedDevicePose_t vrTrackedDevicePose[vr::k_unMaxTrackedDeviceCount];


struct FramebufferDesc
{
	GLuint m_nDepthBufferId;
	GLuint m_nRenderTextureId;
	GLuint m_nRenderFramebufferId;
	GLuint m_nResolveTextureId;
	GLuint m_nResolveFramebufferId;
};
FramebufferDesc leftEyeDesc;
FramebufferDesc rightEyeDesc;

uint32_t RenderWidth;
uint32_t RenderHeight;
//

//function declarations
void init(void);
void display(void);
void keyboard(unsigned char key, int x, int y);
void animate(void);
void reshape(int w, int h);
void render();
void setupSeperateRenderTargets();
void createFrameBuffer(int nWidth, int nHeight,FramebufferDesc &framebufferDesc);
//

GLfloat angle = 0;
GLfloat colour1 = 1;
GLfloat colour2 = 0;
GLfloat colour3 = 0;

int pause = 0;

using std::string;

int main(int argc, char** argv)
{
    

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE);
    glutInitWindowSize(500, 500);
    glutInitWindowPosition(100, 100);

    glutCreateWindow("VR Testing");
    init();
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutIdleFunc(animate);
    glutMainLoop();

    
    
    SDL_Quit();
    return EXIT_SUCCESS;
}

void display(void)
{
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();
    gluLookAt(0.0, 0.0, 5.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
    glRotatef(angle, 1, 0, 0);
    glRotatef(angle, 0, 1, 0);
    glRotatef(angle, 0, 0, 1);
    glColor3f(colour1, colour2, colour3);
    glutWireCube(2.0);

    glColor3f(colour2, colour1, colour3);
    glutSolidSphere(0.5,20,20);
    
    render();
                
    
    glutSwapBuffers();
}

void init(void)
{
    if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO) != 0) 
    {
        SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
        std::cout << "Unable to initialize SDL";
        return;
    }
    
    if(vr::VR_IsRuntimeInstalled() == false)
    {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "VR Runtime", "VR Runtime not found", NULL);
    }
    
    vr::EVRInitError eError = vr::VRInitError_None;
    vrHMD = vr::VR_Init(&eError, vr::VRApplication_Scene);
    
    //variable holds chaperone data from driver
    chaperone = vr::VRChaperone();
    //variable holds render models from driver
    renderModels = (vr::IVRRenderModels *)vr::VR_GetGenericInterface(vr::IVRRenderModels_Version, &eError);
    
    //almost no setup until VR switch is made
    glClearColor(0.0, 0.0, 0.0, 0.0);
    
    
}

void animate(void)
{
    //works an idle function

    if (pause == 0)
    {
        angle += 0.8;

        if (colour1 < 1)
        {
            colour1 += 0.007;
        }
        else
        {
            colour1 = 0;
        }
        if (colour2 < 1)
        {
            colour2 += 0.0211;
        }
        else
        {
            colour2 = 0;
        }
        if (colour3 < 1)
        {
            colour3 += 0.0288;
        }
        else
        {
            colour3 = 0;
        }
    }
    glutPostRedisplay();
}

void keyboard(unsigned char key, int x, int y)
{
    using std::cout;

    switch (key)
    {
    case 27: /* Escape key */
        vr::VR_Shutdown();
        exit(0);
    case 112:
        pause = !pause;
        break;
    default:
        cout << "You pressed " << key << "\n";
        break;
    }
}

void reshape(int w, int h)
{
    /*called when windows is moved or resized etc..*/
    glViewport(0, 0, (GLsizei) w, (GLsizei) h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60, (GLfloat) w / (GLfloat) h, 1.0, 100.0);
    glMatrixMode(GL_MODELVIEW);
}

void setupSeperateRenderTargets()
{
    vrHMD->GetRecommendedRenderTargetSize(&RenderWidth,&RenderHeight);
}

void createFrameBuffer(int nWidth, int nHeight,FramebufferDesc &framebufferDesc)
{
    //credit value inc for glew framebuffer technique
        glGenFramebuffers(1, &framebufferDesc.m_nRenderFramebufferId);
	glBindFramebuffer(GL_FRAMEBUFFER, framebufferDesc.m_nRenderFramebufferId);

	glGenRenderbuffers(1, &framebufferDesc.m_nDepthBufferId);
	glBindRenderbuffer(GL_RENDERBUFFER, framebufferDesc.m_nDepthBufferId);
	glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH_COMPONENT, nWidth, nHeight);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, framebufferDesc.m_nDepthBufferId);

	glGenTextures(1, &framebufferDesc.m_nRenderTextureId);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, framebufferDesc.m_nRenderTextureId);
	glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGBA8, nWidth, nHeight, true);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, framebufferDesc.m_nRenderTextureId, 0);

	glGenFramebuffers(1, &framebufferDesc.m_nResolveFramebufferId);
	glBindFramebuffer(GL_FRAMEBUFFER, framebufferDesc.m_nResolveFramebufferId);

	glGenTextures(1, &framebufferDesc.m_nResolveTextureId);
	glBindTexture(GL_TEXTURE_2D, framebufferDesc.m_nResolveTextureId);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, nWidth, nHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, framebufferDesc.m_nResolveTextureId, 0);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void render()
{
    if(vrHMD)
    {
        vr::Texture_t leftEyeTexture = {(void*)(uintptr_t)leftEyeDesc.m_nResolveTextureId, vr::TextureType_OpenGL, vr::ColorSpace_Gamma };
        vr::VRCompositor()->Submit(vr::Eye_Left, &leftEyeTexture );

        vr::Texture_t rightEyeTexture = {(void*)(uintptr_t)rightEyeDesc.m_nResolveTextureId, vr::TextureType_OpenGL, vr::ColorSpace_Gamma };
        vr::VRCompositor()->Submit(vr::Eye_Right, &rightEyeTexture );
    }
}