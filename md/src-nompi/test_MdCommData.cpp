/*
 * test_MdCommData.cpp
 *
 *  Created on: 2014/06/28
 *      Author: hideo-t
 */

#include <TestBase.h>
#include <MdCommData.h>

/*
 * Tester class for MdCommData
 */
class TestMdCommData : public TestBase {
    /*
     * test target
     */
    CaseData caseData;
    Cell cell_;
    MdCommData commData_;

public:

    void setup();
    void testCommData();
    void testPeerBuffer();
    void run();
};

void TestMdCommData::setup()
{
    caseData.init("testdata/mdcommdata/case.txt", 5, 27);
    commData_.init(&caseData);

    cell_.setBox(BoxXYZ(0,0,0,100,100,100));

    Particle *p;

    p = new Particle;
    p->kind_ = 0;
    p->serial_ = 0;
    p->pos_.set(10,11,12);
    p->vel_dt_.set(0.1, 0.2, 0.3);
    cell_.addParticle(p);

    p = new Particle;
    p->kind_ = 0;
    p->serial_ = 1;
    p->pos_.set(20,21,22);
    p->vel_dt_.set(1.1, 1.2, 1.3);
    cell_.addParticle(p);

    p = new Particle;
    p->kind_ = 0;
    p->serial_ = 2;
    p->pos_.set(30,31,32);
    p->vel_dt_.set(2.1, 2.2, 2.3);
    cell_.addParticle(p);
}

void TestMdCommData::testCommData()
{
    commData_.addTrajectoryDataFrom(&cell_);
    int_equals(commData_.send_molecule_traj_.size(), 3);
    int_equals(commData_.send_molecule_traj_[1].kind_, 0);
    int_equals(commData_.send_molecule_traj_[1].serial_, 1);
    dbl3_equals(commData_.send_molecule_traj_[1].rx_,
            commData_.send_molecule_traj_[1].ry_,
            commData_.send_molecule_traj_[1].rz_,
            20, 21, 22);
    dbl3_equals(commData_.send_molecule_traj_[1].vx_,
            commData_.send_molecule_traj_[1].vy_,
            commData_.send_molecule_traj_[1].vz_,
            0.55, 0.6, 0.65);
}

void TestMdCommData::testPeerBuffer()
{
    MdCommPeerBuffer *buff = commData_.bufferFor(GridIndex3d(0,1,1));

    buff->clearSendMoleculeFullBuffer();
    buff->addMoleculeFullDataFrom(&cell_);
    int_equals(buff->send_molecule_full_.size(), 3);
}

void TestMdCommData::run()
{
    setup();
    testCommData();
    testPeerBuffer();
}

int main(int argc, char *argv[])
{
    TestMdCommData test;
    try {
        test.run();
    } catch (IoException &exp) {
        std::cout << exp << std::endl;
    } catch (DataException &exp) {
        std::cout << exp << std::endl;
    }
    return test.report();
}
