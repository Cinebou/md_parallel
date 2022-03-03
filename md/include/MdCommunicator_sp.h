/*
 * MdCommunicator_sp.h
 *
 *      Author: Hideo Takahashi
 */

#ifndef _MD_COMMUNICATOR_SP_H
#define _MD_COMMUNICATOR_SP_H

#include <CaseData.h>
#include <MdCommData.h>

/*
 * 通信処理を実行するクラス
 */
class MdCommunicator_sp {

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
     * プロセス間の分子の移転の送受信を実行する
     */
    void exchangeMoleculeFullData();

    /*
     * ルートrankプロセスにおいて、各プロセスから送られてくる
     * トラジェクトリーデータを受信する。
     */
    void recvTrajectoryDataAtRoot();
};

#endif /* COMMUNICATOR_H_ */
