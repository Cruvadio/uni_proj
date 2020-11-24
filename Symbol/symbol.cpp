#if defined(__APPLE__) && defined(__MACH__)
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif
#include <algorithm>
#include <cmath>
#include <vector>
#include <iostream>
#include <cstdlib>

using namespace std;

#define WIDTH 940
#define HEIGHT 840

#define A -1.6
#define B 3.1
#define C -2
#define D 2.2

#define K 100


double DELTA = 0.05;

int n = 1;
const int cols = (B - A) / DELTA;
const int rows = (D - C) / DELTA;
const int number_of_cells = cols * rows;


int Scale = WIDTH * 2 / cols ;
vector<vector<int> > grid(number_of_cells), gr(number_of_cells); 


void keyboard (unsigned char key, int x, int y)
{
    if (key == '\033')
        exit(0);
}

double Ikeda (double xn, double yn)
{
    return 0.4 - 6.0/(1.0 + xn*xn + yn*yn);
}

void henon (double &xn, double &yn)
{
    // const double a = 1.4;
    // const double b = 0.3;
    // double x = xn;
    // double y = yn;


    const double a = -0.9;
    const double b = 1.2;

    double x = xn;
    double y = yn;

    // xn = 1.0 - a* (x * x) + y;
    // yn = b * x;


    xn = 1 + a * (x * cos(Ikeda(x, y)) - y * sin(Ikeda(x,y)));
    yn = b * (x * sin( Ikeda(x, y) ) + y * cos (Ikeda(x, y)));
}

int return_cell (double x, double y)
{
    return floor(abs((double)(y - D)/DELTA)) * cols + floor(abs((double)(x - A)/DELTA));
}

void interval (int cell, double& x1, double& y1)
{
    int row = cell / cols;
    int col = cell % cols;

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
            if ((int)component.size() > 1)
            {
                vector<int> comp = component;
                components.push_back(comp);
            }
            component.clear();
        }
    }

    // for (vector<int>comp : components)
    // {
    //     for (int i : comp)
    //     {
    //         cout << i << " ";
    //     }
    //     cout << endl;
    // }
    cout << components.size() << endl;

}

void make_graph()
{
    for (int i = 0; i < number_of_cells; i++)
    {
        double x1, y1;
        interval(i, x1, y1);
          int sqrt_K = sqrt(K);
        //cout << i << endl;
        //cout << "x1 = "<< x1 << " y1 = "<< y1;
         for (int k = 1; k <= sqrt_K; k++)
            for (int m = 1; m <= sqrt_K; m++)
        {
            double x = x1 + (double)k * DELTA/ (double)sqrt_K;
            double y = y1 - (double)m * DELTA/ (double)sqrt_K;
            // double x = x1 + (double)k * DELTA/ (double)K;
            // double y = y1 - (double)k * DELTA/ (double)K;

            henon(x, y);
            
            if (x <= A || x >= B || y <= C || y >= D)
                continue;

            int cell = return_cell(x , y);
            /*
            cout << cell << " ";
            */
            if (find(grid[i].begin(), grid[i].end(), cell) == grid[i].end())
            {
                grid[i].push_back(cell);
                gr[cell].push_back(i);
            }
        }

        //cout << endl;
    }

}

void draw_square (int cell)
{
    int row = cell / cols;
    int col = cell % cols;
    glRectf(col * Scale, row * Scale, (col + 1) * Scale, (row + 1) * Scale);
}

void draw_grid()
{
    glColor3f(0.0, 0.0, 0.0);
    glBegin(GL_LINES);
    

    for (int i = 0; i < HEIGHT * 2; i+= Scale)
    {
        glVertex2d(0, i);
        glVertex2d(WIDTH * 2, i);
    }

    for (int i = 0; i < WIDTH * 2; i+= Scale)
    {
        glVertex2d(i, 0);
        glVertex2d(i, HEIGHT * 2);
    }

    glEnd();
}

void display()
{
    glClearColor(1.0, 1.0, 1.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    //draw_grid(); 
    //
    for (vector<int> comp : components)
    {
        glColor3f(rand() % 255 /(double) 255, rand() % 255 /(double) 255, rand() % 255 /(double) 255);
        for (int v: comp)
        {
            draw_square(v);
        }
    }
    
    glColor3f(0.0, 1.0, 0.0);
    
    //draw_grid();

    glColor3f(1.0, 0.0, 0.0);
    glBegin(GL_LINES);
        glVertex2d(WIDTH, HEIGHT * 2);
        glVertex2d(WIDTH, 0);
        glVertex2d(WIDTH * 2, HEIGHT);
        glVertex2d(0, HEIGHT);
    glEnd();

    glFlush();
}

int main (int argc, char* argv[])
{
    cout << number_of_cells << endl;
    grid.resize(number_of_cells);
    gr.resize(number_of_cells);
    grid.assign(number_of_cells, vector<int>(0));
    gr.assign(number_of_cells, vector<int>(0));
    make_graph();
    find_components();


    printf("%d\n", Scale);
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE);
    glutInitWindowSize(WIDTH, HEIGHT);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Symbol");
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-WIDTH, WIDTH, HEIGHT, -HEIGHT, -1.0, 1.0);
    glTranslatef(-WIDTH, -HEIGHT, 0);
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutMainLoop();

    return 0;
}

