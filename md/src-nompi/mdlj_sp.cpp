/*
 * mdlj_sp.cpp
 *
 * シングルプロセス版のメインルーチン
 *
 * MPIなしで動作させる。力計算や積分のデバッグ用や、並列化効果の性能比較用に利用できる。
 *
 *      Author: Hideo Takahashi
 */

#include <LJParams.h>
#include <MdDriver_sp.h>
#include <Logger.h>

#include <iostream>

int main(int argc, char *argv[]) {

    try {

        int my_rank = 0;
        int num_procs = 1;
        // MPIライブラリは呼ばない

        /* デバッグ用のログファイルを開く。rank別のファイルが作成される。*/
        Logger::openLog("mdlj_sp", my_rank);

        /* 計算条件オブジェクトを作成する */
        /* 引数の数が間違っていたらエラー出力して終了させるべきところ。 */
        CaseData caseData;
        // 次の行では、引数は必ず指定されているものとしてコーディングしている
        caseData.init(argv[1], my_rank, num_procs);

        // 計算条件ファイルの内容も加味してLJのパラメータや、一部のループ不変量を計算する。
        LJParams::initParams(&caseData);

        // ドライバーオブジェクトを初期化する。
        MdDriver_sp driver_sp; // シングルプロセス版ドライバを使う
        driver_sp.init(&caseData);

        // 時間発展ループ本体
        while (caseData.shouldProceed()) {
          //std::cout << "step" << caseData.step_count_ << std::endl;
            driver_sp.doStep();
        }

        // ドライバーに終了処理をさせる
        driver_sp.finalize();

        // ログをクローズする
        Logger::closeLog();
    } catch (IoException &exp) {
        std::cerr << exp << std::endl;
    } catch (DataException &exp) {
        std::cerr << exp << std::endl;
    }

    return 0;
}
