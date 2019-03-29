#include "matr.h"
#include "rational.h"
#include "node.h"
#include "vector.h"
#include "exception.h"
#include <cstdio>
#include <cstring>

Matrix::Matrix(unsigned int rows, unsigned int cols, States state)
{
    this->rows = rows;
    this->cols = cols;
    switch(state)
    {
        case Zeros:
            break;
        case Ones:
            for (unsigned int i = 0; i < rows; i++)
            {
                (*this)(i) = Vector(cols, Ones);
            }
            break;
        case Elementary:
            //TODO Exception:
            for (unsigned int i = 0; i < rows; i++)
            {
                Vector vec(cols);
                vec(i) = 1;
                (*this)(i) = vec;
            }
            break;
        default:
            //TODO Exception
            break;
    }
}

void Matrix::copy(Node<Vector>* p)
{
    if (!p) return;
    node = Node<Vector>::insert(p->return_key(), node, p->value);

    copy(p->return_left());
    copy(p->return_right());
}

Matrix::Matrix(const Matrix& mtr)
{
    rows = mtr.rows;
    cols = mtr.cols;

    copy(mtr.node);
}

Matrix::Matrix(const Vector& vec, Orientation orient)
{
    switch(orient)
    {
        case Vertical:
            rows = vec.size;
            cols = 1;

            for (unsigned int i = 0; i < rows; i++)
            {
                Vector one(1);
                one(0) = vec[i];

                (*this)(i) = vec;
            }
            break;
        case Horizontal:
            rows = 1;
            cols = vec.size;

            (*this)(0) = vec;
            break;
        default:
            //TODO Exception
            break;
    }
}

Matrix::~Matrix()
{
    for (unsigned int i = 0; i < rows; i++)
    {
        if (Node<Vector>::find(i, node))
            node = Node<Vector>::remove(i, node);
    }
}

Matrix::Iterator_Rat::operator Rational_number()
{
    Vector vec = provide();

    return vec[col];
}

Matrix::Iterator_Vec::operator Vector()
{
    Vector vec = provide();
    return vec;
}

Rational_number Matrix::Iterator_Rat::operator= (const Rational_number& rat)
{
    Vector& vec = provide();

    Rational_number res = vec(col) = rat;

    if (!vec) remove();

    return res;
}

Rational_number Matrix::Iterator_Rat::operator*=(const Rational_number& rat)
{
   Vector& vec = provide();
    
    vec(col) *= rat;
    Rational_number res = vec[col];
    if (!vec) remove();

    return res;
}

Rational_number Matrix::Iterator_Rat::operator/=(const Rational_number& rat)
{
    Vector& vec = provide();

    vec(col) /= rat;
    Rational_number res = vec[col];
    if (!vec) remove();

    return res;
}

Rational_number Matrix::Iterator_Rat::operator+=(const Rational_number& rat)
{
    Vector& vec = provide();

    vec(col) += rat;
    Rational_number res = vec[col];

    if (!vec) remove();

    return res;
}

Rational_number Matrix::Iterator_Rat::operator-=(const Rational_number& rat)
{
    Vector& vec = provide();

    vec(col) -= rat;
    Rational_number res = vec[col];
    if (!vec) remove();

    return res;
}

Rational_number Matrix::Iterator_Rat::operator++()
{
    Vector& vec = provide();

    Rational_number res = ++vec(col);

    if (!vec) remove();

    return res;
}

Rational_number Matrix::Iterator_Rat::operator++(int)
{
    Vector& vec = provide();

    Rational_number res = vec(col)++;

    if (!vec) remove();

    return res;

}

Rational_number Matrix::Iterator_Rat::operator--()
{
    Vector& vec = provide();

    Rational_number res = --vec(col);

    if (!vec) remove();

    return res;

}

Rational_number Matrix::Iterator_Rat::operator--(int)
{
    Vector& vec = provide();

    Rational_number res = vec(col)--;

    if (!vec) remove();

    return res;
}



Vector& Matrix::Iterator_Rat::provide()
{
    Node<Vector>* head = master.node;
    if (Node<Vector>::find(row, master.node)) 
        master.node = head = Node<Vector>::insert(row, master.node, Vector(master.cols));
    while(true)
    {
        if (row == head->return_key()) return head->value;
        else if (row < head->return_key()) head = head->return_left();
        else head = head = head->return_right();
    }
}

void Matrix::Iterator_Rat::remove()
{
    master.node = Node<Vector>::remove(row, master.node);
}

Vector Matrix::Iterator_Vec::operator= (const Vector& rv)
{
    //TODO Exception
    if (!rv) remove();
    else
    {
        Vector& vec = provide();
        vec = rv;
    }
    return rv;
}

Vector& Matrix::Iterator_Vec::provide()
{
    Node<Vector>* head = master.node;
    if (Node<Vector>::find(coord, master.node))
        master.node = head = Node<Vector>::insert(coord, master.node, Vector(master.cols));
    while(true)
    {
        if (coord == head->return_key()) return head->value;
        else if (coord < head->return_key()) head = head->return_left();
        else head = head->return_left();
    }
}

void Matrix::Iterator_Vec::remove()
{
    master.node = Node<Vector>::remove(coord, master.node);
}

Rational_number Matrix::operator[] (Matrix_coords coords) const
{
    //TODO Exception
    Vector vec = Node<Vector>::find(coords.row, node);
    if (!vec) return 0;

    return vec[coords.col];

}

void Matrix::make_vertical_vector(Vector& vec, Node<Vector>* p, unsigned int col) const
{
    if (!p) return;
    make_vertical_vector(vec, p->return_left(), col);
    make_vertical_vector(vec, p->return_right(), col);

    vec(p->return_key()) = p->value[col];
}

Vector Matrix::operator[](Matrix_col_coord coord) const
{
    //TODO Exception
    Vector vec(rows);
    make_vertical_vector(vec, node, coord.col);
    
    return vec;
}

Vector Matrix::operator[](Matrix_row_coord coord) const
{
    //TODO Exception
    Vector vec = Node<Vector>::find(coord.row, node);
    if (vec) return Vector(rows);
    
    return vec;
}


Matrix Matrix::operator~() const
{
    Matrix transp(cols, rows);
    for (unsigned int i = 0; i < cols; i++)
    {
        Vector vec(rows);
        make_vertical_vector(vec, node, i);
        if (!vec) continue;

        transp.node = Node<Vector>::insert(i, transp.node, vec);
    }
    return transp;
}

void Matrix::multiply(Matrix& mtr,const Matrix& rv, Node<Vector>* p) const
{
    if (!p) return;
    multiply(mtr, rv, p->return_left());
    multiply(mtr, rv, p->return_right());
    for (unsigned int i = 0; i < rv.cols; i++)
    {
        Matrix_col_coord col;
        col.col = i;
        mtr(p->return_key(), i) = p->value* rv[col];
    }
}

Matrix Matrix::operator*(const Matrix& rv) const
{
    // TODO Excepetion 
    Matrix mtr(*this);

    multiply(mtr, rv, node);

    return mtr;
}

Matrix Matrix::operator^(int power) const
{
    // TODO Exception
    Matrix mtr(*this);

    for (int i = 0; i < power; i++)
        multiply(mtr, *this, mtr.node);

    return mtr;
}

void Matrix::write_node(FILE* file, Node<Rational_number>* p, unsigned int row) const
{
    if (!p) return;
    write_node(file, p->return_left(), row);
    
    char* str = p->value.to_string();
    fprintf(file, "%u %u %s\n", row, p->return_key(), str);
    delete[] str;
    write_node(file, p->return_right(), row);
}

void Matrix::write_node(FILE* file, Node<Vector>* p) const
{
    if (!p) return;
    write_node(file, p->return_left());
    
    write_node(file, p->value.node, p->return_key());

    write_node(file, p->return_right());
}

void Matrix::write(const char* file_name) const
{
    FILE* file = fopen(file_name, "w");
    //TODO Exception
    
    fprintf(file, "matrix %u %u\n", rows, cols);
    
    write_node(file, node);

    fclose(file);
}


char* Matrix::to_string() const
{
    char* completed = NULL;
    Matrix_row_coord coord;
    coord.row = 0;
    char* str = (*this)[coord].to_string();

    completed = new char[strlen(str) + 2];
    strcpy(completed, str);
    strcat(completed, "\n\0");

    delete[] str;

    for (unsigned int i = 1; i < rows; i++)
    {
        coord.row = i;
        char* tmp = completed;
        str = (*this)[coord].to_string();

        completed = new char[strlen(tmp) + strlen(str) + 2];
        strcpy(completed, tmp);
        strcat(completed, str);
        strcat(completed, "\n\0");

        delete[] tmp;
        delete[] str;
    }

    return completed;
}

void Matrix::calculations(Matrix& mtr, Node<Vector>* p, char op) const
{
    if (!p) return;

    calculations(mtr, p->return_left(), op);
    calculations(mtr, p->return_right(), op);

    switch(op)
    {
        case '+':
            mtr(p->return_key()) += p->value;
            break;
        case '-':
            mtr(p->return_key()) -= p->value;
            break;
    }

}

Matrix Matrix::operator+(const Matrix& rv) const
{
    //TODO Exception
    Matrix mtr(*this);
    
    calculations(mtr, rv.node, '+');

    return mtr;
}

Matrix Matrix::operator-(const Matrix& rv) const
{
    //TODO Exception
    Matrix mtr(*this);
    
    calculations(mtr, rv.node, '-');

    return mtr;
}

Matrix Matrix::operator+=(const Matrix& rv)
{
    //TODO Exception
    *this = *this + rv;
    return *this;
}

Matrix Matrix::operator-=(const Matrix& rv)
{
    //TODO Exception
    *this = *this - rv;
    return *this;
}

Matrix Matrix::operator*=(const Matrix& rv)
{
    //TODO Exception
    *this = *this * rv;
    return *this;
}





