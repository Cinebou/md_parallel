#include <Logger.h>
#include <sstream>

std::fstream Logger::out;

void Logger::openLog(const char *progname, int rank) {
    // プログラム名とランク番号からファイル名文字列を組み立てる。
    std::stringstream ss;
    ss << progname << ".log." << rank << ".txt";
    // できた名称でファイルを書き込み用に開く
    out.open(ss.str().c_str(), std::ios::out);
}

void Logger::closeLog() {
    out.close();
}
