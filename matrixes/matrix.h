#pragma once

#ifndef MATRIX_H_
#define MATRIX_H_

#include <stdint.h>
#include "node.h"
#include "matherr.h"

class Rational_number;
class Vector;
class Matrix;


//
// -------------------------------CLASSES, STRUCTERS AND ENUMS FOR HELP-----------------------
//

class ReadFile
{
    static char* read_str(FILE* file, int& err);
    static void read_vector_size (Vector& vec, FILE* file);
    static void read_matrix_size (Matrix& mtr, FILE* file);
    static bool is_number (const char* num);
    public:
        friend class Rational_number;
        static void read_vector_file(Vector& vec, FILE* file);
        static void read_matrix_file(Matrix& mtr, FILE* file);
};

class MathObject
{
    public:
        virtual char* to_string()const = 0;
        virtual void write(const char* file_name)const = 0;
};

enum States
{
    Zeros = 0,
    Ones,
    Elementary
};

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

//
// -------------------------------------------CLASS RATIONAL_NUMBER---------------------------------
//

class Rational_number
{
    uint32_t numerator;
    uint32_t denominator;
    int sign;

    //
    // Great common devisor for cannonical
    //
    unsigned int gcd (unsigned int a, unsigned int b)
    {
        return (b == 0) ? a : gcd(b, a%b);
    }

    public:
        Rational_number() : numerator(0), denominator(1) , sign(1) {}
        Rational_number(const Rational_number& rat);
        Rational_number(const char* ratio);
        Rational_number(const char* num, const char* denom);
    

        Rational_number(const uint32_t num, const uint32_t denom);
        Rational_number(const int num);
        Rational_number(const short num);
        Rational_number(const long num);

        Rational_number(const unsigned int num) : numerator(num), denominator(1), sign(1) {}
        Rational_number(const unsigned short num) : numerator(num), denominator(1), sign(1) {}
        Rational_number(const unsigned long num);

        Rational_number& operator=(const Rational_number& rv);

        friend const Rational_number operator+(const Rational_number lv, const Rational_number rv); 
        friend const Rational_number operator-(const Rational_number lv, const Rational_number rv); 
        friend const Rational_number operator*(const Rational_number lv, const Rational_number rv); 
        friend const Rational_number operator/(const Rational_number lv, const Rational_number rv); 

        Rational_number& operator+ ();
        Rational_number operator- ();

        friend bool operator>(const Rational_number& lv, const Rational_number& rv); 
        friend bool operator<(const Rational_number& lv, const Rational_number& rv); 
        friend bool operator>=(const Rational_number& lv, const Rational_number& rv); 
        friend bool operator<=(const Rational_number& lv, const Rational_number& rv); 
        friend bool operator==(const Rational_number& lv, const Rational_number& rv); 
        friend bool operator!=(const Rational_number& lv, const Rational_number& rv);


        Rational_number& operator+=(const Rational_number& rv);    

        Rational_number& operator-=(const Rational_number& rv);

        Rational_number& operator*=(const Rational_number& rv);

        Rational_number& operator/=(const Rational_number& rv);


        Rational_number& operator++();
        Rational_number operator++(int);

        Rational_number& operator--();
        Rational_number operator--(int);

        explicit operator short();
        explicit operator int();
        explicit operator long int();
        explicit operator double();

        Rational_number get_number_part();
        Rational_number get_fractional_part();

        Rational_number round();
        Rational_number floor();
        void make_canonical();

        char* to_string() const;

        ~Rational_number();
    
};

//
// ----------------------------------------CLASS VECTOR----------------------------------------------
//

class Vector : public MathObject
{
    Node<Rational_number>* node; 
    unsigned int size;
    int references;

    void calculations (Vector& vec,Node<Rational_number>* q, char op) const;
    void dot_product(Rational_number& rat, Node<Rational_number>* p, Node<Rational_number>* q) const;
    void calculations(Vector& vec,Rational_number rat, Node<Rational_number>* q, char op) const;
    void copy (Node<Rational_number>* p);
    void remove_all(Node<Rational_number>* p);

    void write_node(FILE* file, Node<Rational_number>* p) const;
    public:
        class Iterator
        {
            friend class Vector;
            Vector& master;
            unsigned int index;

            Iterator(Vector& a_master, unsigned int ind) : master(a_master), index(ind) {}

            Rational_number& provide();
            void remove();
            public:
                operator Rational_number();
                Rational_number operator=(const Rational_number &rat);
                
                Rational_number operator+=(const Rational_number &rat);
                Rational_number operator-=(const Rational_number &rat);
                Rational_number operator*=(const Rational_number &rat);
                Rational_number operator/=(const Rational_number &rat);

                Rational_number operator++();
                Rational_number operator++(int);
                Rational_number operator--();
                Rational_number operator--(int);



        };
        
        friend class ReadFile;
        friend class Iterator;
        friend class Matrix;
    
        Vector(unsigned int size, States state = Zeros);
        Vector(const Vector& vec);
        Vector(const char* file_name);

        void write (const char* file_name) const;
        ~Vector();

        Rational_number operator[](unsigned int index) const;

        Iterator operator() (unsigned int index);
                
        operator bool() const;

        Vector operator=(const Vector& rv);

        Vector operator+(const Vector& rv) const;
        Vector operator-(const Vector& rv) const;
        Vector operator*= (const Rational_number& rv);
        Vector operator/=(const Rational_number& rv);
        Vector operator-= (const Vector& rv);
        Vector operator+=(const Vector& rv);

        friend Vector operator* (const Vector& lv,const Rational_number& rv);
        friend Vector operator/ (const Vector& lv,const Rational_number& rv);
        friend Vector operator* (const Rational_number& lv,const Vector& rv);
        
        Rational_number operator*(const Vector& rv) const;

        unsigned int get_size() const { return size;}

        char* to_string() const;
};

//
// ------------------------------------------CLASS MATRIX--------------------------------------
//

class Matrix
{
    Node<Vector> *node;
    unsigned int rows;
    unsigned int cols;
    
    void make_vertical_vector(Vector& vec, Node<Vector>* p, unsigned int col) const;
    void multiply (Matrix& mtr, const Matrix& rv, Node<Vector>* p) const;
    void multiply(Matrix& mtr, const Vector& vec, unsigned int row, Node<Vector>* p) const;
    void calculations(Matrix& mtr,const Rational_number& rat, Node<Vector>* p, char op) const;

    void power (Matrix& mtr, Node<Vector>* p) const;
    class Iterator_Vec
    {
        friend class Matrix;
        Matrix& master;
        unsigned int coord;    
        
        Vector& provide();
        void remove();
        Iterator_Vec(Matrix& a_master, unsigned int a_coord) 
                    : master(a_master), coord(a_coord) {}
        
        public:
        operator Vector();
        Vector operator= (const Vector& vec);
        Vector operator+= (const Vector& vec);
        Vector operator-= (const Vector& vec);
    };


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
        friend class ReadFile;
        //friend class Accessor_Rat;
       // friend class Accessor_Vec;
        
        Matrix(const char* file_name);
        Matrix(const Matrix& mt);
        Matrix(unsigned int rows, unsigned int cols, States state = Zeros);
        Matrix(const Vector& vec, Orientation orient = Vertical);
        
        void write(const char* file_name) const;
        char* to_string() const;
        
        Matrix operator=(const Matrix& mtr);

        Matrix operator~() const;
        Matrix operator^(int power) const;
        Matrix operator*(const Matrix& mtr) const;
        
        friend Vector operator* (const Vector& lv, const Matrix& rv);

        friend Matrix operator* (const Matrix& lv, const Rational_number& rv);
        friend Matrix operator* (const Rational_number& lv, const Matrix& rv);
        friend Matrix operator/ (const Matrix& lv, const Rational_number& rv);

        Matrix operator*= (const Rational_number& rat);
        Matrix operator/= (const Rational_number& rat);

    
        Matrix operator+ (const Matrix& rv) const;
        Matrix operator- (const Matrix& rv) const;

        Matrix operator+= (const Matrix& rv);
        Matrix operator-= (const Matrix& rv);
        Matrix operator*= (const Matrix& rv);
        
        Iterator_Rat operator()(unsigned int row, unsigned int col);
        Rational_number operator[](Matrix_coords coords) const;
        Vector operator[](Matrix_row_coord row) const;
        Vector operator[](Matrix_col_coord col) const;

        unsigned int get_rows() const { return rows;} 
        unsigned int get_columns() const { return cols;} 

        ~Matrix();
};

//
// --------------------------------------------EXCEPTION_CLASSES-----------------------------------
//

class Exception
{
    public:
        virtual void debug_print() = 0;
        virtual ~Exception();
};

class WrongVectorSize : public Matherr<Vector>
{
    public:
        WrongVectorSize(const char* op = 0,
                        const Vector& lv = Vector((unsigned int)0),
                        const Vector& rv = Vector((unsigned int)0)) 
                        : Matherr(op, lv, rv) {}
        WrongVectorSize(const WrongVectorSize& mul) : Matherr(mul) {}

        void debug_print();
};

class WrongMatrixSize : public Matherr<Matrix>
{
    public:
        WrongMatrixSize(const char* op = 0, const Matrix& lv = Matrix(0, 0), const Matrix& rv = Matrix(0, 0)) 
                        : Matherr(op, lv, rv) {}
        WrongMatrixSize(const WrongMatrixSize& mat) : Matherr(mat) {}

        void debug_print();
};

class WrongLexeme :public Exception
{
    protected:
        char* lexeme;
    public:
        WrongLexeme(const char* lexeme = 0);
        void debug_print();
        ~WrongLexeme();
};

class NotANumber : public WrongLexeme
{
    public:
        NotANumber(const char* lexeme = 0) : WrongLexeme(lexeme) {}
        void debug_print();
};


class Overflow : public Matherr<Rational_number>
{
   
    public:
        Overflow(const char* op = 0, const Rational_number &lv = 0,const Rational_number &rv = 0) 
                : Matherr(op, lv, rv) {}
        Overflow(const Overflow & ov) : Matherr(ov) {}

        void debug_print();
};

class Zerodivide : public Matherr<Rational_number>
{
    public:
        Zerodivide(const char* op = 0, const Rational_number &lv = 0,const Rational_number &rv = 0) 
                : Matherr(op, lv, rv) {}
        Zerodivide(const Zerodivide & zr) : Matherr(zr) {}
        void debug_print();
};

class WrongArgument : public Exception
{
    public:
        
        void debug_print();
};

class OpenFileError : public Exception
{
    char* file_name;
    public:
        OpenFileError(const char* file);

        void debug_print();

        ~OpenFileError();
};

class OutOfRangeVector : public Exception
{
    const Vector& vec;
    unsigned int index;
    public:
        OutOfRangeVector(const Vector& v, unsigned int i) : vec(v), index (i) {}
        OutOfRangeVector(const OutOfRangeVector& v) : vec(v.vec), index(v.index) {}

        void debug_print();
};

class OutOfRangeMatrix : public Exception
{
    const Matrix& mat;
    unsigned int index;
    public:
        OutOfRangeMatrix(const Matrix& m, unsigned int i) : mat(m), index(i) {}
        OutOfRangeMatrix(const OutOfRangeMatrix& m): mat(m.mat), index(m.index) {}

        void debug_print();
};

#endif
