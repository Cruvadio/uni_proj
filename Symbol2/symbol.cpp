#if defined(__APPLE__) && defined(__MACH__)
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif
#include <algorithm>
#include <cmath>
#include <sys/time.h>
#include <vector>
#include <iostream>
#include <cstdlib>
#include <ctime>

using namespace std;

#define WIDTH 960
#define HEIGHT 800

#define A -1.2
#define B 1.2
#define C -1
#define D 1

#define K 100

#define CELLS 2

#define DELTA 0.05



int n = 1;

 

void keyboard (unsigned char key, int x, int y)
{
    if (key == '\033')
        exit(0);
}

void skeyboard (int key, int x, int y)
{
    if (key == GLUT_KEY_UP)
    {
        n++;
        if (n > 7) n = 7;
        glutPostRedisplay();
    }
    else if (key == GLUT_KEY_DOWN)
    {
        n--;
        if (n < 1) n = 1;
        glutPostRedisplay();
    }
}



void henon (double &xn, double &yn)
{
    // const double a = -0.5;
    // const double b = 0;
    // double x = xn;
    // double y = yn;

    // xn = x*x - y*y + a;
    // yn = 2 * x * y + b;


    const double n = 1.0;

    double x = xn;
    double y = yn;

    xn = x * pow(2.0 - x * x, 1.0/(n + 2.0));
    yn = 0.5 * y;  
}

int return_cell (double x, double y, int cols,  double delta)
{
    return (int)abs((double)(y - D)/delta) * cols + (int)abs((double)(x - A)/delta);
}

void interval (int cell, double& x1, double& y1, int cols, double delta)
{
    int row = cell / cols;
    int col = cell % cols;

    x1 = A + col * delta;

    y1 = D - row * delta;
}

vector<bool> used;
vector<int> order;
 
void dfs1 (int v, vector<vector<int>> &grid) {
	used[v] = true;
	for (size_t i=0; i<grid[v].size(); ++i)
		if (!used[ grid[v][i] ])
			dfs1 (grid[v][i], grid);
	order.push_back (v);
}
 
void dfs2 (int v, vector<vector<int> > &gr, vector<int> &component) {
	used[v] = true;
	component.push_back (v);
	for (size_t i=0; i<gr[v].size(); ++i)
		{if (!used[ gr[v][i] ])
			dfs2 (gr[v][i], gr, component);
            if (gr[v][i] == v)
            component.push_back(v);
            }
}


vector<vector<int> > find_components (vector <vector<int> > &grid, vector<vector<int> > &gr, int number_of_cells)
{
    vector<vector<int> > components;
    vector<int> component;
    used.resize(number_of_cells);
    used.assign(number_of_cells, false);
    for (int i = 0; i < number_of_cells; i++)
        if (!used[i]) dfs1(i, grid);
    used.assign(number_of_cells, false);
    
    
    //for (int v: order)
    //{
      //  cout << v << endl;
    //}

    for (int i = 0; i < number_of_cells; i++)
    {
        int v = order[number_of_cells - 1 - i];
        if (!used[v])
        {
            dfs2(v, gr, component);
            if ((int)component.size() > 1)
            {
                components.push_back(component);
            }
            component.clear();
        }
    }

    // int i = 1;
    // for (vector<int>comp : components)
    // {
    //     cout << i++ << ": " << endl;
    //     for (int k : comp)
    //     {
    //         cout << k << " ";
    //     }
    //     cout << endl;
    // }

    cout << components.size() << endl;
    order.clear();
    return components;
}

void make_graph(vector<vector<int> > &graph, vector<vector<int> >& i_graph, int number_of_cells, double delta)
{
    int cols = (B - A) / delta;
    int sqrt_K = sqrt(K);

    double d =  delta/ ((double)sqrt_K);
    for (int i = 0; i < number_of_cells; i++)
    {
        double x1, y1;
        interval(i, x1, y1, cols, delta);
  //      cout << i << endl;
 //       cout << "x1 = "<< x1 << " y1 = "<< y1 << endl;
        for (int k = 1; k <= sqrt_K; k++)
            for (int m = 1; m <= sqrt_K; m++)
        {
            double x = x1 + (double)k * d;
            double y = y1 - (double)m * d;

            henon(x, y);
            

            
            if (x <= A || x >= B || y <= C || y >= D){
               // cout << "x = "<< x << " y = "<< y << endl;
               // cout << "-1" << endl;
                continue;
            }

            int cell = return_cell(x , y, cols, delta);
            
                                      
            
            
            
            if (find(graph[i].begin(), graph[i].end(), cell) == graph[i].end())
            {

                //  cout << "x = "<< x << " y = "<< y << endl;
                // cout << cell << " " << endl;

                graph[i].push_back(cell);
                i_graph[cell].push_back(i);
            }
        }

        //cout << endl;
    }

}

void output_graph(vector<vector<int>> &graph)
{
    for (int i = 0; i < graph.size(); i++)
    {
        cout << i << ":" << endl;
        
        for (int j = 0; j < graph[i].size(); j++)
        {
            cout << graph[i][j] << ", ";
        }
        
        cout << endl;
    }
}

vector<vector<int> > approximation (int &scale, int &cols)
{
    double delta = DELTA;

    for (int i = 1; i < n; i++)
        delta /= CELLS;
    
    int rows = (D - C) / delta;
    cols = (B - A) / delta;
    int number_of_cells = rows * cols;
    
    scale = WIDTH* 2 / cols;
    vector <vector <int> > graph (number_of_cells);
    vector <vector <int> > i_graph (number_of_cells);

    make_graph(graph, i_graph, number_of_cells, delta);
    
    output_graph(graph);

    return find_components(graph, i_graph, number_of_cells);
}

void draw_square (int cell, int scale,int cols)
{
 
    int row = cell / cols;
    int col = cell % cols;
    glRectf(col * scale, row * scale, (col + 1) * scale, (row + 1) * scale);
}

void draw_grid(int scale)
{
    glColor3f(0.0, 0.0, 0.0);
    glBegin(GL_LINES);
    

    for (int i = 0; i < HEIGHT * 2; i+= scale)
    {
        glVertex2d(0, i);
        glVertex2d(WIDTH * 2, i);
    }

    for (int i = 0; i < WIDTH * 2; i+= scale)
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
    
    struct timeval t1, t2;
    
    gettimeofday(&t1, NULL);
    int scale, cols;
    vector<vector<int> > components = approximation(scale, cols);
    for (vector<int> component : components)
    {
           glColor3f(rand() % 255 /(double) 255, rand() % 255 /(double) 255, rand() % 255 /(double) 255);
        for (int v: component)
        {
            draw_square(v, scale, cols);
        }
    }

    glColor3f(0.0, 1.0, 0.0);

    gettimeofday(&t2, NULL);
    double t = (t2.tv_sec - t1.tv_sec) + (double)(t2.tv_usec - t1.tv_usec) / 1000000;
    
    printf("%lf\n", t);

    //if (n < 6)
    //draw_grid(scale);

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
    //     int scale, cols;
    // vector<vector<int> > components = approximation(scale, cols);
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
    glutSpecialFunc(skeyboard);
    glutMainLoop();

    return 0;
}

