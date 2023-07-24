#ifndef _MATRIX_H
#define _MATRIX_H

#define SIGN(x) (((x) > 0) ? 1 : (((x) < 0) ? -1 : 0))

typedef struct matrix_def matrix_t;

typedef double (*matrixGetter_t)( matrix_t *, uint32_t, uint32_t );
typedef void (*matrixSetter_t)( matrix_t *, uint32_t, uint32_t, double );

struct matrix_def {
	uint32_t row;
	uint32_t col;
	double *data;
	matrixGetter_t get;
	matrixSetter_t set;
	bool fast;
};

#define MATGETFAST(m,i,j) (*(m.data+(i)*m.col+(j)))
#define MATSETFAST(m,i,j,v) (*(m.data+(i)*m.col+(j)) = v)

#define MATGET(m, i, j) (m.get(&m,(i),(j)))
#define MATSET(m, i, j, v) (m.set(&m,(i),(j), v))

#define MAT_T_GET(m, i, j) (m.get(&m,(j),(i)))
#define MAT_T_SET(m, i, j, v) (m.set(&m,(j),(i), v))

#define MATGET1(m, i, j) (m.get(&m,(i)-1,(j)-1))
#define MATSET1(m, i, j, v) (m.set(&m,(i)-1,(j)-1, v))


matrix_t mat_create(uint32_t row, uint32_t col, double *data);
matrix_t mat_createTrans(uint32_t row, uint32_t col, double *data);
void mat_print(matrix_t mat);
void mat_identity(matrix_t d);
void mat_copy(matrix_t d, matrix_t s);
void mat_jacobi(matrix_t a, double d[], matrix_t v);
bool mat_inv(matrix_t mat);
void mat_mult(matrix_t r, matrix_t a, matrix_t b);

void mat_getRotationFromVectors(matrix_t r, matrix_t a, matrix_t b);

#endif