/*
 * MpiTestBase.h
 *
 *  Created on: 2014/06/27
 *      Author: hideo-t
 */

#ifndef MPITESTBASE_H_
#define MPITESTBASE_H_

#include <TestBase.h>
#include <Logger.h>

#include <mpi.h>

/*
 * MPIの制御下で動作するテストドライバープログラムの作成を
 * 定型的に行えるようにするための親クラス。
 *
 * 本クラスのさらに親クラスTestBaseは、TestBase.hを参照。
 *
 * テストドライバープログラムを作るには、本クラスの派生クラスを作成して、
 * そのメソッドとしてテストを記述する。
 * main関数では、そのクラスのインスタンスを一つ作成して、テストメソッドを呼ぶ。
 * できたプログラムはMPIのプログラムとして起動する。
 *
 * 利用例は src-mpi/test_MdCommunicator.cpp を参照。
 */
class MpiTestBase : public TestBase {


public:
    /*
     * 当プロセスのrank
     */
    int my_rank_;

    /*
     * MPIに起動されたプロセスの総数。
     */
    int num_procs_;

    ~MpiTestBase();

    MpiTestBase();

    /*
     * MPIの初期化をする。
     */
    void initMpi(int argc, char *argv[]);
};



#endif /* MPITESTBASE_H_ */
