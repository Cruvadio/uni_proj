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

    public:
        
        class Iterator_Rat
        {
            friend class Matrix;
            Matrix& master;
            unsigned int row;
            unsigned int col;
            
            Rational_number& provide();
            Iterator_Rat(Matrix& master, Matrix_coords coords);
            public:
                
            operator Rational_number();
            Rational_number operator=(const Rational_number& rat);
            Rational_number operator*=(const Rational_number& rat);
            Rational_number operator/=(const Rational_number& rat);

        };
/*
        class Iterator_Vec
        {
            friend class Matrix;
            Matrix& master;
            unsigned int coord;
            Coord_type type;      
            
            Vector& provide();
            Iterator_Vec(Matrix& master, unsigned int coord, Coord_type type);

        };
*/
        class Accessor_Rat
        {
            friend class Matrix;
            const Matrix& master;
            unsigned int row;
            unsigned int col;

            Rational_number provide();
            Accessor_Rat(const Matrix& mtr, Matrix_coords coords);
            public:
                operator Rational_number() const;
        };
        class Accessor_Vec
        {
            friend class Matrix;
            const Matrix& master;
            unsigned int coord;
            Coord_type type;      
            
            Vector provide();
            Accessor_Vec(const Matrix& master, unsigned int coord, Coord_type type);

            operator Vector() const;


        };
        
        friend class Iterator_Rat;
       // friend class Iterator_Vec;
        friend class Accessor_Rat;
        friend class Accessor_Vec;
        
        Matrix(char* file_name);
        Matrix(const Matrix& mt);
        Matrix(unsigned int rows, unsigned int cols, States state = Zeros);
        Matrix(const Vector& vec, Orientation orient = Vertical);

        void write(const char* file_name);
        char* to_string();

        Matrix operator~();
        Matrix operator^(int power);
        Matrix operator*(const Matrix& mtr);
        
        Iterator_Rat operator()(unsigned int row, unsigned int col);
        Accessor_Rat operator[](Matrix_coords coords) const;
        Accessor_Vec operator[](Matrix_row_coord row) const;
        Accessor_Vec operator[](Matrix_col_coord col) const;
};

#endif
