/*
 * test_Cell.cpp
 *
 *  Created on: 2014/06/28
 *      Author: hideo-t
 */

#include <TestBase.h>
#include <Cell.h>

/*
 * Tester class for Cell
 */
class TestCell : public TestBase {
    /*
     * test target
     */
    BoxXYZ box_;
    Cell cell_; // test target
    Cell neighbors_[3][3][3]; // test neighbors

public:

    void setup();
    void testMigrate();
    void run();
};

void TestCell::setup()
{
    double x0 = 100;
    double y0 = 100;
    double z0 = 100;
    double lx = 10;
    double ly = 20;
    double lz = 30;

    box_.set(x0+lx, y0+ly, z0+lz, x0+lx*2, y0+ly*2, z0+lz*2);
    cell_.setBox(box_);
    GridPeerIterator3d pit;
    while (pit.next()) {
        double x = x0 + lx*pit.ix_;
        double y = y0 + ly*pit.iy_;
        double z = z0 + lz*pit.iz_;
        Cell *nc = &neighbors_[pit.ix_][pit.iy_][pit.iz_];
        BoxXYZ box(x,y,z,x+lx,y+ly,z+lz);
        nc->setBox(box);
        cell_.setNeighborCell(pit.ix_, pit.iy_, pit.iz_, nc);
    }
}

void TestCell::testMigrate()
{
    // place a particle slightly out of the box
    Particle *p = new Particle();
    p->pos_.set(115, 130, 145); // in the middle of the box
    cell_.addParticle(p);
    p->pos_ += VectorXYZ(0,-15,0); // y became lower than the box limit
    int_equals(cell_.list_.count(), 1);
    // call migration
    cell_.migrateToNeighbor();
    test_true(cell_.list_.isEmpty());
    int_equals(neighbors_[1][0][1].list_.count(), 1);
}

void TestCell::run()
{
    setup();
    testMigrate();
}

int main(int argc, char *argv[])
{
    TestCell test;
    test.run();
    return test.report();
}
