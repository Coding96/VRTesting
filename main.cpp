/* 
 * File:   main.cpp
 * Author: edward
 *
 * Created on 13 May 2017, 16:22
 */
#include <GL/glut.h>
#include <GL/glu.h>
#include <GL/gl.h>
#include <cstdlib>
#include <iostream>

//function declarations
void init(void);
void display(void);
void keyboard(unsigned char key, int x, int y);
void animate(void);
void reshape(int w, int h);

GLfloat angle = 0;
GLfloat colour1 = 1;
GLfloat colour2 = 0;
GLfloat colour3 = 0;

int pause = 0;

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

    glutSwapBuffers();
}

void init(void)
{
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
