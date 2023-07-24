
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include <math.h>

#include "matrix.h"

double matget_data(matrix_t *mat, uint32_t i, uint32_t j) {
	return mat->data[i*mat->col+j];
}
void matset_data(matrix_t *mat, uint32_t i, uint32_t j, double val) {
	mat->data[i*mat->col+j] = val;
}
matrix_t mat_create(uint32_t row, uint32_t col, double *data) {
	matrix_t mat = {
		.row = row,
		.col = col,
		.data = data,
		.get = matget_data,
		.set = matset_data,
		.fast = true
	};
	return mat;
}


double matget_data_Trans(matrix_t *mat, uint32_t i, uint32_t j) {
	return mat->data[j*mat->col+i];
}
void matset_data_Trans(matrix_t *mat, uint32_t i, uint32_t j, double val) {
	mat->data[j*mat->col+i] = val;
}
matrix_t mat_createTrans(uint32_t row, uint32_t col, double *data) {
	matrix_t mat = {
		.row = row,
		.col = col,
		.data = data,
		.get = matget_data_Trans,
		.set = matset_data_Trans,
		.fast = false
	};
	return mat;
}


void mat_mult_fast(matrix_t r, matrix_t a, matrix_t b) {
	int p = a.col;
	int i,j,k;
	//printf("fast\n");
    for (i = 0; i < a.row; i++) {
		for (j = 0; j < b.col; j++) {
			double sum = 0;
			for (k = 0; k < p; k++) {
				sum += MATGETFAST(a,i,k) * MATGETFAST(b,k,j);
			}
			MATSETFAST(r, i, j, sum);
		}
    }

}

void mat_mult(matrix_t r, matrix_t a, matrix_t b) {
	if(!r.set || !a.get || !b.get) {
		printf("bad getter/setter");
		return;
	}
	if(a.col != b.row || a.row > r.row || b.col > r.col) {
		printf("Size mismatch");
		return;
	}
	if(r.fast && a.fast && b.fast) {
		mat_mult_fast(r,a,b);
		return;
	}
	int p = a.col;
	int i,j,k;
    for (i = 0; i < a.row; i++) {
		for (j = 0; j < b.col; j++) {
			double sum = 0;
			for (k = 0; k < p; k++) {
				sum += a.get(&a,i,k)*b.get(&b,k,j);
			}
			r.set(&r, i, j, sum);
		}
    }
}

void mat_add(matrix_t r, matrix_t a, matrix_t b) {
	if(!r.set || !a.get || !b.get) {
		printf("bad getter/setter");
		return;
	}
	if(a.row != b.row || a.col != b.col) {
		printf("Size mismatch");
		return;
	}
	int i,j;
    for (i = 0; i < a.row; i++) {
		for (j = 0; j < b.col; j++) {
			r.set(&r, i, j, a.get(&a,i,j)+b.get(&b,i,j));
		}
    }
}

//#define INV_SWAP(a,b) {temp=(a);(a)=(b);(b)=temp;}

bool mat_inv(matrix_t mat)
{
	#define INV_VECTORMAX 9
	if(mat.col != mat.row) {
		printf("matInv: not square\r\n");
		return false;
	}
	int n = mat.col;

	int indxc[INV_VECTORMAX+1];
	int indxr[INV_VECTORMAX+1];
	int ipiv[INV_VECTORMAX+1];

	int i,icol=0,irow=0,j,k,l,ll;
	double big,dum,pivinv,tmp1,tmp2;

	for (j=1;j<=n;j++) ipiv[j]=0;
	for (i=1;i<=n;i++) {
		big=0.0;
		for (j=1;j<=n;j++)
			if (ipiv[j] != 1)
				for (k=1;k<=n;k++) {
					if (ipiv[k] == 0) {
						if (fabs(MATGET1(mat,j,k)) >= big) { //if (fabs(a[j][k]) >= big) {
							big=fabs(MATGET1(mat,j,k)); //big=fabs(a[j][k]);
							irow=j;
							icol=k;
						}
					}
				}
		++(ipiv[icol]);
		if (irow != icol) {
			for (l=1;l<=n;l++) { //INV_SWAP(a[irow][l],a[icol][l])
				tmp1 = MATGET1(mat,irow,l);
				tmp2 = MATGET1(mat,icol,l);
				MATSET1(mat,irow,l, tmp2);
				MATSET1(mat,icol,l, tmp1);
			}
		}
		indxr[i]=irow;
		indxc[i]=icol;
		if (MATGET1(mat,icol,icol) == 0.0) { //(a[icol][icol] == 0.0) {
			printf("matInv: Singular Matrix\r\n");
			return false;
		}
		pivinv=1.0/MATGET1(mat,icol,icol); //a[icol][icol];
		MATSET1(mat,icol,icol, 1.0);//a[icol][icol]=1.0;
		for (l=1;l<=n;l++) MATSET1(mat,icol,l, MATGET1(mat,icol,l) * pivinv);//a[icol][l] *= pivinv;
		for (ll=1;ll<=n;ll++)
			if (ll != icol) {
				dum=MATGET1(mat,ll,icol); //dum=a[ll][icol];
				MATSET1(mat,ll,icol,0.0); //a[ll][icol]=0.0;
				for (l=1;l<=n;l++) MATSET1(mat,ll,l, MATGET1(mat,ll,l) - MATGET1(mat,icol,l) * dum); //a[ll][l] -= a[icol][l]*dum;
			}
	}
	for (l=n;l>=1;l--) {
		if (indxr[l] != indxc[l])
			for (k=1;k<=n;k++) { //INV_SWAP(a[k][indxr[l]],a[k][indxc[l]]);
				tmp1 = MATGET1(mat,k,indxr[l]);
				tmp2 = MATGET1(mat,k,indxc[l]);
				MATSET1(mat,k,indxr[l], tmp2);
				MATSET1(mat,k,indxc[l], tmp1);
			}	
	}
	return true;
}
/*
#define ROTATE(a,i,j,k,l) 
g=a[i][j];
h=a[k][l];
a[i][j]=g-s*(h+g*tau);
a[k][l]=h+s*(g-h*tau);
*/

#define ROTATE(m,i,j,k,l) \
g=MATGET1(m,i,j);\
h=MATGET1(m,k,l);\
MATSET1(m,i,j,g-s*(h+g*tau));\
MATSET1(m,k,l,h+s*(g-h*tau));


void mat_jacobi(matrix_t a, double d[], matrix_t v) //int *nrot
{
	#define JAC_VECTORMAX 3
	int j,iq,ip,i;
	double tresh,theta,tau,t,sm,s,h,g,c;
	int n = JAC_VECTORMAX;
	double b[JAC_VECTORMAX+1];
	double z[JAC_VECTORMAX+1];

	for (ip=1;ip<=n;ip++) {
		for (iq=1;iq<=n;iq++) MATSET1(v,ip,iq,0.0); //v[ip][iq]=0.0;
		MATSET1(v,ip,ip,1.0);//v[ip][ip]=1.0;
	}
	for (ip=1;ip<=n;ip++) {
		b[ip]=d[ip]=MATGET1(a,ip,ip); //b[ip]=d[ip]=a[ip][ip];
		z[ip]=0.0;
	}
	int nrot=0;
	for (i=1;i<=50;i++) {
		sm=0.0;
		for (ip=1;ip<=n-1;ip++) {
			for (iq=ip+1;iq<=n;iq++)
				sm += fabs(MATGET1(a,ip,iq)); //sm += fabs(a[ip][iq]);
		}
		if (sm == 0.0) {
			for(i = 1; i <=3; i++) {
				d[i-1] = d[i];
			}
			//printf("nrot: %d\n", nrot);
			return;
		}
		if (i < 4)
			tresh=0.2*sm/(n*n);
		else
			tresh=0.0;
		for (ip=1;ip<=n-1;ip++) {
			for (iq=ip+1;iq<=n;iq++) {
				g=100.0*fabs(MATGET1(a,ip,iq)); //g=100.0*fabs(a[ip][iq]);
				if (i > 4 && (fabs(d[ip])+g) == fabs(d[ip])
					&& (fabs(d[iq])+g) == fabs(d[iq]))
					MATSET1(a,ip,iq,0.0); //a[ip][iq]=0.0;
				else if (fabs(MATGET1(a,ip,iq)) > tresh) { //else if (fabs(a[ip][iq]) > tresh) {
					h=d[iq]-d[ip];
					if ((fabs(h)+g) == fabs(h))
						t=(MATGET1(a,ip,iq))/h; //t=(a[ip][iq])/h;
					else {
						theta=0.5*h/(MATGET1(a,ip,iq)); //theta=0.5*h/(a[ip][iq]);
						t=1.0/(fabs(theta)+sqrt(1.0+theta*theta));
						if (theta < 0.0) t = -t;
					}
					c=1.0/sqrt(1+t*t);
					s=t*c;
					tau=s/(1.0+c);
					h=t*MATGET1(a,ip,iq); //h=t*a[ip][iq];
					z[ip] -= h;
					z[iq] += h;
					d[ip] -= h;
					d[iq] += h;
					MATSET1(a,ip,iq,0.0); //a[ip][iq]=0.0;
					for (j=1;j<=ip-1;j++) {
						ROTATE(a,j,ip,j,iq)
					}
					for (j=ip+1;j<=iq-1;j++) {
						ROTATE(a,ip,j,j,iq)
					}
					for (j=iq+1;j<=n;j++) {
						ROTATE(a,ip,j,iq,j)
					}
					for (j=1;j<=n;j++) {
						ROTATE(v,j,ip,j,iq)
					}
					++(nrot);
				}
			}
		}
		for (ip=1;ip<=n;ip++) {
			b[ip] += z[ip];
			d[ip]=b[ip];
			z[ip]=0.0;
		}
	}
	printf("Too many iterations in routine jacobi");
}


void mat_copy(matrix_t d, matrix_t s) {
	if(!d.set || !s.get) {
		printf("bad getter/setter");
		return;
	}
	int i,j;
	int p = d.row*d.col;
	for (i = 0; i < p; i++) {
		d.data[i] = 0;
	}
    for (i = 0; i < d.row; i++) {
		for (j = 0; j < d.col; j++) {
			MATSET(d,i,j,MATGET(s,i,j));
		}
    }
}


void mat_identity(matrix_t d) {
	if(!d.set) {
		printf("bad getter/setter");
		return;
	}
	int i;
	int p = d.row*d.col;
	for (i = 0; i < p; i++) {
		d.data[i] = 0;
	}
    for (i = 0; i < d.row; i++) {
		MATSET(d,i,i,1.0);
    }
}


void mat_print(matrix_t mat) {
    for(int i = 0; i < mat.row; i++) {
	    for(int j = 0; j < mat.col; j++) {
		    printf("%g, ", MATGET(mat, i,j));
	    }
	    printf("\r\n");
    }
    printf("\r\n");
}


//typedef struct {
//    double i,j,k;
//} vector_t;

double mat_dotProduct(matrix_t a, matrix_t b)
{
    //return a.i*b.i + a.j*b.j + a.k*b.k;
    return a.data[0]*b.data[0] + a.data[1]*b.data[1] + a.data[2]*b.data[2];
}
 
void mat_crossProduct(matrix_t r, matrix_t a, matrix_t b)
{
	/*
    vector_t c = {
        a.j*b.k - a.k*b.j, 
        a.k*b.i - a.i*b.k, 
        a.i*b.j - a.j*b.i
    };
    return c;
    */
    r.data[0] = a.data[1]*b.data[2] - a.data[2]*b.data[1];
    r.data[1] = a.data[2]*b.data[0] - a.data[0]*b.data[2];
    r.data[2] = a.data[0]*b.data[1] - a.data[1]*b.data[0];
}

/*double mat_norm(vector_t a)
{
    //return sqrt(a.i*a.i + a.j*a.j + a.k*a.k);
    return sqrt(a.data[0]*a.data[0] + a.data[1]*a.data[1] + a.data[2]*a.data[2]);
}*/

//function R = fcn_RotationFromTwoVectors(A, B) 
//v = cross(A,B); 
//ssc = [0 -v(3) v(2); v(3) 0 -v(1); -v(2) v(1) 0]; 
//p = (1-dot(A,B))/(norm(v))^2
//R = eye(3) + ssc + (ssc^2)*p;

void mat_getRotationFromVectors(matrix_t r, matrix_t a, matrix_t b) {
	int i;
	bool equals = true;
	for(i = 0; i < 3; i++) {
		equals &= (a.data[i] == b.data[i]);
	}
	if(equals) {
		mat_identity(r);
		return;
	}

	double m_v[3*1];
	matrix_t mat_v = mat_create(3,1,m_v);
	mat_crossProduct(mat_v, a, b);

	double m_ssc[3*3] = {
		//0 -v(3) v(2)
		0, -m_v[2], m_v[1],
		//v(3) 0 -v(1)
		m_v[2], 0, -m_v[0],
		//-v(2) v(1) 0
		-m_v[1], m_v[0], 0
	}; 
	matrix_t mat_ssc = mat_create(3,3,m_ssc);

	double p = (1.0 - mat_dotProduct(a,b)) / (m_v[0]*m_v[0] + m_v[1]*m_v[1] + m_v[2]*m_v[2]);

	double m_ssc2[3*3];
	matrix_t mat_ssc2 = mat_create(3,3,m_ssc2);
	mat_mult(mat_ssc2, mat_ssc, mat_ssc);
	for(i = 0; i < 3*3; i++) {
		m_ssc2[i] *= p;
	}

	double m_eye[3*3];
	matrix_t mat_eye = mat_create(3,3,m_eye);
	mat_identity(mat_eye);

	double m_sscSum[3*3];
	matrix_t mat_sscSum = mat_create(3,3,m_sscSum);

	mat_add(mat_sscSum, mat_ssc, mat_ssc2);
	mat_add(r, mat_eye, mat_sscSum);

}
