
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>

#include "matrix.h"

#include "calibration.h"

double *cal_x;
double *cal_y;
double *cal_z;

//#define D_DIM 9 	// ellipse fitting (Do not use, missing axis rotation transformation to match device axis)
#define D_DIM 6 	// ellipse fitting with same axis
//#define D_DIM 4	// sphere fitting

/*
d2 = x .* x + y .* y + z .* z
*/
double matget_d2(matrix_t *mat, uint32_t i, uint32_t j) {
	return cal_x[i] * cal_x[i] + cal_y[i] * cal_y[i] + cal_z[i] * cal_z[i];
}

/*
    D = [ x .* x + y .* y - 2 * z .* z, ...
        x .* x + z .* z - 2 * y .* y, ...
        2 * x .* y, ...
        2 * x .* z, ...
        2 * y .* z, ...
        2 * x, ...
        2 * y, ...
        2 * z, ...
        1 + 0 * x ];
*/
#if (D_DIM == 9)

double matget_D(matrix_t *mat, uint32_t i, uint32_t j) {
	switch(j) {
		case 0:
			return cal_x[i] * cal_x[i] + cal_y[i] * cal_y[i] - 2 * cal_z[i] * cal_z[i];
		case 1:
			return cal_x[i] * cal_x[i] + cal_z[i] * cal_z[i] - 2 * cal_y[i] * cal_y[i];
		case 2:
			return 2 * cal_x[i] * cal_y[i];
		case 3:
			return 2 * cal_x[i] * cal_z[i];
		case 4:
			return 2 * cal_y[i] * cal_z[i];
		case 5:
			return 2 * cal_x[i];
		case 6:
			return 2 * cal_y[i];
		case 7:
			return 2 * cal_z[i];
		case 8:
			return 1;
		default:
			return 0;
	}
}

#endif
/*
    D = [ x .* x + y .* y - 2 * z .* z, ...
          x .* x + z .* z - 2 * y .* y, ...
          2 * x, ...
          2 * y, ... 
          2 * z, ... 
          1 + 0 * x ]; 
*/

#if (D_DIM == 6)

double matget_D(matrix_t *mat, uint32_t i, uint32_t j) {
	switch(j) {
		case 0:
			return cal_x[i] * cal_x[i] + cal_y[i] * cal_y[i] - 2 * cal_z[i] * cal_z[i];
		case 1:
			return cal_x[i] * cal_x[i] + cal_z[i] * cal_z[i] - 2 * cal_y[i] * cal_y[i];
		case 2:
			return 2 * cal_x[i];
		case 3:
			return 2 * cal_y[i];
		case 4:
			return 2 * cal_z[i];
		case 5:
			return 1;
		default:
			return 0;
	}
}

#endif

/*
    D = [ 2 * x, ...
          2 * y, ... 
          2 * z, ... 
          1 + 0 * x ];
*/

#if (D_DIM == 4)

double matget_D(matrix_t *mat, uint32_t i, uint32_t j) {
	switch(j) {
		case 0:
			return 2 * cal_x[i];
		case 1:
			return 2 * cal_y[i];
		case 2:
			return 2 * cal_z[i];
		case 3:
			return 1;
		default:
			return 0;
	}
}

#endif

double matget_Dp(matrix_t *mat, uint32_t i, uint32_t j) {
	return matget_D(mat, j, i);
}

void cal_do(double *x, double *y, double *z, uint32_t ptr, double *center, double *radii)
{
    int i;
    cal_x = x;
    cal_y = y;
    cal_z = z;

	double m_tmp3[3*3];
	matrix_t mat_tmp3 = mat_create(3,3,m_tmp3);
	double m_tmp4[4*4];
	matrix_t mat_tmp4 = mat_create(4,4,m_tmp4);


//d2 = x .* x + y .* y + z .* z; % the RHS of the llsq problem (y's)
//u = inv( D' * D ) * ( D' * d2 );  % solution to the normal equations

	matrix_t mat_D = {
		.row = ptr,
		.col = D_DIM,
		.data = NULL,
		.get = matget_D,
		.set = NULL,
		.fast = false
	};

	matrix_t mat_Dp = {
		.row = D_DIM,
		.col = ptr,
		.data = NULL,
		.get = matget_Dp,
		.set = NULL,
		.fast = false
	};

	matrix_t mat_d2 = {
		.row = ptr,
		.col = 1,
		.data = NULL,
		.get = matget_d2,
		.set = NULL,
		.fast = false
	};

	double m_Dp_D[D_DIM*D_DIM];
	matrix_t mat_Dp_D = mat_create(D_DIM,D_DIM,m_Dp_D);
	double m_Dp_d2[D_DIM*1];
	matrix_t mat_Dp_d2 = mat_create(D_DIM,1,m_Dp_d2);
	double m_u[D_DIM*1];
	matrix_t mat_u = mat_create(D_DIM,1,m_u);

	mat_mult(mat_Dp_D, mat_Dp, mat_D);

	mat_mult(mat_Dp_d2, mat_Dp, mat_d2);

	mat_inv(mat_Dp_D);

	mat_mult(mat_u, mat_Dp_D, mat_Dp_d2);
	//mat_print(mat_u);
	
	double m_v[10];

#if (D_DIM == 9)
// DIM 9
//    v(0) = u(0) +     u(1) - 1;
//    v(1) = u(0) - 2 * u(1) - 1;
//    v(2) = u(1) - 2 * u(0) - 1;
//    v( 3 : 9 ) = u( 2 : 8 );
	//matrix_t mat_v = mat_create(10,1,m_v);
	m_v[0] = m_u[0] + m_u[1] - 1;
	m_v[1] = m_u[0] - 2 * m_u[1] - 1;
	m_v[2] = m_u[1] - 2 * m_u[0] - 1;
	for(i = 2; i < 9; i++) {
		m_v[i+1] = m_u[i];
	}
	//mat_print(mat_v);
#endif

#if (D_DIM == 6)
//DIM 6
//    v(0) = u(0) +     u(1) - 1;
//    v(1) = u(0) - 2 * u(1) - 1;
//    v(2) = u(1) - 2 * u(0) - 1;
//    v = [ v(0) v(1) v(2) 0 0 0 u( 2 : 5 )' ];
	m_v[0] = m_u[0] + m_u[1] - 1;
	m_v[1] = m_u[0] - 2 * m_u[1] - 1;
	m_v[2] = m_u[1] - 2 * m_u[0] - 1;
	m_v[3] = 0;
	m_v[4] = 0;
	m_v[5] = 0;
	m_v[6] = m_u[2];
	m_v[7] = m_u[3];
	m_v[8] = m_u[4];
	m_v[9] = m_u[5];
#endif

#if (D_DIM == 4)
//DIM 4
//v = [ -1 -1 -1 0 0 0 u( 0 : 3 )' ];
	m_v[0] = -1;
	m_v[1] = -1;
	m_v[2] = -1;
	m_v[3] = 0;
	m_v[4] = 0;
	m_v[5] = 0;
	m_v[6] = m_u[0];
	m_v[7] = m_u[1];
	m_v[8] = m_u[2];
	m_v[9] = m_u[3];
#endif

/*
A = [ v(0) v(3) v(4) v(6); ...
      v(3) v(1) v(5) v(7); ...
      v(4) v(5) v(2) v(8); ...
      v(6) v(7) v(8) v(9) ]
*/
	double m_A[4*4];
	matrix_t mat_A = mat_create(4,4,m_A);
	MATSET(mat_A, 0, 0, m_v[0]);
	MATSET(mat_A, 0, 1, m_v[3]);
	MATSET(mat_A, 0, 2, m_v[4]);
	MATSET(mat_A, 0, 3, m_v[6]);

	MATSET(mat_A, 1, 0, m_v[3]);
	MATSET(mat_A, 1, 1, m_v[1]);
	MATSET(mat_A, 1, 2, m_v[5]);
	MATSET(mat_A, 1, 3, m_v[7]);

	MATSET(mat_A, 2, 0, m_v[4]);
	MATSET(mat_A, 2, 1, m_v[5]);
	MATSET(mat_A, 2, 2, m_v[2]);
	MATSET(mat_A, 2, 3, m_v[8]);

	MATSET(mat_A, 3, 0, m_v[6]);
	MATSET(mat_A, 3, 1, m_v[7]);
	MATSET(mat_A, 3, 2, m_v[8]);
	MATSET(mat_A, 3, 3, m_v[9]);
	//mat_print(mat_A);

//center = inv(-A( 1:3, 1:3 )) * v( 7:9 );
	//double m_center[3];
	matrix_t mat_center = mat_create(3,1,center);

	mat_copy(mat_tmp3, mat_A);
	for(i = 0; i < 3*3; i++) {
		m_tmp3[i] = -m_tmp3[i];
	}
	mat_inv(mat_tmp3);

	double *m_v_6_8 = m_v+6;
	matrix_t mat_v_6_8 = mat_create(3,1,m_v_6_8);

	mat_mult(mat_center, mat_tmp3, mat_v_6_8);
	//mat_print(mat_center);

//T = eye( 4 );
//T( 4, 1:3 ) = center';
	double m_T[4*4];
	matrix_t mat_T = mat_create(4,4,m_T);
	matrix_t mat_Tp = mat_createTrans(4,4,m_T);
	mat_identity(mat_T);
	MATSET(mat_T, 3, 0, center[0]);
	MATSET(mat_T, 3, 1, center[1]);
	MATSET(mat_T, 3, 2, center[2]);
	//mat_print(mat_T);

//R = T * A * T';
	double m_R[4*4];
	matrix_t mat_R = mat_create(4,4,m_R);

	mat_mult(mat_tmp4, mat_T, mat_A);
	mat_mult(mat_R, mat_tmp4, mat_Tp);
	//mat_print(mat_R);

//R1 = R( 1:3, 1:3 ) / -R( 4, 4 )
	double m_R1[3*3];
	matrix_t mat_R1 = mat_create(3,3,m_R1);
	mat_copy(mat_R1, mat_R);
	double Rdenom = -MATGET(mat_R,3,3);
	for(i = 0; i < 3*3; i++) {
		m_R1[i] /= Rdenom;
	}
	//mat_print(mat_R1);

//[ evecs, evals ] = eig( R1 )
	double m_evecs[3*3];
	matrix_t mat_evecs = mat_create(3,3,m_evecs);
	double m_evals[4] = {1,1,1};
	mat_jacobi(mat_R1, m_evals, mat_evecs);

	/*mat_print(mat_evecs);
	printf("\n");
	for(i = 0; i < 3; i++) {
		printf("%e\n", m_evals[i]);
	}*/

//radii = sqrt( 1 ./ diag( abs( evals ) ) );
//sgns = sign( diag( evals ) );
//radii = radii .* sgns
	//double m_radii[3];

	for(i = 0; i < 3; i++) {
		if(m_evals[i] != 0) {
			radii[i] = SIGN(m_evals[i]) * sqrt( 1 / fabs(m_evals[i]) );
		} else {
			radii[i] = 1;
		}
	}

	//printf("center\n");
	//mat_print(mat_center);

	//printf("evecs\n");
	//mat_print(mat_evecs);

	//printf("radii\n");
	//for(i = 0; i < 3; i++) {
	//	printf("%e,\n", m_radii[i]);
	//}

}

double matget_cir_u(matrix_t *mat, uint32_t i, uint32_t j) {
	switch(j) {
		case 0:
			return cal_x[i];
		case 1:
			return cal_y[i];
		case 2:
			return 1;
		default:
			return 0;
	}
}
double matget_cir_up(matrix_t *mat, uint32_t i, uint32_t j) {
	return matget_cir_u(mat, j, i);
}

double matget_cir_v(matrix_t *mat, uint32_t i, uint32_t j) {
	switch(j) {
		case 0:
			return -(cal_x[i]*cal_x[i] + cal_y[i]*cal_y[i]);
		default:
			return 0;
	}
}

bool cal_circle_fitting(double *x, double *y, uint32_t ptr, double *center, double *radius)
{
    cal_x = x;
    cal_y = y;

//    u=[x y ones(size(x))]
	matrix_t mat_u = {
		.row = ptr,
		.col = 3,
		.data = NULL,
		.get = matget_cir_u,
		.set = NULL,
		.fast = false
	};

// u transpose
	matrix_t mat_up = {
		.row = 3,
		.col = ptr,
		.data = NULL,
		.get = matget_cir_up,
		.set = NULL,
		.fast = false
	};

//    v=[-(x.^2+y.^2)]
	matrix_t mat_v = {
		.row = ptr,
		.col = 1,
		.data = NULL,
		.get = matget_cir_v,
		.set = NULL,
		.fast = false
	};

//    a=(inv(u'*u)*u')*v
	double m_up_u[3*3];
	matrix_t mat_up_u = mat_create(3,3,m_up_u);
	double m_inv_up[3*ptr];
	matrix_t mat_inv_up = mat_create(3,ptr,m_inv_up);
	double m_a[3];
	matrix_t mat_a = mat_create(3,1,m_a);

	mat_mult(mat_up_u, mat_up, mat_u);
	bool res = mat_inv(mat_up_u);
	if(!res) {
		return false;
	}
	mat_mult(mat_inv_up, mat_up_u, mat_up);
	mat_mult(mat_a, mat_inv_up, mat_v);

//   xc = -.5*a(1);
	center[0] = -0.5 * m_a[0];

//   yc = -.5*a(2);
	center[1] = -0.5 * m_a[1];

//   R  =  sqrt( ((a(1)^2+a(2)^2)/4) - a(3));
	*radius = sqrt( ((m_a[0]*m_a[0] + m_a[1]*m_a[1])/4.) - m_a[2] );

	return true;
}