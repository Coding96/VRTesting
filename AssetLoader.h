#include <GL/glut.h>
#include <GL/glu.h>
#include <GL/gl.h>

typedef struct{
    GLfloat vertex1;
    GLfloat vertex2;
    GLfloat vertex3;
    GLfloat vertex4;
    GLfloat vertex5;
    GLfloat vertex6;
    GLfloat vertex7;
    GLfloat vertex8;
}cube;

extern cube* cubeloader(void);
extern void sphereloader(void);


