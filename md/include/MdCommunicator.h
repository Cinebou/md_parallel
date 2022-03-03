/*
 * Communicator.h
 *
 *      Author: Hideo Takahashi
 */

#ifndef _COMMUNICATOR_H
#define _COMMUNICATOR_H

#include <CaseData.h>
#include <MdCommData.h>

#include <mpi.h>

/*
 * 通信処理を実行するクラス
 */
class MdCommunicator {

    /*
     * MPIのライブラリにユーザ定義のデータ型を登録する時に与えられるコードを保持しておく変数
     * 本プログラムではいくつかの構造体(struct)の構造をMPIに登録して、MPIで送受信するデータ型
     * として扱う。そのためには、型をMPIに登録してコードを割り振ってもらい、送受信関数を
     * 呼び出す際に、割り振られたコードを指定する必要がある。
     */
    static MPI_Datatype MPI_MOLECULE_FULL_DATA_TYPE;
    static MPI_Datatype MPI_MOLECULE_POS_DATA_TYPE;
    static MPI_Datatype MPI_MOLECULE_TRAJECTORY_DATA_TYPE;

    /*
     * 計算条件
     */
    CaseData *caseData_;

    /*
     * 送受信データ
     */
    MdCommData *commData_;

public:
    /*
     * 初期化する
     */
    void init(CaseData *caseData, MdCommData *commData_);

    /*
     * MPIにデータを登録する。
     * initメソッドから呼ぶ。
     */
    void initMpiTypes();

    /*
     * プロセス間の分子の移転の送受信を実行する
     */
    void exchangeMoleculeFullData();

    /*
     * プロセス間の表面セルの分子の座標、原子の種類の送受信を実行する
     */
    void exchangeMoleculePosData();

    /*
     * トラジェクトリーデータをルートrankプロセスに送る
     */
    void sendTrajectroyDataToRoot();

    /*
     * ルートrankプロセスにおいて、各プロセスから送られてくる
     * トラジェクトリーデータを受信する。
     */
    void recvTrajectoryDataAtRoot();

    void calcEnergy();
};

#endif /* COMMUNICATOR_H_ */
