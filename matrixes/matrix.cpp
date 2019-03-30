#include "matr.h"
#include "rational.h"
#include "node.h"
#include "vector.h"
#include "exception.h"
#include <cstdio>
#include <cstring>


//
// -----------------------------CONSTRUCTORS--------------------------
//

Matrix::Matrix(unsigned int rows, unsigned int cols, States state)
{
    node = 0;
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

Matrix::Matrix(const Matrix& mtr)
{
    node = 0;
    rows = mtr.rows;
    cols = mtr.cols;

    Node<Vector>::copy(node, mtr.node);
}

Matrix::Matrix(const Vector& vec, Orientation orient)
{
    node = 0;
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

//
// ---------------------------------ITERATOR_RATIONAL---------------------------
//

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
    Node<Vector>* head = Node<Vector>::find(row, master.node);

    if (head) return head->value;

    master.node = head = Node<Vector>::insert(row, master.node, Vector(master.cols));

    return Node<Vector>::find(row, master.node)->value;
}

void Matrix::Iterator_Rat::remove()
{
    master.node = Node<Vector>::remove(row, master.node);
}

//
// -----------------------------------ITERATOR_VECTOR-------------------------------------
//

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

Vector Matrix::Iterator_Vec::operator+= (const Vector& rv)
{
    //TODO Exception
    Vector &vec = provide();
    
    vec+=rv;

    Vector res = vec;

    if (!vec) remove();
    return res;
}

Vector Matrix::Iterator_Vec::operator-= (const Vector& rv)
{
    //TODO Exception
    Vector &vec = provide();
    
    vec-=rv;

    Vector res = vec;

    if (!vec) remove();
    return res;
}

Vector& Matrix::Iterator_Vec::provide()
{
    Node<Vector>* head = Node<Vector>::find(coord, master.node);

    if (head) return head->value;

    master.node = head = Node<Vector>::insert(coord, master.node, Vector(master.cols));

    return Node<Vector>::find(coord, master.node)->value;
}

void Matrix::Iterator_Vec::remove()
{
    master.node = Node<Vector>::remove(coord, master.node);
}

//
// -------------------------------------OPERATORS--------------------------------------
//

Matrix Matrix::operator=(const Matrix& rv)
{
    rows = rv.rows;
    cols = rv.cols;

    for (unsigned int i = 0; i < rows; i++)
    {
        if (Node<Vector>::find(i, node))
            node = Node<Vector>::remove(i, node);
    }
    node = 0;
    Node<Vector>::copy(node, rv.node);

    return rv;
}

Rational_number Matrix::operator[] (Matrix_coords coords) const
{
    //TODO Exception
    Node<Vector>* f = Node<Vector>::find(coords.row, node);
    if (!f) return 0;
    Vector vec = f->value;

    return vec[coords.col];

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
    
    Node<Vector>* f = Node<Vector>::find(coord.row, node);
    if (!f) return Vector(rows);
    Vector vec = f->value;

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

Matrix Matrix::operator*(const Matrix& rv) const
{
    // TODO Excepetion 
    Matrix mtr(*this);

    multiply(mtr, ~rv, node);

    return mtr;
}
/*
void Matrix::power(Vector& vec,const Vector& vec, Node<Vector>* p) const
{
    if (!p) return;
    power(mtr, p->return_left());
    power(mtr, p->return_right());


}
*/
Matrix Matrix::operator^(int pow) const
{
    // TODO Exception
    if (!pow) return Matrix(rows, cols, Elementary);
    if (pow == 1) return *this;


    Matrix mtr(*this);
        for (unsigned int i = 0;i < rows; i++)
        {
            Vector vec(cols);
            Matrix_row_coord row;
            row.row = i;
            for (int p = 1; p < pow; p++)
            {
                for (unsigned int j = 0; j < cols; j++)
                {
                    Matrix_col_coord col;
                    col.col = j;
                    vec(j) = mtr[row] * (*this)[col];
                }
                mtr(i) = vec;
            }
        }
    return mtr;
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
Matrix::Iterator_Rat Matrix::operator()(unsigned int row, unsigned int col)
{
    //TODO Exception
    
    return Iterator_Rat(*this, row, col);
}


Matrix::Iterator_Vec Matrix::operator()(unsigned int row)
{
   return Iterator_Vec(*this, row);
}

Matrix operator* (const Matrix& lv,const Rational_number& rv)
{
    Matrix mtr(lv);
    mtr.calculations(mtr, rv, lv.node, '*');

    return mtr;
}
Matrix operator* (const Rational_number& lv,const Matrix& rv)
{
    Matrix mtr(rv);
    mtr.calculations(mtr, lv, rv.node, '*');

    return mtr;
}

Matrix operator/ (const Matrix& lv,const Rational_number& rv)
{
    Matrix mtr(lv);
    mtr.calculations(mtr, rv, lv.node, '/');

    return mtr;
}

Matrix Matrix::operator*= (const Rational_number& rv)
{
    *this = *this * rv;
    return *this;
}

Matrix Matrix::operator/= (const Rational_number& rv)
{
    *this = *this / rv;
    return *this;
}

//
//---------------------------------RECURSIVE_CALCULATIONS--------------------------------
//

void Matrix::make_vertical_vector(Vector& vec, Node<Vector>* p, unsigned int col) const
{
    if (!p) return;
    make_vertical_vector(vec, p->return_left(), col);
    make_vertical_vector(vec, p->return_right(), col);

    vec(p->return_key()) = p->value[col];
}


void Matrix::multiply(Matrix& mtr,const Matrix& rv, Node<Vector>* p) const
{
    if (!p) return;
    multiply(mtr, rv, p->return_left());
    multiply(mtr, rv, p->return_right());
    
    multiply(mtr, p->value, p->return_key(), rv.node);
}

void Matrix::multiply(Matrix& mtr, const Vector& vec, unsigned int row, Node<Vector>* p) const
{
    if (!p) return;
    multiply(mtr, vec, row, p->return_left());
    multiply(mtr, vec, row, p->return_right());
    mtr(row, p->return_key()) = vec * p->value;
}


void Matrix::calculations(Matrix& mtr,const Rational_number& rat, Node<Vector>* p, char op) const
{
    if (!p) return;
    switch(op)
    {
        case '*':
            mtr(p->return_key()) = (Vector)mtr(p->return_key()) * rat;
            break;
        case '/':
            mtr(p->return_key()) = (Vector)mtr(p->return_key()) / rat;
    }
    calculations(mtr, rat, p->return_left(), op);
    calculations(mtr, rat, p->return_right(), op);
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

//
// --------------------------------------------OTHER---------------------------------------
//

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

//
// --------------------------------------DESTRUCTOR---------------------------------
//

Matrix::~Matrix()
{
    for (unsigned int i = 0; i < rows; i++)
    {
        if (Node<Vector>::find(i, node))
            node = Node<Vector>::remove(i, node);
    }
}

