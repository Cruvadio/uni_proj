#if defined(__APPLE__) && defined(__MACH__)
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#include<cmath>
#include<vector>
#include<iostream>
#include <sys/time.h>

using std::vector;

#define WIDTH 1280
#define HEIGHT 720

#define A 0.5

#define N 30

const double h = (2.0 * A) / WIDTH;


void keyboard (unsigned char key, int x, int y)
{
    if (key == '\033')
        exit(0);
}


struct Point
{
    double x;
    double y;

    Point(double x = 0, double y = 0) : x(x), y(y) {}

    Point operator/ (double k)
    {
        return Point (x / k, y / k);
    }

    Point& operator/= (double k)
    {
        x/=k;
        y/=k;
        return *this;
    }

    Point operator+ (const Point& rv)
    {
        return Point(x + rv.x, y + rv.y);
    }

    Point operator- (const Point& rv)
    {
        return Point(x - rv.x, y - rv.y);
    }

    bool operator< (Point rv)
    {
        return x < rv.x && y < rv.y;
    }

    bool operator> (Point rv)
    {
        return x > rv.x && y > rv.y;
    }


    Point absolute () const
    {
        return Point(abs(x), abs(y));
    }
};

struct Interval
{
    Point begin;
    Point end;

    Interval(double x1 = 0, double y1 = 0, double x2 = 0, double y2 = 0): begin(x1, y1), end(x2, y2)
    {}
    
    Interval (const Point &p1, const Point &p2): begin(p1), end(p2) {}
    operator double()
    {
        return sqrt(pow ((end.x - begin.x), 2) + pow ((end.y - begin.y), 2));
    }
    
    vector<Interval> return_halfs ()
    {
        vector<Interval> parts;
        
        Point half((begin + end)/2);
        parts.push_back(Interval(begin, half));
        parts.push_back(Interval(half, end));

        return parts;
    }

    Interval f_interval(void (*f) (Point& ))
    {
        Interval tmp = *this;
        f(tmp.begin);
        f(tmp.end);

        return tmp;
    }
    
    static vector<Interval> make_partition (const vector<Interval>& vec)
    {
        vector<Interval> partition;
        for (Interval i : vec)
        {
            vector<Interval> half = i.return_halfs();

            partition.push_back(half[0]);
            partition.push_back(half[1]);
        }

        return partition;
    }

    void show()
    {
        glVertex2f(begin.x, begin.y);
        glVertex2f(end.x, end.y);
    }

};

void f (Point & p)
{
    double x = p.x;
    double y = p.y;

    p.x = 1.1 * x - 0.1 * y *sin(x);
    p.y = 0.7 * y - 0.5 * x * x;
}

void g (Point& p)
{
    const double a = 0.4;

    double x = p.x;
    double y = p.y;

    p.x = x + y + a * x * (1 - x *x);
    p.y = y + a * x * (1 - x*x);
}

void k (Point &p)
{
    const double a = 0.4;

    double x = p.x;
    double y = p.y;

    p.x = x - y;
    p.y = y - a * p.x * (1 - p.x * p.x);
}

/*
Interval f_interval (Interval i, void (*f) (Point&) )
{
    f(i.begin);
    f(i.end);

    return Interval(i);
}*/


void partitions (vector<Interval> intervals, vector<Interval> & vec, void (*f) (Point& ))
{
    for (Interval i: intervals)
    {
        Interval new_i = i.f_interval(f);
       
        if (new_i > h)
        {
            partitions(i.return_halfs(), vec, f);
            
        }
        else
        {
            if (abs(new_i.begin.x) > 2 ||
                abs(new_i.begin.y) >  0.5||
                abs(new_i.end.x) > 2 ||
                abs(new_i.end.y) > 0.5)
                
                    continue;
                    
            vec.push_back(new_i);
        }
    }
}


vector<Interval> left;
vector<Interval> right;

void iterations (Interval start, vector<Interval>& finish, void (*f) (Point& ))
{
    vector<Interval> intervals;
    intervals.push_back(start);
    vector<Interval> vec;

    for (int i = 0; i < N; i++)
    {
        partitions(intervals, vec, f);     

        /*glBegin(GL_LINE_STRIP);
            glColor3f(0.0, 0.0 + (double)i/N, 0.0);
            for (Interval inter : finish)
                inter.show();
        glEnd();
        */
        intervals = vec;
        if (i + 1 >= N)
            break;
        vec.clear();
    }

    finish = vec;
}

void display(void)
{
   
    glClearColor(1.0, 1.0, 1.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    glColor3f(0.0, 0.0, 0.0);
    glBegin(GL_LINES);
        glVertex2f(-WIDTH, 0);
        glVertex2f(WIDTH, 0);
        glVertex2f(0, HEIGHT);
        glVertex2f(0, -HEIGHT);
    glEnd();

    glBegin(GL_LINE_STRIP);
        int i = 0;
        for (Interval inter : left)
        {
            glColor4f(0.0, 0.0, 0.0, 1.0);
            i++;
            inter.show();
        }
    glEnd();

    glBegin(GL_LINE_STRIP);
        i = 0;
        for (Interval inter : right)
        {
            glColor4f(1.0, 0.0, 0.0, 1.0);
            i++;
            inter.show();
        }
    glEnd();

    glFlush();
}

vector<Point> homoclinics;

int main (int argc, char* argv[])
{
    Interval line (-0.2, 0.0, 0.2, 0.0);

    struct timeval t1, t2;
    gettimeofday(&t1, NULL);

    iterations(line, left, g);


    //line.end.x = 1.0;
    //iterations(line, right);

    iterations(line, right, k);

    gettimeofday(&t2, NULL);

    double t = (t2.tv_sec - t1.tv_sec) + (double)(t2.tv_usec - t1.tv_usec) / 1000000;
    Point delta = Point(0.001, 0.001);
    
    
    //std::cout << left.size() << std::endl;
    std::cout << N << " " << t << std::endl;

    // for (int i = 0; i < left.size(); i++)
    // {
    //     for (int j = 0; j < right.size(); j++)
    //     {
    //         if ((left[i].begin - right[j].begin).absolute() < delta)
    //             homoclinics.push_back(left[i].begin);
    //         if ((left[i].end - right[i].end).absolute() < delta)
    //             homoclinics.push_back(left[i].end);

            
    //     }
    // }

    // for(auto p : homoclinics)
    // {
    //     std::cout << p.x << " " << p.y << std::endl;
    // }

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE);
    glutInitWindowSize(WIDTH, HEIGHT);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Iterations");
    glEnable(GL_BLEND);
    
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-1.5, 1.5, -0.5, 0.5, -1, 1);
    //glScalef(2.0, 2.0, 1.0);
    glutKeyboardFunc(keyboard);
    glutDisplayFunc(display);
    glutMainLoop();

    return 0;
}
