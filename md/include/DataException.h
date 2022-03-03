#ifndef _DATA_EXCEPTION_H
#define _DATA_EXCEPTION_H

#include <iostream>
#include <sstream>

/*
 * 入力ファイルのデータに問題があった場合に挙げる例外クラス
 *
 * 投げる箇所の使用例:
 * std::string msg = "エラーの内容を説明する文字列";
 * throw DataException(__FILE__, __LINE__, msg);
 *
 * 拾う箇所の使用例：
 *
 * try {
 *    some_function();
 * } catch (DataException &exp) {
 *    std::cerr << exp << std::endl;
 * }
 *
 */
class DataException {
public:

    // throw が書いてあるソースファイル内の位置
    const char *source_file_;
    int source_line_;

    // 問題を説明した文字列
    std::string msg_str_;

    // コンストラクタ
    DataException(const char *file, int line, const std::string &msg_str) {
        source_file_ = file;
        source_line_ = line;
        msg_str_ = msg_str;
    }

    // 保持している情報をストリームに出力する。
    // 以下のoperator<< の中から使うためのもの。
    void writeTo(std::ostream &os) const {
        os << "DataException : " << msg_str_
           << ", at \"" << source_file_
           << "\", line " << source_line_ << " ";
    }
};

// ストリームに例外のメッセージを出力するオペレータ。
// ヘッダの中に関数定義を書いているので、staticリンケージを指定しないと重複関数定義エラー
// になることに注意。
static inline std::ostream &operator<<(std::ostream &os, const DataException &exp) {
    exp.writeTo(os);
    return os;
}

#endif
