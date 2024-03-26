#ifndef __MG_TEST_MACRO_H__
#define __MG_TEST_MACRO_H__

#define TEST_FAILURE { output.success = 0; output.file = __FILE__; output.line = __LINE__; return output; }

#define PRINT_FAILURE(t1, t2, a, b, print) { fprintf(stderr, t1); print(stderr, a); fprintf(stderr, t2); print(stderr, b); }
#define PRINT_SINGLE(t, a, print) { fprintf(stderr, t); print(a); }

#define TEST_EQUAL(exp, act) if ((exp) != (act)) { TEST_FAILURE } else { output.success = 1; }
#define TEST_NOT_EQUAL(exp, act) if ((exp) == (act)) { TEST_FAILURE } else { output.success = 1; }
#define TEST_EQUAL_PRINT(exp, act, print) if ((exp) != (act)) { PRINT_FAILURE("EXPECTED:\t", "ACTUAL:\t", (exp), (act), (print)) TEST_FAILURE } else { output.success = 1; }
#define TEST_NOT_EQUAL_PRINT(exp, act, print) if ((exp) == (act)) { PRINT_FAILURE("NOT EXPECTED\t", "ACTUAL:\t", (exp), (act), (print)) TEST_FAILURE } else { output.success = 1; }

#define TEST_ZERO(exp) if (exp) { TEST_FAILURE } else { output.success = 1; }
#define TEST_NOT_ZERO(exp) if (!(exp)) { TEST_FAILURE } else { output.success = 1; }
#define TEST_ZERO_PRINT(exp, print) if (exp) { PRINT_SINGLE("EXPECTED ZERO:\t", exp, print) TEST_FAILURE } else { output.success = 1; }
#define TEST_NOT_ZERO_PRINT(exp, print) if (!(exp)) { PRINT_SINGLE("EXPECTED NOT ZERO:\t", exp, print) TEST_FAILURE } else { output.success = 1; }

#define TEST_FALSE(exp) if (exp) { TEST_FAILURE } else { output.success = 1; }
#define TEST_TRUE(exp) if (!(exp)) { TEST_FAILURE } else { output.success = 1; }

#endif
