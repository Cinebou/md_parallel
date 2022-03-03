/*
 * test_ParticleList.cpp
 *
 *  Created on: 2014/06/28
 *      Author: hideo-t
 */

#include <TestBase.h>
#include <Particle.h>

/*
 * Tester class for ParticleList
 */
class TestParticleList : public TestBase {
    /*
     * test target
     */
    ParticleList list_;
    ParticleList free_list_;

public:

    void run();
};


void TestParticleList::run()
{
    test_null(list_.head_);
    test_null(list_.tail_);
    Particle *p = new Particle();
    list_.add(p);
    ptr_equals(list_.head_, p);
    ptr_equals(list_.tail_, p);
    Particle *q = new Particle();
    list_.add(q);
    ptr_equals(list_.tail_, q);
    ptr_equals(p->next_, q);
    ptr_equals(q->prev_, p);
}

int main(int argc, char *argv[])
{
    TestParticleList test;
    test.run();
    return test.report();
}
