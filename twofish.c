/* Implementation of Twofish by Tom St Denis */
#include "mycrypt.h"

#ifdef TWOFISH

const struct _cipher_descriptor twofish_desc =
{
    "twofish",
    7,
    16, 32, 16, 16,
    &twofish_setup,
    &twofish_ecb_encrypt,
    &twofish_ecb_decrypt,
    &twofish_test,
    &twofish_keysize
};

/* the two polynomials */
#define MDS_POLY          0x169
#define RS_POLY           0x14D

/* The 4x4 MDS Linear Transform */
static const unsigned char MDS[4][4] = {
    { 0x01, 0xEF, 0x5B, 0x5B },
    { 0x5B, 0xEF, 0xEF, 0x01 },
    { 0xEF, 0x5B, 0x01, 0xEF },
    { 0xEF, 0x01, 0xEF, 0x5B }
};

/* The 4x8 RS Linear Transform */
static const unsigned char RS[4][8] = {
    { 0x01, 0xA4, 0x55, 0x87, 0x5A, 0x58, 0xDB, 0x9E },
    { 0xA4, 0x56, 0x82, 0xF3, 0X1E, 0XC6, 0X68, 0XE5 },
    { 0X02, 0XA1, 0XFC, 0XC1, 0X47, 0XAE, 0X3D, 0X19 },
    { 0XA4, 0X55, 0X87, 0X5A, 0X58, 0XDB, 0X9E, 0X03 }
};

/* sbox usage orderings */
static const unsigned char qord[4][5] = {
   { 1, 1, 0, 0, 1 },
   { 0, 1, 1, 0, 0 },
   { 0, 0, 0, 1, 1 },
   { 1, 0, 1, 1, 0 }
};

#ifdef TWOFISH_TABLES
static const unsigned char SBOX[2][256] = {
{
 0xa9, 0x67, 0xb3, 0xe8, 0x04, 0xfd, 0xa3, 0x76, 0x9a, 0x92, 
 0x80, 0x78, 0xe4, 0xdd, 0xd1, 0x38, 0x0d, 0xc6, 0x35, 0x98, 
 0x18, 0xf7, 0xec, 0x6c, 0x43, 0x75, 0x37, 0x26, 0xfa, 0x13, 
 0x94, 0x48, 0xf2, 0xd0, 0x8b, 0x30, 0x84, 0x54, 0xdf, 0x23, 
 0x19, 0x5b, 0x3d, 0x59, 0xf3, 0xae, 0xa2, 0x82, 0x63, 0x01, 
 0x83, 0x2e, 0xd9, 0x51, 0x9b, 0x7c, 0xa6, 0xeb, 0xa5, 0xbe, 
 0x16, 0x0c, 0xe3, 0x61, 0xc0, 0x8c, 0x3a, 0xf5, 0x73, 0x2c, 
 0x25, 0x0b, 0xbb, 0x4e, 0x89, 0x6b, 0x53, 0x6a, 0xb4, 0xf1, 
 0xe1, 0xe6, 0xbd, 0x45, 0xe2, 0xf4, 0xb6, 0x66, 0xcc, 0x95,
 0x03, 0x56, 0xd4, 0x1c, 0x1e, 0xd7, 0xfb, 0xc3, 0x8e, 0xb5, 
 0xe9, 0xcf, 0xbf, 0xba, 0xea, 0x77, 0x39, 0xaf, 0x33, 0xc9, 
 0x62, 0x71, 0x81, 0x79, 0x09, 0xad, 0x24, 0xcd, 0xf9, 0xd8, 
 0xe5, 0xc5, 0xb9, 0x4d, 0x44, 0x08, 0x86, 0xe7, 0xa1, 0x1d, 
 0xaa, 0xed, 0x06, 0x70, 0xb2, 0xd2, 0x41, 0x7b, 0xa0, 0x11, 
 0x31, 0xc2, 0x27, 0x90, 0x20, 0xf6, 0x60, 0xff, 0x96, 0x5c, 
 0xb1, 0xab, 0x9e, 0x9c, 0x52, 0x1b, 0x5f, 0x93, 0x0a, 0xef, 
 0x91, 0x85, 0x49, 0xee, 0x2d, 0x4f, 0x8f, 0x3b, 0x47, 0x87, 
 0x6d, 0x46, 0xd6, 0x3e, 0x69, 0x64, 0x2a, 0xce, 0xcb, 0x2f, 
 0xfc, 0x97, 0x05, 0x7a, 0xac, 0x7f, 0xd5, 0x1a, 0x4b, 0x0e, 
 0xa7, 0x5a, 0x28, 0x14, 0x3f, 0x29, 0x88, 0x3c, 0x4c, 0x02, 
 0xb8, 0xda, 0xb0, 0x17, 0x55, 0x1f, 0x8a, 0x7d, 0x57, 0xc7, 
 0x8d, 0x74, 0xb7, 0xc4, 0x9f, 0x72, 0x7e, 0x15, 0x22, 0x12, 
 0x58, 0x07, 0x99, 0x34, 0x6e, 0x50, 0xde, 0x68, 0x65, 0xbc, 
 0xdb, 0xf8, 0xc8, 0xa8, 0x2b, 0x40, 0xdc, 0xfe, 0x32, 0xa4, 
 0xca, 0x10, 0x21, 0xf0, 0xd3, 0x5d, 0x0f, 0x00, 0x6f, 0x9d, 
 0x36, 0x42, 0x4a, 0x5e, 0xc1, 0xe0},
{
 0x75, 0xf3, 0xc6, 0xf4, 0xdb, 0x7b, 0xfb, 0xc8, 0x4a, 0xd3, 
 0xe6, 0x6b, 0x45, 0x7d, 0xe8, 0x4b, 0xd6, 0x32, 0xd8, 0xfd, 
 0x37, 0x71, 0xf1, 0xe1, 0x30, 0x0f, 0xf8, 0x1b, 0x87, 0xfa, 
 0x06, 0x3f, 0x5e, 0xba, 0xae, 0x5b, 0x8a, 0x00, 0xbc, 0x9d, 
 0x6d, 0xc1, 0xb1, 0x0e, 0x80, 0x5d, 0xd2, 0xd5, 0xa0, 0x84, 
 0x07, 0x14, 0xb5, 0x90, 0x2c, 0xa3, 0xb2, 0x73, 0x4c, 0x54, 
 0x92, 0x74, 0x36, 0x51, 0x38, 0xb0, 0xbd, 0x5a, 0xfc, 0x60, 
 0x62, 0x96, 0x6c, 0x42, 0xf7, 0x10, 0x7c, 0x28, 0x27, 0x8c, 
 0x13, 0x95, 0x9c, 0xc7, 0x24, 0x46, 0x3b, 0x70, 0xca, 0xe3, 
 0x85, 0xcb, 0x11, 0xd0, 0x93, 0xb8, 0xa6, 0x83, 0x20, 0xff,
 0x9f, 0x77, 0xc3, 0xcc, 0x03, 0x6f, 0x08, 0xbf, 0x40, 0xe7, 
 0x2b, 0xe2, 0x79, 0x0c, 0xaa, 0x82, 0x41, 0x3a, 0xea, 0xb9, 
 0xe4, 0x9a, 0xa4, 0x97, 0x7e, 0xda, 0x7a, 0x17, 0x66, 0x94, 
 0xa1, 0x1d, 0x3d, 0xf0, 0xde, 0xb3, 0x0b, 0x72, 0xa7, 0x1c, 
 0xef, 0xd1, 0x53, 0x3e, 0x8f, 0x33, 0x26, 0x5f, 0xec, 0x76, 
 0x2a, 0x49, 0x81, 0x88, 0xee, 0x21, 0xc4, 0x1a, 0xeb, 0xd9, 
 0xc5, 0x39, 0x99, 0xcd, 0xad, 0x31, 0x8b, 0x01, 0x18, 0x23, 
 0xdd, 0x1f, 0x4e, 0x2d, 0xf9, 0x48, 0x4f, 0xf2, 0x65, 0x8e, 
 0x78, 0x5c, 0x58, 0x19, 0x8d, 0xe5, 0x98, 0x57, 0x67, 0x7f, 
 0x05, 0x64, 0xaf, 0x63, 0xb6, 0xfe, 0xf5, 0xb7, 0x3c, 0xa5, 
 0xce, 0xe9, 0x68, 0x44, 0xe0, 0x4d, 0x43, 0x69, 0x29, 0x2e, 
 0xac, 0x15, 0x59, 0xa8, 0x0a, 0x9e, 0x6e, 0x47, 0xdf, 0x34, 
 0x35, 0x6a, 0xcf, 0xdc, 0x22, 0xc9, 0xc0, 0x9b, 0x89, 0xd4, 
 0xed, 0xab, 0x12, 0xa2, 0x0d, 0x52, 0xbb, 0x02, 0x2f, 0xa9, 
 0xd7, 0x61, 0x1e, 0xb4, 0x50, 0x04, 0xf6, 0xc2, 0x16, 0x25, 
 0x86, 0x56, 0x55, 0x09, 0xbe, 0x91}
};

static const unsigned char GF_EF[256] = {
 0x00, 0xef, 0xb7, 0x58, 0x07, 0xe8, 0xb0, 0x5f, 0x0e, 0xe1, 
 0xb9, 0x56, 0x09, 0xe6, 0xbe, 0x51, 0x1c, 0xf3, 0xab, 0x44, 
 0x1b, 0xf4, 0xac, 0x43, 0x12, 0xfd, 0xa5, 0x4a, 0x15, 0xfa, 
 0xa2, 0x4d, 0x38, 0xd7, 0x8f, 0x60, 0x3f, 0xd0, 0x88, 0x67, 
 0x36, 0xd9, 0x81, 0x6e, 0x31, 0xde, 0x86, 0x69, 0x24, 0xcb, 
 0x93, 0x7c, 0x23, 0xcc, 0x94, 0x7b, 0x2a, 0xc5, 0x9d, 0x72, 
 0x2d, 0xc2, 0x9a, 0x75, 0x70, 0x9f, 0xc7, 0x28, 0x77, 0x98, 
 0xc0, 0x2f, 0x7e, 0x91, 0xc9, 0x26, 0x79, 0x96, 0xce, 0x21,
 0x6c, 0x83, 0xdb, 0x34, 0x6b, 0x84, 0xdc, 0x33, 0x62, 0x8d, 
 0xd5, 0x3a, 0x65, 0x8a, 0xd2, 0x3d, 0x48, 0xa7, 0xff, 0x10, 
 0x4f, 0xa0, 0xf8, 0x17, 0x46, 0xa9, 0xf1, 0x1e, 0x41, 0xae, 
 0xf6, 0x19, 0x54, 0xbb, 0xe3, 0x0c, 0x53, 0xbc, 0xe4, 0x0b, 
 0x5a, 0xb5, 0xed, 0x02, 0x5d, 0xb2, 0xea, 0x05, 0xe0, 0x0f, 
 0x57, 0xb8, 0xe7, 0x08, 0x50, 0xbf, 0xee, 0x01, 0x59, 0xb6, 
 0xe9, 0x06, 0x5e, 0xb1, 0xfc, 0x13, 0x4b, 0xa4, 0xfb, 0x14, 
 0x4c, 0xa3, 0xf2, 0x1d, 0x45, 0xaa, 0xf5, 0x1a, 0x42, 0xad, 
 0xd8, 0x37, 0x6f, 0x80, 0xdf, 0x30, 0x68, 0x87, 0xd6, 0x39, 
 0x61, 0x8e, 0xd1, 0x3e, 0x66, 0x89, 0xc4, 0x2b, 0x73, 0x9c, 
 0xc3, 0x2c, 0x74, 0x9b, 0xca, 0x25, 0x7d, 0x92, 0xcd, 0x22, 
 0x7a, 0x95, 0x90, 0x7f, 0x27, 0xc8, 0x97, 0x78, 0x20, 0xcf, 
 0x9e, 0x71, 0x29, 0xc6, 0x99, 0x76, 0x2e, 0xc1, 0x8c, 0x63, 
 0x3b, 0xd4, 0x8b, 0x64, 0x3c, 0xd3, 0x82, 0x6d, 0x35, 0xda, 
 0x85, 0x6a, 0x32, 0xdd, 0xa8, 0x47, 0x1f, 0xf0, 0xaf, 0x40, 
 0x18, 0xf7, 0xa6, 0x49, 0x11, 0xfe, 0xa1, 0x4e, 0x16, 0xf9, 
 0xb4, 0x5b, 0x03, 0xec, 0xb3, 0x5c, 0x04, 0xeb, 0xba, 0x55, 
 0x0d, 0xe2, 0xbd, 0x52, 0x0a, 0xe5};

static const unsigned char GF_5B[256] = {
 0x00, 0x5b, 0xb6, 0xed, 0x05, 0x5e, 0xb3, 0xe8, 0x0a, 0x51, 
 0xbc, 0xe7, 0x0f, 0x54, 0xb9, 0xe2, 0x14, 0x4f, 0xa2, 0xf9, 
 0x11, 0x4a, 0xa7, 0xfc, 0x1e, 0x45, 0xa8, 0xf3, 0x1b, 0x40, 
 0xad, 0xf6, 0x28, 0x73, 0x9e, 0xc5, 0x2d, 0x76, 0x9b, 0xc0, 
 0x22, 0x79, 0x94, 0xcf, 0x27, 0x7c, 0x91, 0xca, 0x3c, 0x67, 
 0x8a, 0xd1, 0x39, 0x62, 0x8f, 0xd4, 0x36, 0x6d, 0x80, 0xdb, 
 0x33, 0x68, 0x85, 0xde, 0x50, 0x0b, 0xe6, 0xbd, 0x55, 0x0e, 
 0xe3, 0xb8, 0x5a, 0x01, 0xec, 0xb7, 0x5f, 0x04, 0xe9, 0xb2,
 0x44, 0x1f, 0xf2, 0xa9, 0x41, 0x1a, 0xf7, 0xac, 0x4e, 0x15, 
 0xf8, 0xa3, 0x4b, 0x10, 0xfd, 0xa6, 0x78, 0x23, 0xce, 0x95, 
 0x7d, 0x26, 0xcb, 0x90, 0x72, 0x29, 0xc4, 0x9f, 0x77, 0x2c, 
 0xc1, 0x9a, 0x6c, 0x37, 0xda, 0x81, 0x69, 0x32, 0xdf, 0x84, 
 0x66, 0x3d, 0xd0, 0x8b, 0x63, 0x38, 0xd5, 0x8e, 0xa0, 0xfb, 
 0x16, 0x4d, 0xa5, 0xfe, 0x13, 0x48, 0xaa, 0xf1, 0x1c, 0x47, 
 0xaf, 0xf4, 0x19, 0x42, 0xb4, 0xef, 0x02, 0x59, 0xb1, 0xea, 
 0x07, 0x5c, 0xbe, 0xe5, 0x08, 0x53, 0xbb, 0xe0, 0x0d, 0x56, 
 0x88, 0xd3, 0x3e, 0x65, 0x8d, 0xd6, 0x3b, 0x60, 0x82, 0xd9, 
 0x34, 0x6f, 0x87, 0xdc, 0x31, 0x6a, 0x9c, 0xc7, 0x2a, 0x71, 
 0x99, 0xc2, 0x2f, 0x74, 0x96, 0xcd, 0x20, 0x7b, 0x93, 0xc8, 
 0x25, 0x7e, 0xf0, 0xab, 0x46, 0x1d, 0xf5, 0xae, 0x43, 0x18, 
 0xfa, 0xa1, 0x4c, 0x17, 0xff, 0xa4, 0x49, 0x12, 0xe4, 0xbf, 
 0x52, 0x09, 0xe1, 0xba, 0x57, 0x0c, 0xee, 0xb5, 0x58, 0x03, 
 0xeb, 0xb0, 0x5d, 0x06, 0xd8, 0x83, 0x6e, 0x35, 0xdd, 0x86, 
 0x6b, 0x30, 0xd2, 0x89, 0x64, 0x3f, 0xd7, 0x8c, 0x61, 0x3a, 
 0xcc, 0x97, 0x7a, 0x21, 0xc9, 0x92, 0x7f, 0x24, 0xc6, 0x9d, 
 0x70, 0x2b, 0xc3, 0x98, 0x75, 0x2e};

#define sbox(i, x) ((unsigned long)SBOX[i][(x)&255])

#else

/* The Q-box tables */
static const unsigned char qbox[2][4][16] = { 
{
   { 0x8, 0x1, 0x7, 0xD, 0x6, 0xF, 0x3, 0x2, 0x0, 0xB, 0x5, 0x9, 0xE, 0xC, 0xA, 0x4 },
   { 0xE, 0XC, 0XB, 0X8, 0X1, 0X2, 0X3, 0X5, 0XF, 0X4, 0XA, 0X6, 0X7, 0X0, 0X9, 0XD },
   { 0XB, 0XA, 0X5, 0XE, 0X6, 0XD, 0X9, 0X0, 0XC, 0X8, 0XF, 0X3, 0X2, 0X4, 0X7, 0X1 },
   { 0XD, 0X7, 0XF, 0X4, 0X1, 0X2, 0X6, 0XE, 0X9, 0XB, 0X3, 0X0, 0X8, 0X5, 0XC, 0XA }
}, 
{
   { 0X2, 0X8, 0XB, 0XD, 0XF, 0X7, 0X6, 0XE, 0X3, 0X1, 0X9, 0X4, 0X0, 0XA, 0XC, 0X5 },
   { 0X1, 0XE, 0X2, 0XB, 0X4, 0XC, 0X3, 0X7, 0X6, 0XD, 0XA, 0X5, 0XF, 0X9, 0X0, 0X8 },
   { 0X4, 0XC, 0X7, 0X5, 0X1, 0X6, 0X9, 0XA, 0X0, 0XE, 0XD, 0X8, 0X2, 0XB, 0X3, 0XF },
   { 0xB, 0X9, 0X5, 0X1, 0XC, 0X3, 0XD, 0XE, 0X6, 0X4, 0X7, 0XF, 0X2, 0X0, 0X8, 0XA }
}
};

/* computes S_i[x] */
#ifdef CLEAN_STACK
static unsigned long _sbox(int i, unsigned long x)
#else
unsigned long sbox(int i, unsigned long x)
#endif
{
   unsigned char a0,b0,a1,b1,a2,b2,a3,b3,a4,b4,y;

   /* a0,b0 = [x/16], x mod 16 */
   a0 = (x>>4)&15;
   b0 = (x)&15;

   /* a1 = a0 ^ b0 */
   a1 = a0 ^ b0;

   /* b1 = a0 ^ ROR(b0, 1) ^ 8a0 */
   b1 = (a0 ^ ((b0<<3)|(b0>>1)) ^ (a0<<3)) & 15;

   /* a2,b2 = t0[a1], t1[b1] */
   a2 = qbox[i][0][a1];
   b2 = qbox[i][1][b1];

   /* a3 = a2 ^ b2 */
   a3 = a2 ^ b2;

   /* b3 = a2 ^ ROR(b2, 1) ^ 8a2 */
   b3 = (a2 ^ ((b2<<3)|(b2>>1)) ^ (a2<<3)) & 15;

   /* a4,b4 = t2[a3], t3[b3] */
   a4 = qbox[i][2][a3];
   b4 = qbox[i][3][b3];

   /* y = 16b4 + a4 */
   y = (b4 << 4) + a4;

   /* return result */
   return (unsigned long)y;
}

#ifdef CLEAN_STACK
static unsigned long sbox(int i, unsigned long x)
{
   unsigned long y;
   y = _sbox(i, x);
   burn_stack(sizeof(unsigned char) * 11);
   return y;
}
#endif /* CLEAN_STACK */

#endif /* TWOFISH_TABLES */

/* computes ab mod p */
static unsigned long gf_mult(unsigned long a, unsigned long b, unsigned long p)
{
   unsigned long result = 0;
   while (a) {
      if (a&1)
         result ^= b;
      a >>= 1;
      b <<= 1;
      if (b & 0x100)
         b ^= p;
   }
   return result & 255;
}


/* Computes [y0 y1 y2 y3] = MDS . [x0 x1 x2 x3] */
static void mds_mult(const unsigned char *in, unsigned char *out)
{
  int x, y;
  unsigned char tmp[4];

  for (x = 0; x < 4; x++) {
      tmp[x] = 0;
      for (y = 0; y < 4; y++)
          tmp[x] ^= gf_mult(in[y], MDS[x][y], MDS_POLY);
  }
  for (x = 0; x < 4; x++)
      out[x] = tmp[x];
  zeromem(tmp, 4);
}

/* computes [y0 y1 y2 y3] = RS . [x0 x1 x2 x3 x4 x5 x6 x7] */
static void rs_mult(const unsigned char *in, unsigned char *out)
{
  int x, y;
  unsigned char tmp[4];

  for (x = 0; x < 4; x++) {
      tmp[x] = 0;
      for (y = 0; y < 8; y++)
          tmp[x] ^= gf_mult(in[y], RS[x][y], RS_POLY);
  }
  for (x = 0; x < 4; x++)
      out[x] = tmp[x];
  zeromem(tmp, 4);
}

/* computes [y0 y1 y2 y3] = MDS . [x0] */
#ifndef TWOFISH_TABLES
static unsigned long mds_column_mult(unsigned char in, int col)
{
   return
       (gf_mult(in, MDS[0][col], MDS_POLY) << 0) |
       (gf_mult(in, MDS[1][col], MDS_POLY) << 8) |
       (gf_mult(in, MDS[2][col], MDS_POLY) << 16) |
       (gf_mult(in, MDS[3][col], MDS_POLY) << 24);
}
#else
static unsigned long mds_column_mult(unsigned char in, int col)
{
   unsigned long x01, x5B, xEF;

   x01 = in;
   x5B = GF_5B[in];
   xEF = GF_EF[in];

   switch (col) {
       case 0:
          return (x01 << 0 ) |
                 (x5B << 8 ) |
                 (xEF << 16) |
                 (xEF << 24);
       case 1:
          return (xEF << 0 ) |
                 (xEF << 8 ) |
                 (x5B << 16) |
                 (x01 << 24);
       case 2:
          return (x5B << 0 ) |
                 (xEF << 8 ) |
                 (x01 << 16) |
                 (xEF << 24);
       case 3:
          return (x5B << 0 ) |
                 (x01 << 8 ) |
                 (xEF << 16) |
                 (x5B << 24);
   }
   /* avoid warnings, we'd never get here normally but just to calm compiler warnings... */
   return 0;
}
#endif

/* computes h(x) */
static void h_func(const unsigned char *in, unsigned char *out, unsigned char *M, int k, int offset)
{
  int x;
  unsigned char y[4];

  for (x = 0; x < 4; x++)
      y[x] = in[x];

  switch (k) {
     case 4:
            y[0] = sbox(1, y[0]) ^ M[4 * (6 + offset) + 0];
            y[1] = sbox(0, y[1]) ^ M[4 * (6 + offset) + 1];
            y[2] = sbox(0, y[2]) ^ M[4 * (6 + offset) + 2];
            y[3] = sbox(1, y[3]) ^ M[4 * (6 + offset) + 3];
     case 3:
            y[0] = sbox(1, y[0]) ^ M[4 * (4 + offset) + 0];
            y[1] = sbox(1, y[1]) ^ M[4 * (4 + offset) + 1];
            y[2] = sbox(0, y[2]) ^ M[4 * (4 + offset) + 2];
            y[3] = sbox(0, y[3]) ^ M[4 * (4 + offset) + 3];
     case 2:
            y[0] = sbox(1, sbox(0, sbox(0, y[0]) ^ M[4 * (2 + offset) + 0]) ^ M[4 * (0 + offset) + 0]);
            y[1] = sbox(0, sbox(0, sbox(1, y[1]) ^ M[4 * (2 + offset) + 1]) ^ M[4 * (0 + offset) + 1]);
            y[2] = sbox(1, sbox(1, sbox(0, y[2]) ^ M[4 * (2 + offset) + 2]) ^ M[4 * (0 + offset) + 2]);
            y[3] = sbox(0, sbox(1, sbox(1, y[3]) ^ M[4 * (2 + offset) + 3]) ^ M[4 * (0 + offset) + 3]);
  }
  mds_mult(y, out);
}

#ifndef TWOFISH_SMALL

static unsigned long g_func(unsigned long x, symmetric_key *key)
{
   return
       key->twofish.S[0][(x>>0)&255] ^
       key->twofish.S[1][(x>>8)&255] ^
       key->twofish.S[2][(x>>16)&255] ^
       key->twofish.S[3][(x>>24)&255];
}

#else

#ifdef CLEAN_STACK
static unsigned long _g_func(unsigned long x, symmetric_key *key)
#else
unsigned long g_func(unsigned long x, symmetric_key *key)
#endif
{
   unsigned char g, i, y, z;
   unsigned long res;

   res = 0;
   for (y = 0; y < 4; y++) {
       z = key->twofish.start;

       /* do unkeyed substitution */
       g = sbox(qord[y][z++], (x >> (8*y)) & 255);

       /* first subkey */
       i = 0;

       /* do key mixing+sbox until z==5 */
       while (z != 5) {
          g = g ^ key->twofish.S[4*i++ + y];
          g = sbox(qord[y][z++], g);
       }

       /* multiply g by a column of the MDS */
       res ^= mds_column_mult(g, y);
   }
   return res;
}

#ifdef CLEAN_STACK
static unsigned long g_func(unsigned long x, symmetric_key *key)
{
    unsigned long y;
    y = _g_func(x, key);
    burn_stack(sizeof(unsigned char) * 4 + sizeof(unsigned long));
    return y;
}
#endif

#endif

#ifdef CLEAN_STACK
static int _twofish_setup(const unsigned char *key, int keylen, int num_rounds, symmetric_key *skey)
#else
int twofish_setup(const unsigned char *key, int keylen, int num_rounds, symmetric_key *skey)
#endif
{
#ifndef TWOFISH_SMALL
   int g, z, i;
   unsigned char S[4*4];
#endif
   int k, x, y, start;
   unsigned char tmp[4], tmp2[4], M[8*4];
   unsigned long A, B;

   _ARGCHK(key != NULL);
   _ARGCHK(skey != NULL);

   /* invalid arguments? */
   if (num_rounds != 16 && num_rounds != 0) {
      return CRYPT_INVALID_ROUNDS;
   }

   if (keylen != 16 && keylen != 24 && keylen != 32) {
      return CRYPT_INVALID_KEYSIZE;
   }

   /* k = keysize/64 [but since our keysize is in bytes...] */
   k = keylen / 8;

   /* copy the key into M */
   for (x = 0; x < keylen; x++)
       M[x] = key[x];

   /* create the S[..] words */
#ifndef TWOFISH_SMALL
   for (x = 0; x < k; x++)
       rs_mult(M+(x*8), S+(x*4));
#else
   for (x = 0; x < k; x++)
       rs_mult(M+(x*8), skey->twofish.S+(x*4));
#endif

   /* make subkeys */
   for (x = 0; x < 20; x++) {
       /* A = h(p * 2x, Me) */
       for (y = 0; y < 4; y++)
           tmp[y] = x+x;
       h_func(tmp, tmp2, M, k, 0);
       LOAD32L(A, tmp2);

       /* B = ROL(h(p * (2x + 1), Mo), 8) */
       for (y = 0; y < 4; y++)
           tmp[y] = x+x+1;
       h_func(tmp, tmp2, M, k, 1);
       LOAD32L(B, tmp2);
       B = ROL(B, 8);

       /* K[2i]   = A + B */
       skey->twofish.K[x+x] = (A + B) & 0xFFFFFFFFUL;

       /* K[2i+1] = (A + 2B) <<< 9 */
       skey->twofish.K[x+x+1] = ROL(B + B + A, 9);
   }

   /* where to start in the sbox layers */
   switch (k) {
         case 4 : start = 0; break;
         case 3 : start = 1; break; 
         default: start = 2; break;
   }

#ifndef TWOFISH_SMALL
   /* make the sboxes (large ram variant) */
   for (y = 0; y < 4; y++) {
       for (x = 0; x < 256; x++) {
           z = start;

           /* do unkeyed substitution */
           g = sbox(qord[y][z++], x);

           /* first subkey */
           i = 0;

           /* do key mixing+sbox until z==5 */
           while (z != 5) {
               g = g ^ S[4*i++ + y];
               g = sbox(qord[y][z++], g);
           }
           
           /* multiply g by a column of the MDS */
           skey->twofish.S[y][x] = mds_column_mult(g, y);
       }
   }
#else
   /* small ram variant */
   skey->twofish.start = start;
#endif
   return CRYPT_OK;
}

#ifdef CLEAN_STACK
int twofish_setup(const unsigned char *key, int keylen, int num_rounds, symmetric_key *skey)
{
   int x;
   x = _twofish_setup(key, keylen, num_rounds, skey);
   burn_stack(sizeof(int) * 7 + sizeof(unsigned char) * 56 + sizeof(unsigned long) * 2);
   return x;
}
#endif

#ifdef CLEAN_STACK
static void _twofish_ecb_encrypt(const unsigned char *pt, unsigned char *ct, symmetric_key *key)
#else
void twofish_ecb_encrypt(const unsigned char *pt, unsigned char *ct, symmetric_key *key)
#endif
{
    unsigned long a,b,c,d,ta,tb,tc,td,t1,t2;
    int r;

    _ARGCHK(pt != NULL);
    _ARGCHK(ct != NULL);
    _ARGCHK(key != NULL);

    LOAD32L(a,&pt[0]); LOAD32L(b,&pt[4]);
    LOAD32L(c,&pt[8]); LOAD32L(d,&pt[12]);
    a ^= key->twofish.K[0];
    b ^= key->twofish.K[1];
    c ^= key->twofish.K[2];
    d ^= key->twofish.K[3];

    for (r = 0; r < 16; r += 2) {
        t1 = g_func(a, key);
        t2 = g_func(ROL(b, 8), key);
        t2 += (t1 += t2);
        t1 += key->twofish.K[r+r+8];
        t2 += key->twofish.K[r+r+9];
        c  ^= t1; c = ROR(c, 1);
        d  = ROL(d, 1) ^ t2;

        t1 = g_func(c, key);
        t2 = g_func(ROL(d, 8), key);
        t2 += (t1 += t2);
        t1 += key->twofish.K[r+r+10];
        t2 += key->twofish.K[r+r+11];
        a ^= t1; a = ROR(a, 1);
        b  = ROL(b, 1) ^ t2;
    }

    /* output with "undo last swap" */
    ta = c ^ key->twofish.K[4];
    tb = d ^ key->twofish.K[5];
    tc = a ^ key->twofish.K[6];
    td = b ^ key->twofish.K[7];

    /* store output */
    STORE32L(ta,&ct[0]); STORE32L(tb,&ct[4]);
    STORE32L(tc,&ct[8]); STORE32L(td,&ct[12]);
}

#ifdef CLEAN_STACK
void twofish_ecb_encrypt(const unsigned char *pt, unsigned char *ct, symmetric_key *key)
{
   _twofish_ecb_encrypt(pt, ct, key);
   burn_stack(sizeof(unsigned long) * 10 + sizeof(int));
}
#endif

#ifdef CLEAN_STACK
static void _twofish_ecb_decrypt(const unsigned char *ct, unsigned char *pt, symmetric_key *key)
#else
void twofish_ecb_decrypt(const unsigned char *ct, unsigned char *pt, symmetric_key *key)
#endif
{
    unsigned long a,b,c,d,ta,tb,tc,td,t1,t2;
    int r;

    _ARGCHK(pt != NULL);
    _ARGCHK(ct != NULL);
    _ARGCHK(key != NULL);

    /* load input */
    LOAD32L(ta,&ct[0]); LOAD32L(tb,&ct[4]);
    LOAD32L(tc,&ct[8]); LOAD32L(td,&ct[12]);

    /* undo undo final swap */
    a = tc ^ key->twofish.K[6];
    b = td ^ key->twofish.K[7];
    c = ta ^ key->twofish.K[4];
    d = tb ^ key->twofish.K[5];

    for (r = 14; r >= 0; r -= 2) {
        t1 = g_func(c, key);
        t2 = g_func(ROL(d, 8), key);
        t2 += (t1 += t2);
        t1 += key->twofish.K[r+r+10];
        t2 += key->twofish.K[r+r+11];
        a  = ROL(a, 1) ^ t1;
        b  = b ^ t2; b = ROR(b, 1);

        t1 = g_func(a, key);
        t2 = g_func(ROL(b, 8), key);
        t2 += (t1 += t2);
        t1 += key->twofish.K[r+r+8];
        t2 += key->twofish.K[r+r+9];
        c  = ROL(c, 1) ^ t1;
        d  = d ^ t2; d = ROR(d, 1);
    }

    /* pre-white */
    a ^= key->twofish.K[0];
    b ^= key->twofish.K[1];
    c ^= key->twofish.K[2];
    d ^= key->twofish.K[3];
    
    /* store */
    STORE32L(a, &pt[0]); STORE32L(b, &pt[4]);
    STORE32L(c, &pt[8]); STORE32L(d, &pt[12]);
}

#ifdef CLEAN_STACK
void twofish_ecb_decrypt(const unsigned char *ct, unsigned char *pt, symmetric_key *key)
{
   _twofish_ecb_decrypt(ct, pt, key);
   burn_stack(sizeof(unsigned long) * 10 + sizeof(int));
}
#endif

int twofish_test(void)
{
 static const struct { 
     int keylen;
     unsigned char key[32], pt[16], ct[16];
 } tests[] = {
   { 16,
     { 0x9F, 0x58, 0x9F, 0x5C, 0xF6, 0x12, 0x2C, 0x32,
       0xB6, 0xBF, 0xEC, 0x2F, 0x2A, 0xE8, 0xC3, 0x5A },
     { 0xD4, 0x91, 0xDB, 0x16, 0xE7, 0xB1, 0xC3, 0x9E,
       0x86, 0xCB, 0x08, 0x6B, 0x78, 0x9F, 0x54, 0x19 },
     { 0x01, 0x9F, 0x98, 0x09, 0xDE, 0x17, 0x11, 0x85,
       0x8F, 0xAA, 0xC3, 0xA3, 0xBA, 0x20, 0xFB, 0xC3 }
   }, {
     24,
     { 0x88, 0xB2, 0xB2, 0x70, 0x6B, 0x10, 0x5E, 0x36,
       0xB4, 0x46, 0xBB, 0x6D, 0x73, 0x1A, 0x1E, 0x88,
       0xEF, 0xA7, 0x1F, 0x78, 0x89, 0x65, 0xBD, 0x44 },
     { 0x39, 0xDA, 0x69, 0xD6, 0xBA, 0x49, 0x97, 0xD5,
       0x85, 0xB6, 0xDC, 0x07, 0x3C, 0xA3, 0x41, 0xB2 },
     { 0x18, 0x2B, 0x02, 0xD8, 0x14, 0x97, 0xEA, 0x45,
       0xF9, 0xDA, 0xAC, 0xDC, 0x29, 0x19, 0x3A, 0x65 }
   }, { 
     32,
     { 0xD4, 0x3B, 0xB7, 0x55, 0x6E, 0xA3, 0x2E, 0x46,
       0xF2, 0xA2, 0x82, 0xB7, 0xD4, 0x5B, 0x4E, 0x0D,
       0x57, 0xFF, 0x73, 0x9D, 0x4D, 0xC9, 0x2C, 0x1B,
       0xD7, 0xFC, 0x01, 0x70, 0x0C, 0xC8, 0x21, 0x6F },
     { 0x90, 0xAF, 0xE9, 0x1B, 0xB2, 0x88, 0x54, 0x4F,
       0x2C, 0x32, 0xDC, 0x23, 0x9B, 0x26, 0x35, 0xE6 },
     { 0x6C, 0xB4, 0x56, 0x1C, 0x40, 0xBF, 0x0A, 0x97,
       0x05, 0x93, 0x1C, 0xB6, 0xD4, 0x08, 0xE7, 0xFA }
   }
};


 symmetric_key key;
 unsigned char tmp[2][16];
 int errno, i;
 
 for (i = 0; i < (int)(sizeof(tests)/sizeof(tests[0])); i++) {
    if ((errno = twofish_setup(tests[i].key, tests[i].keylen, 0, &key)) != CRYPT_OK) {
       return errno;
    }
    twofish_ecb_encrypt(tests[i].pt, tmp[0], &key);
    twofish_ecb_decrypt(tmp[0], tmp[1], &key);
    if (memcmp(tmp[0], tests[i].ct, 16) || memcmp(tmp[1], tests[i].pt, 16)) {
       return CRYPT_FAIL_TESTVECTOR;
    }
 }    
 return CRYPT_OK;
}

int twofish_keysize(int *desired_keysize)
{
   _ARGCHK(desired_keysize);
   if (*desired_keysize < 16)
      return CRYPT_INVALID_KEYSIZE;
   if (*desired_keysize < 24) {
      *desired_keysize = 16;
      return CRYPT_OK;
   } else if (*desired_keysize < 32) {
      *desired_keysize = 24;
      return CRYPT_OK;
   } else {
      *desired_keysize = 32;
      return CRYPT_OK;
   }
}

#endif



