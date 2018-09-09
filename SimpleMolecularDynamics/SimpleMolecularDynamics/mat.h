#ifndef MAT_H
#define MAT_H

class vec;

class mat{

	friend mat operator*(const double, mat);
	friend mat operator*(mat, const double);
	friend mat operator/(mat, const double);
	friend mat operator+(mat, const mat&);
	friend mat operator-(mat, const mat&);
	//friend mat operator*(const mat&, const mat&);
	//friend vec operator*(const mat&, vec);
	friend bool operator==(const mat&, const mat&);
	friend bool operator!=(const mat&, const mat&);

public:
	mat(); //default
	mat(int, int); // row und col count
	mat(vec[], int); // just row count, col count is dimension of vectors
	mat(const mat&);
	~mat();

	mat& operator=(const mat&);
	mat& operator+=(const mat&);
	mat& operator-=(const mat&);
	mat& operator*=(const double);
	mat& operator/= (const double);
	vec& operator[](const int); // int is row-index

	int get_cols(); // dimension of row-vectors
	int get_rows(); // return rows
	void fill(double);
	double get_max();
	double get_min();
	//double det(); // Determinante

private:
	vec *row;
	int rows;
	int cols;
};

#endif // MAT_H
