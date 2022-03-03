/*
 * mdlj.cpp
 *
 *      Author: Hideo Takahashi
 */

#include <LJParams.h>
#include <MdDriver.h>
#include <Logger.h>

#include <iostream>

#include <mpi.h>

int main(int argc, char *argv[]) {
    clock_t start = clock();    // スタート時間
    try {

        int my_rank = 0;
        int num_procs = 27;
        // MPIのライブラリを初期化する。
        // MPIがargcを書き換える可能性もあるので、引数の解釈は、この関数の後でやる。
        MPI_Init(&argc, &argv);

        /* 自身のrank番号と、総プロセス数を取得する */
        MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
        MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

        /* デバッグ用のログファイルを開く。rank別のファイルが作成される。*/
        Logger::openLog("mdlj", my_rank);

        /* 計算条件オブジェクトを作成する */
        /* 引数の数が間違っていたらエラー出力して終了させるべきところ。 */
        CaseData caseData;
        // 次の行では、引数は必ず指定されているものとしてコーディングしている
        caseData.init(argv[1], my_rank, num_procs);

        // 計算条件ファイルの内容も加味してLJのパラメータや、一部のループ不変量を計算する。
        LJParams::initParams(&caseData);

        // ドライバーオブジェクトを初期化する
        MdDriver driver;
        driver.init(&caseData);

        // 時間発展ループ本体
        while (caseData.shouldProceed()) {
          if (caseData.step_count_ == 0){ //1step目の処理（転入分子、初期速度計算無し）
            driver.doInitialStep();
          }

          if(caseData.step_count_ % caseData.output_interval_ == 0 ){ //結果出力回
            driver.doStepWithOutput();
          }else{ //結果出力回でない
            driver.doStepWithoutOutput();
          }
        }

        // ドライバーに終了処理をさせる
        driver.finalize();

        // MPIの停止処理
        MPI_Finalize();

        clock_t end = clock();     // 終了時間
        if (my_rank == 0){
          std::cout << "time = " << (double)(end - start) / CLOCKS_PER_SEC << "sec.\n";
        }
        // ログをクローズする
        Logger::closeLog();
    } catch (IoException &exp) {
        std::cerr << exp << std::endl;
    } catch (DataException &exp) {
        std::cerr << exp << std::endl;
    }

    return 0;
}
