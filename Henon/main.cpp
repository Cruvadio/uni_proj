#if defined(__APPLE__) && defined(__MACH__)
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif
#include <stdio.h>

#define WIDTH 1280
#define HEIGHT 720

#define N 15000

#define A 1.4
#define B 0.3

double x0 = 0;
double y0 = 0;




void displayMe(void)
{
    glClearColor(1, 1, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    glBegin(GL_POINTS);
    glColor3f(0,0,0);
        glVertex3f(x0 * 0.5625, y0 * 2, 0.0);
        double x = x0;
        double y = y0;
        for (int i = 0; i < N; i++)
        {
            double xn = 1 - A * (x * x) + y;
            double yn = B * x;
            
            glVertex3f(xn *  0.5625, yn * 2, 0.0);

            x = xn;
            y = yn;
        }
    glEnd();
    glFlush();
}
 
int main(int argc, char** argv)
{
    scanf("%lf%lf", &x0, &y0);
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE);
    glutInitWindowSize(WIDTH, HEIGHT);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Henon");
    glutDisplayFunc(displayMe);
    glutMainLoop();
    return 0;
}
