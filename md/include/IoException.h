#ifndef _IO_EXCEPTION_H
#define _IO_EXCEPTION_H

#include <string>
#include <iostream>
#include <cerrno>
#include <cstring>

/*
 * ファイルの読み書きに失敗した場合に使用する例外クラス。
 * 指定された入力ファイルがない、権限がなくて出力ファイルを作れなかった、などの場合に使う
 */
class IoException {

    // throw が書いてあるソースファイル内の位置
    const char *source_file_;
    int source_line_;

    // OSから取得したエラー番号
    int errno_;

    // 問題を説明した文字列
    std::string name_;

public:
    IoException(const char *source_file, int source_line, const char *name) {
        source_file_ = source_file;
        source_line_ = source_line;
        name_ = name;
        errno_ = errno; // 右辺はOSから渡される、直前に起きたエラーのエラー番号。みかけは変数だが実際には複雑なマクロ。
    }

    IoException(const char *source_file, int source_line, const std::string &name) {
        source_file_ = source_file;
        source_line_ = source_line;
        name_ = name;
        errno_ = errno;
    }

    void writeTo(std::ostream &os) const {
        // strerror は、errno のエラー番号に対する説明文を返す関数
        os << "IoException : " << strerror(errno_)
           << ":" << name_ << ", at " << source_file_
           << "(" << source_line_ << ")\n";
    }
};

// ストリームに例外のメッセージを出力するオペレータ。
// ヘッダの中に関数定義を書いているので、staticリンケージを指定しないと重複関数定義エラー
// になることに注意。
static inline std::ostream &operator<<(std::ostream &os, const IoException &exp) {
    exp.writeTo(os);
    return os;
}


#endif
