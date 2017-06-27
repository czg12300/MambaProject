#ifndef _liblbp_h_
#define _liblbp_h_

#include <stdint.h>
#define LIBLBP_INDEX(row,col,num_rows) ((col)*(num_rows)+(row))
#define LIBLBP_MIN(a,b) ((a) > (b) ? (b) : (a))

namespace e {
	//typedef long unsigned int t_index;
	typedef uint32_t t_index;

	extern void liblbp_pyr_features_sparse(t_index *vec
		, uint32_t vec_nDim
		, uint32_t *img
		, uint16_t img_nRows
		, uint16_t img_nCols);

	extern void liblbp_pyr_features(char *vec
		, uint32_t vec_nDim
		, uint32_t *img
		, uint16_t img_nRows
		, uint16_t img_nCols);

	extern double liblbp_pyr_dotprod(double *vec
		, uint32_t vec_nDim
		, uint32_t *img
		, uint16_t img_nRows
		, uint16_t img_nCols);

	extern void liblbp_pyr_addvec(int64_t *vec
		, uint32_t vec_nDim
		, uint32_t *img
		, uint16_t img_nRows
		, uint16_t img_nCols);

	extern void liblbp_pyr_subvec(int64_t *vec
		, uint32_t vec_nDim
		, uint32_t *img
		, uint16_t img_nRows
		, uint16_t img_nCols);

	extern uint32_t liblbp_pyr_get_dim(uint16_t img_nRows
		, uint16_t img_nCols
		, uint16_t nPyramids);
}
#endif
