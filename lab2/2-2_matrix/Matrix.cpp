#include "Matrix.hpp"
#include <cstdio>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <cmath>
#include <stdexcept>

// Identity matrix
Matrix Matrix::eye(const unsigned int n)
{
	Matrix m(n);
	#pragma omp parallel for
	for(unsigned int i=0; i<n; i++){
		for (unsigned int j=0; j<n; j++){
			if (i == j) m[i][i] = 1.0;
			else m[i][j] = 0.0;
		}
	}
	return m;
}

Matrix Matrix::random(const unsigned int i)
{
	return Matrix::random(i, i);
}

Matrix Matrix::random(const unsigned int rows, const unsigned int cols)
{

	Matrix m(rows, cols);
	for(unsigned int i=0; i<rows*cols; i++){
		m.array[i] = (double)rand()/(double) RAND_MAX;
	}

	return m;
}

// =============================================================== //
// Constructors

Matrix::Matrix(int m): Matrix(m,m) {}

Matrix::Matrix(int m, int n)
: rows(m), cols(n)
{
	this->array = new double[this->rows*this->cols];
}

Matrix::Matrix(const Matrix& matrix)
: Matrix(matrix.rows, matrix.cols)
{
	memcpy(this->array, matrix.array, sizeof(double)*this->rows*this->cols);
}
// =============================================================== //
// Destructor
Matrix::~Matrix()
{
	delete[] this->array;
}

// =============================================================== //
// Arithmetic operators
Matrix& Matrix::operator=(const Matrix& matrix)
{

	if(this->rows != matrix.rows || this->cols != matrix.cols){
		this->rows = matrix.rows;
		this->cols = matrix.cols;

		delete[] this->array;
		this->array = new double[this->rows*this->cols];		
	}

	memcpy(this->array, matrix.array, sizeof(double)*this->rows*this->cols);
	return *this;
}

Matrix& Matrix::operator+=(const Matrix& matrix)
{
	if(this->rows != matrix.rows || this->cols != matrix.cols){
		throw std::invalid_argument("Size of matrices does not align");
	}

	for(unsigned int i=0; i<this->rows*this->cols; i++){
		this->array[i] += matrix.array[i];
	}
	return *this;
}

Matrix& Matrix::operator+=(const double value)
{

	for(unsigned int i=0; i<this->rows*this->cols; i++){
		this->array[i] += value;
	}
	return *this;
}

Matrix& Matrix::operator-=(const Matrix& matrix)
{
	if(this->rows != matrix.rows || this->cols != matrix.cols){
		throw std::invalid_argument("Size of matrices does not align");
	}

	for(unsigned int i=0; i<this->rows*this->cols; i++){
		this->array[i] -= matrix.array[i];
	}
	return *this;
}

Matrix operator+(const Matrix& m1, const Matrix& m2)
{
	Matrix m(m1);

	m += m2;
	return m;
}

Matrix operator-(const Matrix& m1, const Matrix& m2)
{
	Matrix m(m1);

	m -= m2;
	return m;
}

Matrix& Matrix::operator*=(const Matrix& matrix)
{
	if(this->cols != matrix.rows)
		throw std::invalid_argument("Second dimension of first matrix does not match first dimension of second matrix.");

	double *arr = new double[matrix.cols*this->rows];
	memset(arr, 0, sizeof(double)*matrix.cols*this->rows);

	#pragma omp parallel for
	for( unsigned int i=0; i<this->rows; i++){
		for( unsigned int k=0; k<this->cols; k++){
			#pragma GCC ivdep
			for( unsigned int j=0; j<matrix.cols; j++){
				arr[i * matrix.cols + j] += (*this)[i][k] * matrix[k][j];
			}
		}
	}

	delete[] this->array;
	this->array = arr;
	this->cols = matrix.cols;

	return *this;
}

Matrix operator*(const Matrix& m1, const Matrix& m2)
{
	Matrix m(m1);

	m *= m2;
	return m;
}

Matrix& Matrix::operator*=(const double value)
{

	for( unsigned int i=0; i<this->rows*this->cols; i++){
		this->array[i] *= value;
	}
	return *this;
}

Matrix& Matrix::operator/=(const double value)
{
	*this *= (1/value);
	return *this;
}

// =============================================================== //
// Indexing operator
double* Matrix::operator[](unsigned int i) const
{
	if(i >= this->rows)
		throw std::invalid_argument("Index out of bounds");

	return &(this->array[i * this->cols]);
}

// =============================================================== //
// Matrix functions
Matrix Matrix::exp(const double tol) const
{
	if(this->rows != this->cols)
		throw std::invalid_argument("Matrix exponential only defined for square matrices");

	// Get number iterations required for tolerance
	// int N = 0;
	// double x = this->norm();
	// double err = (x / 2) * (1 + (x * (x + 1)) / (x + 2));

	// while (err > tol) {
	// 	++N;
	// 	err *= (x/(N+1));
	// }
	// if (N < x) 
	// 	N = ceil(x); // make sure N >= x

	// printf("Iterations: %d\n", N);

	// Hard coded number of iterations for now.
	int N = 30; 

	// Do exponentiation through Horners scheme
	Matrix I = Matrix::eye(this->rows);
	Matrix res = I;

	for(int i=N; i>0; i--){
		res *= *this;
		res *= (double)1/(double)i;
		res += I;
	}

	return res;
}

Matrix Matrix::transpose() const
{
	Matrix m(this->cols, this->rows);

	#pragma omp parallel for
	for(unsigned int i=0; i<this->rows; i++){
		#pragma GCC ivdep
		for(unsigned int j=0; j<this->cols; j++){
			m[j][i] = (*this)[i][j];
		}
	}
	return m;
}

double Matrix::norm() const 
{
	double res = 0;

	#pragma omp parallel for reduction(+:res)
	for (unsigned long i=0; i<this->rows*this->cols; i++){
		res += this->array[i]*this->array[i];
	}

	return sqrt(res);
}

// =============================================================== //
// Helpful stuff
void Matrix::print() const
{

	printf("%dx%d -> [\n", this->rows, this->cols);
	for (unsigned int i=0; i<this->rows; i++){
		printf(" [");
		for (unsigned int j=0; j<this->cols; j++){
			if (j > 0)
				printf(", ");
			printf(" %.4f", (*this)[i][j]);
		}
		printf("],\n");
	}
	printf("]\n");
}

void Matrix::fillMatrix(
	double array[],
	unsigned int lx,
	unsigned int ly,
	unsigned int ox,
	unsigned int oy) 
{
	for (unsigned int i=0; i<lx; i++){
		for (unsigned int j=0; j<ly; j++){
			(*this)[ox+i][oy+j] = array[i*ly + j];
		}
	}
}

inline unsigned int Matrix::index(unsigned int i, unsigned int j) const
{
	return i * this->cols + j;
}

double* Matrix::getArray() const
{
	return this->array;
}

unsigned Matrix::getRows() const {
	return rows;
}

unsigned Matrix::getCols() const {
	return cols;
}