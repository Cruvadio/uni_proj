#if defined(__APPLE__) && defined(__MACH__)
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#include <cmath>
#include <vector>
#include <iostream>

using namespace std;

#define WIDTH 1200
#define HEIGHT 400

#define A -1.5
#define B 1.5
#define C -0.5
#define D 0.5

#define K 10

#define DELTA 0.1

const int number_of_cells = ((B - A) * (D - C))/ (DELTA * DELTA);
const int rows = (B - A) / DELTA;
const int cols = (D - C) / DELTA;

int Scale = WIDTH / rows;
vector<vector<int> > grid(number_of_cells), gr(number_of_cells); 

void henon (double &xn, double &yn)
{
    const double a = 1.4;
    const double b = 0.3;
    double x = xn;
    double y = yn;

    xn = 1 - a* (x * x) + y;
    yn = b * x;
}

int return_cell (double x, double y)
{
    return (int)abs((y - D)/DELTA) * ((B - A) / DELTA) + (int)abs((x - A)/DELTA);
}

void interval (int cell, double& x1, double& y1)
{
    int row = cell / rows;
    int col = cell % rows;

    x1 = A + col * DELTA;

    y1 = D - row * DELTA;
}

vector<bool> used;
vector<int> order, component;
vector<vector<int> > components;
 
void dfs1 (int v) {
	used[v] = true;
	for (size_t i=0; i<grid[v].size(); ++i)
		if (!used[ grid[v][i] ])
			dfs1 (grid[v][i]);
	order.push_back (v);
}
 
void dfs2 (int v) {
	used[v] = true;
	component.push_back (v);
	for (size_t i=0; i<gr[v].size(); ++i)
		if (!used[ gr[v][i] ])
			dfs2 (gr[v][i]);
}


void find_components ()
{
    used.resize(number_of_cells);
    used.assign(number_of_cells, false);
    for (int i = 0; i < number_of_cells; i++)
        if (!used[i]) dfs1(i);
    used.assign(number_of_cells, false);

    for (int i = 0; i < number_of_cells; i++)
    {
        int v = order[number_of_cells - 1 - i];
        if (!used[v])
        {
            dfs2(v);
            if ((int)component.size() <= 1)
                continue;
            components.push_back(component);
            component.clear();
        }
    }
}

void make_graph()
{
    for (int i = 0; i < number_of_cells; i++)
    {
        double x1, y1;
        interval(i, x1, y1);
        for (int k = 0; k < K; k++)
        {
            double x = x1 + k * (DELTA / K);
            double y = y1 + k * (DELTA / K);

            henon(x, y);
            
            if (x < A || x > B || y < C || y > D)
                continue;

            int cell = return_cell(x , y);

            if (cell >= number_of_cells || cell < 0) continue;

            grid[i].push_back(cell);
            gr[cell].push_back(i);
        }
    }
}

void draw_square (int cell)
{
    glColor3f(0.0, 0.0, 1.0);
    int row = cell / rows;
    int col = cell % rows;
    glRectf(row * Scale, col * Scale, (row + 1) * Scale, (col + 1) * Scale);
}

void draw_grid()
{
    glColor3f(0.0, 0.0, 0.0);
    glBegin(GL_LINES);
    

    for (int i = 0; i < rows; i+= Scale)
    {
        glVertex2f(i, 0);
        glVertex2f(i, HEIGHT);
    }

    for (int i = 0; i < cols; i+= Scale)
    {
        glVertex2f(0, i);
        glVertex2f(WIDTH, i);
    }

    glEnd();
}

void display()
{
    glClearColor(1.0, 1.0, 1.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

   // draw_grid();
   /* 
    for (vector<int> comp : components)
        for (int v: comp)
        {
            draw_square(v);
        }
        */
}

int main (int argc, char* argv[])
{/*
    grid.resize(number_of_cells);
    gr.resize(number_of_cells);
    make_graph();
    find_components();
*/
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE);
    glutInitWindowSize(WIDTH, HEIGHT);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Symbol");
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho( - WIDTH, WIDTH, -HEIGHT, HEIGHT, -1.0, 1.0);
    glutDisplayFunc(display);
    glutMainLoop();

    return 0;
}

