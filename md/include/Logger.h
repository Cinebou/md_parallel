#ifndef _LOGGER_H
#define _LOGGER_H

#include <iostream>
#include <fstream>


/*
 * 処理の途中経過の記録である「ログ」ファイルを出力するためのクラス。
 *
 * static メンバーしか持たないので Logger::out や Logger::openLog などのようにアクセスする。
 * インスタンスを作る必要はない。（Logger log; などとする必要はない。）
 */

class Logger {
public:

    // ログの出力ストリーム
    // ここに std::cout と同様の書き方でログを出力する。
    // 最初にopenLogを呼んでおく必要がある。
    // 使い方の例 : Logger::out << "This is a log message at line " << line << std::endl;
    static std::fstream out;

    // ログファイルを開く
    // ファイル名の基本部分とランク番号を渡す。MPI動作させない場合には0を渡す。
    // 呼び方の例 : Logger::openLog("test_Node", 0);
    //
    static void openLog(const char *progname, int rank);

    // ログファイルを閉じる
    static void closeLog();

};

#endif
