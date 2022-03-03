/*
 * TestBase.h
 *
 *  Created on: 2014/06/27
 *      Author: hideo-t
 */

#ifndef TESTBASE_H_
#define TESTBASE_H_

#include <iostream>

#include <VectorXYZ.h>
#include <GridIterator3d.h>

/*
 * macro definitions to be used by test programs
 */

#define test_true(EXPRESSION) \
        testTrueFunc(EXPRESSION, #EXPRESSION, __FILE__, __LINE__)

#define test_false(EXPRESSION) \
        testFalseFunc(EXPRESSION, #EXPRESSION, __FILE__, __LINE__)

#define test_not_null(EXPRESSION) \
        testNotNullFunc(EXPRESSION, #EXPRESSION, __FILE__, __LINE__)

#define test_null(EXPRESSION) \
        testNullFunc(EXPRESSION, #EXPRESSION, __FILE__, __LINE__)

#define dbl_equals(EXPRESSION, EXPECTED_VALUE) \
        dblEqualsFunc(EXPRESSION, #EXPRESSION, EXPECTED_VALUE, __FILE__, __LINE__)

#define dbl3_equals(V1, V2, V3, E1, E2, E3) \
        dbl3EqualsFunc(V1, V2, V3, #V1, #V2, #V3, E1, E2, E3, __FILE__, __LINE__)

#define int_equals(EXPRESSION, EXPECTED_VALUE) \
        intEqualsFunc(EXPRESSION, #EXPRESSION, EXPECTED_VALUE, __FILE__, __LINE__)

#define int3_equals(V1, V2, V3, E1, E2, E3) \
        int3EqualsFunc(V1, V2, V3, #V1, #V2, #V3, E1, E2, E3, __FILE__, __LINE__)

#define size_equals(EXPRESSION, EXPECTED_VALUE) \
        sizeEqualsFunc(EXPRESSION, #EXPRESSION, EXPECTED_VALUE, __FILE__, __LINE__)

#define ptr_equals(EXPRESSION, EXPECTED_VALUE) \
        ptrEqualsFunc(EXPRESSION, #EXPRESSION, EXPECTED_VALUE, __FILE__, __LINE__)

#define xyz_equals(EXPRESSION, EXPECTED_VALUE) \
        xyzEqualsFunc(EXPRESSION, #EXPRESSION, EXPECTED_VALUE, __FILE__, __LINE__)

#define i3d_equals(EXPRESSION, EXPECTED_VALUE) \
        i3dEqualsFunc(EXPRESSION, #EXPRESSION, EXPECTED_VALUE, __FILE__, __LINE__)
/*
 * テストドライバープログラムの作成を
 * 定型的に行えるようにするための親クラス。
 *
 * テストドライバープログラムを作るには、本クラスの派生クラスを作成して、
 * そのメソッドとしてテストを記述する。
 * main関数では、そのクラスのインスタンスを一つ作成して、テストメソッドを呼ぶ。
 *
 * 利用例は src-nompi/test_CaseData.cpp を参照。
 */
class TestBase {

    std::ostream *pout_;

    int test_count_;
    int test_passed_count_;
    int test_failed_count_;
    double tolerance_;

public:

    ~TestBase();

    TestBase();

    void setOut(std::ostream *pout);

    void setTolerance(double tolerance);

    /*
     * Write a report.
     */
    int report();

    /*
     * Functions called from the above macros.
     * Do not call these functions directly.
     */
    void testTrueFunc(bool expression, const char *expression_source,
            const char *file, int line);

    void testFalseFunc(bool expression, const char *expression_source,
            const char *file, int line);

    void testNotNullFunc(void *expression, const char *expression_source,
            const char *file, int line);

    void testNullFunc(void *expression, const char *expression_source,
            const char *file, int line);

    /* double version */
    void dblEqualsFunc(double calculated_value, const char *calculated_source,
            double expected_value, const char *file, int line);

    /* double triplet version */
    void dbl3EqualsFunc(double v1, double v2, double v3,
            const char *s1, const char *s2, const char *s3,
            double e1, double e2, double e3, const char *file, int line);

    /* int version */
    void intEqualsFunc(int calculated_value, const char *calculated_source,
            int expected_value, const char *file, int line);

    /* int triplet version */
    void int3EqualsFunc(int v1, int v2, int v3,
            const char *s1, const char *s2, const char *s3,
            int e1, int e2, int e3, const char *file, int line);

    /* size_t version */
    void sizeEqualsFunc(size_t calculated_value, const char *calculated_source,
            size_t expected_value, const char *file, int line);

    /* pointer version */
    void ptrEqualsFunc(void *calculated_value, const char *calculated_source,
            void *expected_value, const char *file, int line);

    /* VectorXYZ version. be careful of mutual(cyclic) dependecy. */
    void xyzEqualsFunc(const VectorXYZ &calculated_value, const char *calculated_source,
            const VectorXYZ &expected_value, const char *file, int line);

    /* GridIndex3d version. be careful of mutual(cyclic) dependecy. */
    void i3dEqualsFunc(const GridIndex3d &calculated_value, const char *calculated_source,
            const GridIndex3d &expected_value, const char *file, int line);

};

#endif /* TESTBASE_H_ */
