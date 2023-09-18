// This file is generated. Do not edit.
#ifndef VP8_RTCD_H_
#define VP8_RTCD_H_

#ifdef RTCD_C
#define RTCD_EXTERN
#else
#define RTCD_EXTERN extern
#endif

/*
 * VP8
 */

struct blockd;
struct macroblockd;
struct loop_filter_info;

/* Encoder forward decls */
struct block;
struct macroblock;
struct variance_vtable;
union int_mv;
struct yv12_buffer_config;

#ifdef __cplusplus
extern "C" {
#endif

void vp8_bilinear_predict16x16_c(unsigned char *src_ptr, int src_pixels_per_line, int xoffset, int yoffset, unsigned char *dst_ptr, int dst_pitch);
void vp8_bilinear_predict16x16_sse2(unsigned char *src_ptr, int src_pixels_per_line, int xoffset, int yoffset, unsigned char *dst_ptr, int dst_pitch);
void vp8_bilinear_predict16x16_ssse3(unsigned char *src_ptr, int src_pixels_per_line, int xoffset, int yoffset, unsigned char *dst_ptr, int dst_pitch);
RTCD_EXTERN void (*vp8_bilinear_predict16x16)(unsigned char *src_ptr, int src_pixels_per_line, int xoffset, int yoffset, unsigned char *dst_ptr, int dst_pitch);

void vp8_bilinear_predict4x4_c(unsigned char *src_ptr, int src_pixels_per_line, int xoffset, int yoffset, unsigned char *dst_ptr, int dst_pitch);
void vp8_bilinear_predict4x4_sse2(unsigned char *src_ptr, int src_pixels_per_line, int xoffset, int yoffset, unsigned char *dst_ptr, int dst_pitch);
#define vp8_bilinear_predict4x4 vp8_bilinear_predict4x4_sse2

void vp8_bilinear_predict8x4_c(unsigned char *src_ptr, int src_pixels_per_line, int xoffset, int yoffset, unsigned char *dst_ptr, int dst_pitch);
void vp8_bilinear_predict8x4_sse2(unsigned char *src_ptr, int src_pixels_per_line, int xoffset, int yoffset, unsigned char *dst_ptr, int dst_pitch);
#define vp8_bilinear_predict8x4 vp8_bilinear_predict8x4_sse2

void vp8_bilinear_predict8x8_c(unsigned char *src_ptr, int src_pixels_per_line, int xoffset, int yoffset, unsigned char *dst_ptr, int dst_pitch);
void vp8_bilinear_predict8x8_sse2(unsigned char *src_ptr, int src_pixels_per_line, int xoffset, int yoffset, unsigned char *dst_ptr, int dst_pitch);
void vp8_bilinear_predict8x8_ssse3(unsigned char *src_ptr, int src_pixels_per_line, int xoffset, int yoffset, unsigned char *dst_ptr, int dst_pitch);
RTCD_EXTERN void (*vp8_bilinear_predict8x8)(unsigned char *src_ptr, int src_pixels_per_line, int xoffset, int yoffset, unsigned char *dst_ptr, int dst_pitch);

void vp8_copy_mem16x16_c(unsigned char *src, int src_stride, unsigned char *dst, int dst_stride);
void vp8_copy_mem16x16_sse2(unsigned char *src, int src_stride, unsigned char *dst, int dst_stride);
#define vp8_copy_mem16x16 vp8_copy_mem16x16_sse2

void vp8_copy_mem8x4_c(unsigned char *src, int src_stride, unsigned char *dst, int dst_stride);
void vp8_copy_mem8x4_mmx(unsigned char *src, int src_stride, unsigned char *dst, int dst_stride);
#define vp8_copy_mem8x4 vp8_copy_mem8x4_mmx

void vp8_copy_mem8x8_c(unsigned char *src, int src_stride, unsigned char *dst, int dst_stride);
void vp8_copy_mem8x8_mmx(unsigned char *src, int src_stride, unsigned char *dst, int dst_stride);
#define vp8_copy_mem8x8 vp8_copy_mem8x8_mmx

void vp8_dc_only_idct_add_c(short input_dc, unsigned char *pred_ptr, int pred_stride, unsigned char *dst_ptr, int dst_stride);
void vp8_dc_only_idct_add_mmx(short input_dc, unsigned char *pred_ptr, int pred_stride, unsigned char *dst_ptr, int dst_stride);
#define vp8_dc_only_idct_add vp8_dc_only_idct_add_mmx

void vp8_dequant_idct_add_c(short *input, short *dq, unsigned char *dest, int stride);
void vp8_dequant_idct_add_mmx(short *input, short *dq, unsigned char *dest, int stride);
#define vp8_dequant_idct_add vp8_dequant_idct_add_mmx

void vp8_dequant_idct_add_uv_block_c(short *q, short *dq, unsigned char *dst_u, unsigned char *dst_v, int stride, char *eobs);
void vp8_dequant_idct_add_uv_block_sse2(short *q, short *dq, unsigned char *dst_u, unsigned char *dst_v, int stride, char *eobs);
#define vp8_dequant_idct_add_uv_block vp8_dequant_idct_add_uv_block_sse2

void vp8_dequant_idct_add_y_block_c(short *q, short *dq, unsigned char *dst, int stride, char *eobs);
void vp8_dequant_idct_add_y_block_sse2(short *q, short *dq, unsigned char *dst, int stride, char *eobs);
#define vp8_dequant_idct_add_y_block vp8_dequant_idct_add_y_block_sse2

void vp8_dequantize_b_c(struct blockd*, short *DQC);
void vp8_dequantize_b_mmx(struct blockd*, short *DQC);
#define vp8_dequantize_b vp8_dequantize_b_mmx

void vp8_filter_by_weight16x16_c(unsigned char *src, int src_stride, unsigned char *dst, int dst_stride, int src_weight);
void vp8_filter_by_weight16x16_sse2(unsigned char *src, int src_stride, unsigned char *dst, int dst_stride, int src_weight);
#define vp8_filter_by_weight16x16 vp8_filter_by_weight16x16_sse2

void vp8_filter_by_weight4x4_c(unsigned char *src, int src_stride, unsigned char *dst, int dst_stride, int src_weight);
#define vp8_filter_by_weight4x4 vp8_filter_by_weight4x4_c

void vp8_filter_by_weight8x8_c(unsigned char *src, int src_stride, unsigned char *dst, int dst_stride, int src_weight);
void vp8_filter_by_weight8x8_sse2(unsigned char *src, int src_stride, unsigned char *dst, int dst_stride, int src_weight);
#define vp8_filter_by_weight8x8 vp8_filter_by_weight8x8_sse2

void vp8_loop_filter_bh_c(unsigned char *y_ptr, unsigned char *u_ptr, unsigned char *v_ptr, int y_stride, int uv_stride, struct loop_filter_info *lfi);
void vp8_loop_filter_bh_sse2(unsigned char *y_ptr, unsigned char *u_ptr, unsigned char *v_ptr, int y_stride, int uv_stride, struct loop_filter_info *lfi);
#define vp8_loop_filter_bh vp8_loop_filter_bh_sse2

void vp8_loop_filter_bv_c(unsigned char *y_ptr, unsigned char *u_ptr, unsigned char *v_ptr, int y_stride, int uv_stride, struct loop_filter_info *lfi);
void vp8_loop_filter_bv_sse2(unsigned char *y_ptr, unsigned char *u_ptr, unsigned char *v_ptr, int y_stride, int uv_stride, struct loop_filter_info *lfi);
#define vp8_loop_filter_bv vp8_loop_filter_bv_sse2

void vp8_loop_filter_mbh_c(unsigned char *y_ptr, unsigned char *u_ptr, unsigned char *v_ptr, int y_stride, int uv_stride, struct loop_filter_info *lfi);
void vp8_loop_filter_mbh_sse2(unsigned char *y_ptr, unsigned char *u_ptr, unsigned char *v_ptr, int y_stride, int uv_stride, struct loop_filter_info *lfi);
#define vp8_loop_filter_mbh vp8_loop_filter_mbh_sse2

void vp8_loop_filter_mbv_c(unsigned char *y_ptr, unsigned char *u_ptr, unsigned char *v_ptr, int y_stride, int uv_stride, struct loop_filter_info *lfi);
void vp8_loop_filter_mbv_sse2(unsigned char *y_ptr, unsigned char *u_ptr, unsigned char *v_ptr, int y_stride, int uv_stride, struct loop_filter_info *lfi);
#define vp8_loop_filter_mbv vp8_loop_filter_mbv_sse2

void vp8_loop_filter_bhs_c(unsigned char *y_ptr, int y_stride, const unsigned char *blimit);
void vp8_loop_filter_bhs_sse2(unsigned char *y_ptr, int y_stride, const unsigned char *blimit);
#define vp8_loop_filter_simple_bh vp8_loop_filter_bhs_sse2

void vp8_loop_filter_bvs_c(unsigned char *y_ptr, int y_stride, const unsigned char *blimit);
void vp8_loop_filter_bvs_sse2(unsigned char *y_ptr, int y_stride, const unsigned char *blimit);
#define vp8_loop_filter_simple_bv vp8_loop_filter_bvs_sse2

void vp8_loop_filter_simple_horizontal_edge_c(unsigned char *y_ptr, int y_stride, const unsigned char *blimit);
void vp8_loop_filter_simple_horizontal_edge_sse2(unsigned char *y_ptr, int y_stride, const unsigned char *blimit);
#define vp8_loop_filter_simple_mbh vp8_loop_filter_simple_horizontal_edge_sse2

void vp8_loop_filter_simple_vertical_edge_c(unsigned char *y_ptr, int y_stride, const unsigned char *blimit);
void vp8_loop_filter_simple_vertical_edge_sse2(unsigned char *y_ptr, int y_stride, const unsigned char *blimit);
#define vp8_loop_filter_simple_mbv vp8_loop_filter_simple_vertical_edge_sse2

void vp8_short_idct4x4llm_c(short *input, unsigned char *pred_ptr, int pred_stride, unsigned char *dst_ptr, int dst_stride);
void vp8_short_idct4x4llm_mmx(short *input, unsigned char *pred_ptr, int pred_stride, unsigned char *dst_ptr, int dst_stride);
#define vp8_short_idct4x4llm vp8_short_idct4x4llm_mmx

void vp8_short_inv_walsh4x4_c(short *input, short *mb_dqcoeff);
void vp8_short_inv_walsh4x4_sse2(short *input, short *mb_dqcoeff);
#define vp8_short_inv_walsh4x4 vp8_short_inv_walsh4x4_sse2

void vp8_short_inv_walsh4x4_1_c(short *input, short *mb_dqcoeff);
#define vp8_short_inv_walsh4x4_1 vp8_short_inv_walsh4x4_1_c

void vp8_sixtap_predict16x16_c(unsigned char *src_ptr, int src_pixels_per_line, int xoffset, int yoffset, unsigned char *dst_ptr, int dst_pitch);
void vp8_sixtap_predict16x16_sse2(unsigned char *src_ptr, int src_pixels_per_line, int xoffset, int yoffset, unsigned char *dst_ptr, int dst_pitch);
void vp8_sixtap_predict16x16_ssse3(unsigned char *src_ptr, int src_pixels_per_line, int xoffset, int yoffset, unsigned char *dst_ptr, int dst_pitch);
RTCD_EXTERN void (*vp8_sixtap_predict16x16)(unsigned char *src_ptr, int src_pixels_per_line, int xoffset, int yoffset, unsigned char *dst_ptr, int dst_pitch);

void vp8_sixtap_predict4x4_c(unsigned char *src_ptr, int src_pixels_per_line, int xoffset, int yoffset, unsigned char *dst_ptr, int dst_pitch);
void vp8_sixtap_predict4x4_mmx(unsigned char *src_ptr, int src_pixels_per_line, int xoffset, int yoffset, unsigned char *dst_ptr, int dst_pitch);
void vp8_sixtap_predict4x4_ssse3(unsigned char *src_ptr, int src_pixels_per_line, int xoffset, int yoffset, unsigned char *dst_ptr, int dst_pitch);
RTCD_EXTERN void (*vp8_sixtap_predict4x4)(unsigned char *src_ptr, int src_pixels_per_line, int xoffset, int yoffset, unsigned char *dst_ptr, int dst_pitch);

void vp8_sixtap_predict8x4_c(unsigned char *src_ptr, int src_pixels_per_line, int xoffset, int yoffset, unsigned char *dst_ptr, int dst_pitch);
void vp8_sixtap_predict8x4_sse2(unsigned char *src_ptr, int src_pixels_per_line, int xoffset, int yoffset, unsigned char *dst_ptr, int dst_pitch);
void vp8_sixtap_predict8x4_ssse3(unsigned char *src_ptr, int src_pixels_per_line, int xoffset, int yoffset, unsigned char *dst_ptr, int dst_pitch);
RTCD_EXTERN void (*vp8_sixtap_predict8x4)(unsigned char *src_ptr, int src_pixels_per_line, int xoffset, int yoffset, unsigned char *dst_ptr, int dst_pitch);

void vp8_sixtap_predict8x8_c(unsigned char *src_ptr, int src_pixels_per_line, int xoffset, int yoffset, unsigned char *dst_ptr, int dst_pitch);
void vp8_sixtap_predict8x8_sse2(unsigned char *src_ptr, int src_pixels_per_line, int xoffset, int yoffset, unsigned char *dst_ptr, int dst_pitch);
void vp8_sixtap_predict8x8_ssse3(unsigned char *src_ptr, int src_pixels_per_line, int xoffset, int yoffset, unsigned char *dst_ptr, int dst_pitch);
RTCD_EXTERN void (*vp8_sixtap_predict8x8)(unsigned char *src_ptr, int src_pixels_per_line, int xoffset, int yoffset, unsigned char *dst_ptr, int dst_pitch);

void vp8_rtcd(void);

#ifdef RTCD_C
#include "vpx_ports/x86.h"
static void setup_rtcd_internal(void)
{
    int flags = x86_simd_caps();

    (void)flags;

    vp8_bilinear_predict16x16 = vp8_bilinear_predict16x16_sse2;
    if (flags & HAS_SSSE3) vp8_bilinear_predict16x16 = vp8_bilinear_predict16x16_ssse3;
    vp8_bilinear_predict8x8 = vp8_bilinear_predict8x8_sse2;
    if (flags & HAS_SSSE3) vp8_bilinear_predict8x8 = vp8_bilinear_predict8x8_ssse3;
    vp8_sixtap_predict16x16 = vp8_sixtap_predict16x16_sse2;
    if (flags & HAS_SSSE3) vp8_sixtap_predict16x16 = vp8_sixtap_predict16x16_ssse3;
    vp8_sixtap_predict4x4 = vp8_sixtap_predict4x4_mmx;
    if (flags & HAS_SSSE3) vp8_sixtap_predict4x4 = vp8_sixtap_predict4x4_ssse3;
    vp8_sixtap_predict8x4 = vp8_sixtap_predict8x4_sse2;
    if (flags & HAS_SSSE3) vp8_sixtap_predict8x4 = vp8_sixtap_predict8x4_ssse3;
    vp8_sixtap_predict8x8 = vp8_sixtap_predict8x8_sse2;
    if (flags & HAS_SSSE3) vp8_sixtap_predict8x8 = vp8_sixtap_predict8x8_ssse3;
}
#endif

#ifdef __cplusplus
}  // extern "C"
#endif

#endif
