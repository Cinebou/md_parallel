
#include <TestBase.h>
#include <MdCommunicator.h>
#include <GridIterator3d.h>
#include <Logger.h>

#include <mpi.h>

/*
 * Tester class for VectorXYZ
 */
class TestVectorXYZ : public TestBase {

public:

    void run();
};


void TestVectorXYZ::run()
{
    VectorXYZ p1(1.0, 2.0, 3.0);

    dbl_equals(p1.x_, 1.0);
    dbl_equals(p1.y_, 2.0);
    dbl_equals(p1.z_, 3.0);

    VectorXYZ p2(2.0, 3.0, 4.0);

    VectorXYZ p3 = p1 + p2;
    xyz_equals(p3, VectorXYZ(3.0, 5.0, 7.0));
}

int main(int argc, char *argv[])
{
    TestVectorXYZ test;
    test.run();
    return test.report();
}
