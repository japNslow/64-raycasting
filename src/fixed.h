#ifndef FIXED_H
#define FIXED_H

/* 8.8 Fixed-Point Types */
typedef signed int   fixed;   /* 16-bit signed: 8 bits int, 8 bits frac */
typedef unsigned int ufixed;  /* 16-bit unsigned */

#define FP_SHIFT        8
#define FP_ONE          (1 << FP_SHIFT)
#define FP_HALF         (1 << (FP_SHIFT - 1))

/* Conversion macros */
#define INT_TO_FIXED(x) ((fixed)((x) << FP_SHIFT))
#define FIXED_TO_INT(x) ((int)((x) >> FP_SHIFT))
#define FRAC_PART(x)    ((unsigned char)((x) & 0xFF))

#endif /* FIXED_H */
