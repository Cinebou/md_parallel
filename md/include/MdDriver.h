/*
 * Driver.h
 *
 *      Author: Hideo Takahashi
 */

#ifndef _MDDRIVER_H
#define _MDDRIVER_H

#include <CaseData.h>
#include <MdCommData.h>
#include <MdProcData.h>
#include <MdCommunicator.h>

/*
 * ドライバークラス。
 * MDシミュレーションを構成する全てのオブジェクトを保持し、
 * 進行を制御する。
 */
class MdDriver {
    /*
     * 計算条件
     */
    CaseData *caseData_;
    /*
     * 送受信データ
     */
    MdCommData commData_;
    /*
     * １プロセス分の計算対象データ
     */
    MdProcData procData_;
    /*
     * 送受信処理実行クラス
     */
    MdCommunicator communicator_;

public:

    ~MdDriver();

    MdDriver();

    /*
     * 初期化
     */
    void init(CaseData *caseData);

    /*
     * 時間発展計算を１ステップ実行し、ファイルにデータを出力すべき回次であれば
     * ファイル出力も行う。
     */
    void doStep();
    void doStepWithOutput();
    void doStepWithoutOutput();
    void doInitialStep();

    /*
     * 所望の回数、時間発展処理を実行し終えた後で、ファイルのクローズなどの後処理を行う。
     */
    void finalize();
};

#endif /* MDDRIVER_H_ */
