/*
 * MpiTestBase.cpp
 *
 *  Created on: 2014/06/27
 *      Author: hideo-t
 */

#include <MpiTestBase.h>

MpiTestBase::~MpiTestBase()
{
    MPI_Finalize();
    Logger::closeLog();
}

MpiTestBase::MpiTestBase()
{

}

void MpiTestBase::initMpi(int argc, char *argv[])
{
    // MPIを初期化する
    MPI_Init(&argc, &argv);

    // 自身のrankを取得する
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank_);

    // 総プロセス数を取得する
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs_);

    // 自身の実行形式ファイル名から、ディレクトリ名を除いた部分を path に格納する
    std::string path = argv[0];
    size_t pos = path.find_last_of('/');
    if (pos != std::string::npos) {
        path = path.substr(pos+1);
    }

    // ログファイルを開く。自身のファイル名とランク番号からなるファイル名とする。
    Logger::openLog(path.c_str(), my_rank_);
    setOut(&Logger::out);

}
