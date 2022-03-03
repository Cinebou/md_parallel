/*
 * test_GridIterator3d.cpp
 *
 *  Created on: 2014/06/28
 *      Author: hideo-t
 */

#include <TestBase.h>
#include <GridIterator3d.h>

/*
 * Tester class for GridIterator3d
 */
class TestGridIterator3d : public TestBase {
    /*
     * test target
     */
    GridIterator3d p1_, p2_;

public:

    void testDirIterator3d();
    void testPeerIterator3d();
    void testIterator3d();
    void run();
};


void TestGridIterator3d::testDirIterator3d()
{
    GridDirIterator3d it;
    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
            for (int k = -1; k <= 1; k++) {
                if (i == 0 && j == 0 && k == 0) {
                    continue;
                }
                test_true(it.next());
                int_equals(it.ix_, i);
                int_equals(it.iy_, j);
                int_equals(it.iz_, k);
            }
        }
    }
    test_false(it.next());
}

void TestGridIterator3d::testPeerIterator3d()
{
    GridPeerIterator3d it;
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            for (int k = 0; k < 3; k++) {
                if (i == 1 && j == 1 && k == 1) {
                    continue;
                }
                test_true(it.next());
                int_equals(it.ix_, i);
                int_equals(it.iy_, j);
                int_equals(it.iz_, k);
            }
        }
    }
    test_false(it.next());
}

void TestGridIterator3d::testIterator3d()
{
    GridRange3d range;
    range.setRange(0,1,2,3,4,5);
    GridIterator3d it(range);
    for (int i = 0; i <= 3; i++) {
        for (int j = 1; j <= 4; j++) {
            for (int k = 2; k <= 5; k++) {
                test_true(it.next());
                int_equals(it.ix_, i);
                int_equals(it.iy_, j);
                int_equals(it.iz_, k);
            }
        }
    }
    test_false(it.next());
}

void TestGridIterator3d::run()
{
    testDirIterator3d();
    testPeerIterator3d();
    testIterator3d();
}

int main(int argc, char *argv[])
{
    TestGridIterator3d test;
    test.run();
    return test.report();
}
