#pragma once

#ifndef MATR_H_
#define MATR_H_

#include "rational.h"
#include "vector.h"
#include "node.h"

enum Orientation
{
    Vertical = 0,
    Horizontal
};

enum Coord_type
{
    Row = 0,
    Col
};

struct Matrix_coords
{
    unsigned int row;
    unsigned int col;
};

struct Matrix_row_coord
{
    unsigned int row;
};

struct Matrix_col_coord
{
    unsigned int col;
};

class Matrix : public MathObject
{
    Node<Vector> *node;
    unsigned int rows;
    unsigned int cols;
    
    void make_vertical_vector(Vector& vec, Node<Vector>* p, unsigned int col) const;
    void multiply (Matrix& mtr, const Matrix& rv, Node<Vector>* p) const;
    class Iterator_Vec
    {
        friend class Matrix;
        Matrix& master;
        unsigned int coord;
        Coord_type type;      
        
        Vector& provide();
        void remove();
        Iterator_Vec(Matrix& a_master, unsigned int a_coord, Coord_type a_type) 
                    : master(a_master), coord(a_coord), type(a_type) {}
        
        public:
        operator Vector();
        Vector operator= (const Vector& vec);
        Vector operator+= (const Vector& vec);
        Vector operator-= (const Vector& vec);
    };

    void copy(Node<Vector>* p);

    Iterator_Vec operator()(unsigned int coord);
    
    void calculations(Matrix& mtr, Node<Vector>* p, char op) const;

    void write_node(FILE* file, Node<Vector>* p) const;
    void write_node(FILE* file, Node<Rational_number>* p, unsigned int row) const;

    public:
        
        class Iterator_Rat
        {
            friend class Matrix;
            Matrix& master;
            unsigned int row;
            unsigned int col;
            
            Vector& provide();
            void remove();
            Vector find(Node<Vector>* p);
            Iterator_Rat(Matrix& a_master, unsigned int a_row, unsigned int a_col) 
                        : master(a_master), row(a_row), col(a_col) {}
            public:
                
            operator Rational_number();
            Rational_number operator=(const Rational_number& rat);
            Rational_number operator*=(const Rational_number& rat);
            Rational_number operator/=(const Rational_number& rat);
            Rational_number operator+=(const Rational_number& rat);
            Rational_number operator-=(const Rational_number& rat);
            Rational_number operator++();
            Rational_number operator++(int);
            Rational_number operator--();
            Rational_number operator--(int);



        };

        friend class Iterator_Rat;
        friend class Iterator_Vec;
        //friend class Accessor_Rat;
       // friend class Accessor_Vec;
        
        Matrix(char* file_name);
        Matrix(const Matrix& mt);
        Matrix(unsigned int rows, unsigned int cols, States state = Zeros);
        Matrix(const Vector& vec, Orientation orient = Vertical);
        
        void write(const char* file_name) const;
        char* to_string() const;
        
        Matrix operator=(const Matrix& mtr);

        Matrix operator~() const;
        Matrix operator^(int power) const;
        Matrix operator*(const Matrix& mtr) const;

        Matrix operator+ (const Matrix& rv) const;
        Matrix operator- (const Matrix& rv) const;

        Matrix operator+= (const Matrix& rv);
        Matrix operator-= (const Matrix& rv);
        Matrix operator*= (const Matrix& rv);
        
        Iterator_Rat operator()(unsigned int row, unsigned int col);
        Rational_number operator[](Matrix_coords coords) const;
        Vector operator[](Matrix_row_coord row) const;
        Vector operator[](Matrix_col_coord col) const;

        ~Matrix();
};

#endif
