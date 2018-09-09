#ifndef VEC_H
#define VEC_H

class vec{

	inline friend double operator*(const vec &left, const vec &right){
		if (left.dim != right.dim){
			// cerr << "dot product failed: vectors of different dimension" << endl;
			return 0;
		}
		double result = 0;
		int i;
		for (i = 0; i < left.dim; i++) result += left.comp[i] * right.comp[i];
		return result;
	}

	inline friend bool operator==(const vec &left, const vec &right){
		if (left.dim != right.dim) return false;
		int i;
		for (i = 0; i < left.dim; i++) if (left.comp[i] != right.comp[i]) return false;
		return true;
	}

public:
	vec(); // default
	vec(int);     // vector of dimension n
	vec(double*, int); // vector with components of array double*
	vec(const vec&);
	~vec();

	vec& operator=(const vec&);
	vec& operator+=(const vec&);
	vec& operator-=(const vec&);
	vec& operator*=(const double);
	vec& operator/=(const double);
	double& operator[](const int);

	int get_dim();
	double v_abs();
    void normalize();

private:
	double *comp;
	int dim;
};

inline vec operator*(const double left, vec right){
	right *= left;
	return right;
}

inline vec operator*(vec left, const double right){
	left *= right;
	return left;
}

inline vec operator/(const vec &left, const double right){
	return (1 / right)*left;
}

inline vec operator+(vec left, const vec &right){
	left += right;
	return left;
}

inline vec operator-(vec left, const vec &right){
	left += (-1)*right;
	return left;
}

inline bool operator!=(const vec &left, const vec &right){
	return !operator==(left, right);
}

#endif // VEC_H
