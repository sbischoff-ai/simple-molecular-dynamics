#include "vec.h"
#include "mat.h"
#include <cmath>
#include <iostream>
using namespace std;

mat::mat(){
	this->row = 0;
	this->rows = 0;
        this->cols = 0;
}

mat::mat(int r, int c){
	int i;
	this->rows = r;
        this->cols = c;
	this->row = new vec[r];
	for (i = 0; i < r; i++) this->row[i] = vec(0, c);
}

mat::mat(vec v[], int r){
	int i;
	this->rows = r;
        this->cols = v[1].get_dim();
	this->row = new vec[r];
	for (i = 0; i < r; i++) {
                if ((i>0) && (v[i].get_dim() != cols)){
			this->row[i] = vec(v[i - 1].get_dim());
			cerr << "matrix construction failed: row vector dimensions don't match" << endl;
		}
		else this->row[i] = v[i];
	}
}

mat::mat(const mat &other){
	int i;
	this->rows = other.rows;
        this->cols = other.cols;
	this->row = new vec[other.rows];
	for (i = 0; i < this->rows; i++) this->row[i] = other.row[i];
}

mat::~mat(){
	if (this->row) delete[] this->row;
}

mat& mat::operator=(const mat &right){
	if (this == &right) return (*this);
	if (this->rows != right.rows){
		if (this->row) delete[] this->row;
		this->row = new vec[right.rows];
	}
	int i;
	for (i = 0; i < right.rows; i++) this->row[i] = right.row[i];
	this->rows = right.rows;
        this->cols = right.cols;
	return (*this);
}

mat& mat::operator+=(const mat &right){
	if (this->rows != right.rows){
		cerr << "matrix addition failed: matrices of different row-count" << endl;
		return (*this);
	}
	int i;
	for (i = 0; i < right.rows; i++) this->row[i] += right.row[i];
	return (*this);
}

mat& mat::operator-=(const mat &right){
	(*this) += (-1)*(*this);
	return (*this);
}

mat& mat::operator*=(const double right){
	int i;
	for (i = 0; i < this->rows; i++) this->row[i] *= right;
	return (*this);
}

mat& mat::operator/=(const double right){
	(*this) *= (1 / right);
	return (*this);
}

vec& mat::operator[](const int i){
    if ((this->rows < i)||(i<1)) cerr << "matrix row index invalid" << endl;
	return this->row[i - 1];
}

int mat::get_cols(){
        return this->cols;
}

int mat::get_rows(){
	return this->rows;
}

void mat::fill(double x){
        int i, j;
        for(i = 0; i < this->rows; i++) for(j = 1; j <= this->cols; j++) this->row[i][j] = x;
}

double mat::get_max(){
    int i, j;
    double result = this->row[0][1];
    for(i = 0; i < this->rows; i++) for(j = 1; j <= this->cols; j++){
        if(this->row[i][j] > result) result = this->row[i][j];
    }
    return result;
}

double mat::get_min(){
    int i, j;
    double result = this->row[0][1];
    for(i = 0; i < this->rows; i++) for(j = 1; j <= this->cols; j++){
        if(this->row[i][j] < result) result = this->row[i][j];
    }
    return result;
}

mat operator*(const double left, mat right){
	right *= left;
	return right;
}

mat operator*(mat left, const double right){
	left *= right;
	return left;
}

mat operator/(mat left, const double right){
	left *= (1 / right);
	return left;
}

mat operator+(mat left, const mat &right){
	left += right;
	return left;
}

mat operator-(mat left, const mat &right){
	left += (-1)*right;
	return left;
}

bool operator==(const mat &left, const mat &right){
	if (left.rows != right.rows) return false;
	int i;
	for (i = 0; i < left.rows; i++) if (left.row[i] != right.row[i]) return false;
	return true;
}

bool operator!=(mat &left, mat &right){
	return !operator==(left, right);
}
