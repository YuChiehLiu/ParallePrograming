#include "PPintrin.h"

// implementation of absSerial(), but it is vectorized using PP intrinsics
void absVector(float *values, float *output, int N)
{
  __pp_vec_float x;
  __pp_vec_float result;
  __pp_vec_float zero = _pp_vset_float(0.f);
  __pp_mask maskAll, maskIsNegative, maskIsNotNegative;

  //  Note: Take a careful look at this loop indexing.  This example
  //  code is not guaranteed to work when (N % VECTOR_WIDTH) != 0.
  //  Why is that the case?
  for (int i = 0; i < N; i += VECTOR_WIDTH)
  {

    // All ones
    maskAll = _pp_init_ones();

    // All zeros
    maskIsNegative = _pp_init_ones(0);

    // Load vector of values from contiguous memory addresses
    _pp_vload_float(x, values + i, maskAll); // x = values[i];

    // Set mask according to predicate
    _pp_vlt_float(maskIsNegative, x, zero, maskAll); // if (x < 0) {

    // Execute instruction using mask ("if" clause)
    _pp_vsub_float(result, zero, x, maskIsNegative); //   output[i] = -x;

    // Inverse maskIsNegative to generate "else" mask
    maskIsNotNegative = _pp_mask_not(maskIsNegative); // } else {

    // Execute instruction ("else" clause)
    _pp_vload_float(result, values + i, maskIsNotNegative); //   output[i] = x; }

    // Write results back to memory
    _pp_vstore_float(output + i, result, maskAll);
  }
}

void clampedExpVector(float *values, int *exponents, float *output, int N)
{
  //
  // PP STUDENTS TODO: Implement your vectorized version of
  // clampedExpSerial() here.
  //
  // Your solution should work for any value of
  // N and VECTOR_WIDTH, not just when VECTOR_WIDTH divides N
  //
  __pp_vec_float a;
  __pp_vec_int b;
  __pp_vec_float result;
  __pp_vec_int int0 = _pp_vset_int(0);
  __pp_vec_int int1 = _pp_vset_int(1);
  __pp_vec_float f0 = _pp_vset_float(0);
  __pp_vec_float f1 = _pp_vset_float(1.f);
  __pp_vec_float f9 = _pp_vset_float(9.999999f);

  __pp_mask maskAll, maskIsZero, maskIsNotZero, maskCnt, maskAvoid;

  int vectornums = VECTOR_WIDTH;

  /*Preventing (N < VECTOR_WIDTH)*/
  if(N<VECTOR_WIDTH)
  {
    vectornums = N;
  }

  for (int i = 0; i < N; i += VECTOR_WIDTH)
  {
    maskAll = _pp_init_ones(vectornums); // all ones
    maskIsZero = _pp_init_ones(0); // all zeros

    _pp_vload_float(a, values + i, maskAll); // a=values[i];
    _pp_vload_int(b, exponents + i, maskAll); // b=exponents[i];

    /* if exponent[i] is 0, output[i] = 1 */
    _pp_veq_int(maskIsZero, b, int0, maskAll); // if(y==0){
    _pp_vstore_float(output + i, f1, maskIsZero); // output[i]=1.f;

    maskAvoid = _pp_mask_not(maskAll);
    maskIsZero = _pp_mask_or(maskIsZero, maskAvoid);

    /* else really do the calculating */
    maskIsNotZero = _pp_mask_not(maskIsZero); // }else{
    _pp_vload_float(result, values + i, maskIsNotZero); // result = a;
    _pp_vsub_int(b, b, int1, maskIsNotZero); // exponents--;
    maskCnt = maskIsNotZero; // maskCnt for counting exponents
    while(_pp_cntbits(maskCnt) > 0)
    {
      _pp_vgt_int(maskCnt, b, int0, maskCnt); // if(y>0){
      _pp_vmult_float(result, result, a, maskCnt); // result*=x;
      _pp_vsub_int(b, b, int1, maskCnt); // exponents--
    }

    /* following codes are for the result over 9.999999f */
    maskIsZero = _pp_init_ones(0); // initialize all zeros
    _pp_vgt_float(maskIsZero, result, f9, maskIsNotZero); // if(result >9.999999f){
    _pp_vadd_float(result, f0, f9, maskIsZero); // result = 9.999999f; }

    /* Putting the rest of results into output[i] */
    _pp_vstore_float(output + i, result, maskIsNotZero); // output[i] = result;

    /* Preventing the number of the vectors is not the multiple of VECTOR_WIDTH */
    if((N-i-VECTOR_WIDTH)<VECTOR_WIDTH)
    {
      vectornums = N-i-VECTOR_WIDTH;
    }
  }
}

// returns the sum of all elements in values
// You can assume N is a multiple of VECTOR_WIDTH
// You can assume VECTOR_WIDTH is a power of 2
float arraySumVector(float *values, int N)
{

  //
  // PP STUDENTS TODO: Implement your vectorized version of arraySumSerial here
  //

  for (int i = 0; i < N; i += VECTOR_WIDTH)
  {
  }

  return 0.0;
}