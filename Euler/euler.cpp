#if defined(__APPLE__) && defined(__MACH__)
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif
#include <stdio.h>
#include <cmath>

#define WIDTH 1280
#define HEIGHT 720

#define N 100

double a;

double x0 = 0;
double y_0 = 1.0;
double t0 = 0;
double dt = 10.0 / N;


double dx (double x, double y, double t)
{
    return y;
}

double dy (double x, double y, double t)
{
    return (-0.25)*y + 0.5*x - 0.5 * (x * x * x) + a*cos(0.2 * t);
}

void euler (double x0, double y0, double t0)
{
    double x = x0;
    double y = y0;
    double t = t0;
    
    glVertex2f(x, y);
    for (double tn = t + dt; tn <= 100; tn += dt) 
    {
        float xn = x +(tn - t)* dx(x, y, t);
        float yn = y + (tn - t) * dy (x, y, t);
        t = tn;
        
        //printf("xn = %f\nyn = %f\n\n", xn, yn);
        
        x = xn;
        y = yn;
        glVertex2f(xn, yn);
    }


}

void displayMe(void)
{
    glClearColor(1.0, 1.0, 1.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT); 
 
    glColor3f(0.0, 0.0, 0.0); 
    glBegin(GL_LINES);   
    glVertex2f(-WIDTH, 0);   
    glVertex2f(WIDTH, 0);   
    glEnd();
 
    glBegin(GL_LINES);
    glVertex2f(0, HEIGHT);
    glVertex2f(0, -HEIGHT);
    glEnd();
    
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0, 0.0, 0.0);
    euler(x0, y_0, t0);
    glEnd();
    glFlush();
}
 
int main(int argc, char** argv)
{
    scanf("%lf", &a);
    
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE);
    glutInitWindowSize(WIDTH, HEIGHT);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Euler");
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    int orthoW = a? 6 : 3;
    int orthoH = a? 3 : 2;
    glOrtho( -orthoW, orthoW, -orthoH, orthoH , -1.0, 1.0);
    glutDisplayFunc(displayMe);
    glutMainLoop();
    
    return 0;
}
