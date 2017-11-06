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
#include "Matrices.h"
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

Matrix4 m_mat4ProjectionLeft;
Matrix4 m_mat4ProjectionRight;
Matrix4 m_mat4eyePosLeft;
Matrix4 m_mat4eyePosRight;

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
void RenderSteroTargets();
void RenderSceneByEye(vr::Hmd_Eye eye);
void SetupCameras();
Matrix4 GetHMDMatrixProjectionEye(vr::Hmd_Eye nEye);
Matrix4 GetHMDMatrixPoseEye( vr::Hmd_Eye nEye );
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
    //almost no setup until VR switch is made
    glClearColor(0.0, 0.0, 0.0, 0.0);
    
    
    
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
    
    glewExperimental = GL_TRUE;
    GLenum nGlewError = glewInit();
    
    vr::EVRInitError eError = vr::VRInitError_None;
    vrHMD = vr::VR_Init(&eError, vr::VRApplication_Scene);
    
    //variable holds chaperone data from driver
    chaperone = vr::VRChaperone();
    //variable holds render models from driver
    renderModels = (vr::IVRRenderModels *)vr::VR_GetGenericInterface(vr::IVRRenderModels_Version, &eError);
    
    //TODO**************************************************
    //setupScene();
    //setupCameras();
    
    setupSeperateRenderTargets();
    
    
    
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
    
    createFrameBuffer(RenderWidth,RenderHeight,leftEyeDesc);
    createFrameBuffer(RenderWidth,RenderHeight,rightEyeDesc);
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

void RenderSteroTargets()
{
        glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );
	glEnable( GL_MULTISAMPLE );

	// Left Eye
	glBindFramebuffer( GL_FRAMEBUFFER, leftEyeDesc.m_nRenderFramebufferId );
 	glViewport(0, 0, RenderWidth, RenderHeight );
 	RenderSceneByEye( vr::Eye_Left );
 	glBindFramebuffer( GL_FRAMEBUFFER, 0 );
	
	glDisable( GL_MULTISAMPLE );
	 	
 	glBindFramebuffer(GL_READ_FRAMEBUFFER, leftEyeDesc.m_nRenderFramebufferId);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, leftEyeDesc.m_nResolveFramebufferId );

        glBlitFramebuffer( 0, 0, RenderWidth, RenderHeight, 0, 0, RenderWidth, RenderHeight, 
		GL_COLOR_BUFFER_BIT,
 		GL_LINEAR );

 	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0 );	

	glEnable( GL_MULTISAMPLE );

	// Right Eye
	glBindFramebuffer( GL_FRAMEBUFFER, rightEyeDesc.m_nRenderFramebufferId );
 	glViewport(0, 0, RenderWidth, RenderHeight );
 	RenderSceneByEye( vr::Eye_Right );
 	glBindFramebuffer( GL_FRAMEBUFFER, 0 );
 	
	glDisable( GL_MULTISAMPLE );

 	glBindFramebuffer(GL_READ_FRAMEBUFFER, rightEyeDesc.m_nRenderFramebufferId );
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, rightEyeDesc.m_nResolveFramebufferId );
	
        glBlitFramebuffer( 0, 0, RenderWidth, RenderHeight, 0, 0, RenderWidth, RenderHeight, 
		GL_COLOR_BUFFER_BIT,
 		GL_LINEAR  );

 	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0 );
}

void RenderSceneByEye(vr::Hmd_Eye eye)
{
    
}

void SetupCameras()
{
        m_mat4ProjectionLeft = GetHMDMatrixProjectionEye( vr::Eye_Left );
	m_mat4ProjectionRight = GetHMDMatrixProjectionEye( vr::Eye_Right );
	m_mat4eyePosLeft = GetHMDMatrixPoseEye( vr::Eye_Left );
	m_mat4eyePosRight = GetHMDMatrixPoseEye( vr::Eye_Right );
}

Matrix4 GetHMDMatrixProjectionEye( vr::Hmd_Eye nEye )
{

	vr::HmdMatrix44_t mat = vrHMD->GetProjectionMatrix(nEye, 0.1f, 300.0f);

	return Matrix4(
		mat.m[0][0], mat.m[1][0], mat.m[2][0], mat.m[3][0],
		mat.m[0][1], mat.m[1][1], mat.m[2][1], mat.m[3][1],
		mat.m[0][2], mat.m[1][2], mat.m[2][2], mat.m[3][2],
		mat.m[0][3], mat.m[1][3], mat.m[2][3], mat.m[3][3]
		);
}
	
Matrix4 GetHMDMatrixPoseEye( vr::Hmd_Eye nEye )
{


	vr::HmdMatrix34_t matEyeRight = vrHMD->GetEyeToHeadTransform(nEye);
	Matrix4 matrixObj(
		matEyeRight.m[0][0], matEyeRight.m[1][0], matEyeRight.m[2][0], 0.0,
		matEyeRight.m[0][1], matEyeRight.m[1][1], matEyeRight.m[2][1], 0.0,
		matEyeRight.m[0][2], matEyeRight.m[1][2], matEyeRight.m[2][2], 0.0,
		matEyeRight.m[0][3], matEyeRight.m[1][3], matEyeRight.m[2][3], 1.0f
		);

	return matrixObj.invert();
}

void render()
{
    if(vrHMD)
    {
        RenderSteroTargets();
        
        vr::Texture_t leftEyeTexture = {(void*)(uintptr_t)leftEyeDesc.m_nResolveTextureId, vr::TextureType_OpenGL, vr::ColorSpace_Gamma };
        vr::VRCompositor()->Submit(vr::Eye_Left, &leftEyeTexture );

        vr::Texture_t rightEyeTexture = {(void*)(uintptr_t)rightEyeDesc.m_nResolveTextureId, vr::TextureType_OpenGL, vr::ColorSpace_Gamma };
        vr::VRCompositor()->Submit(vr::Eye_Right, &rightEyeTexture );
    }
}