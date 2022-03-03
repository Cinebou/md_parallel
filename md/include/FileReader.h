#ifndef _FILE_READER_H
#define _FILE_READER_H

#include <string>
#include <fstream>
#include <sstream>

#include <IoException.h>
#include <DataException.h>

/*
 * テキスト形式の数値データファイルを読むためのクラス
 * std::ifstreamを直接使うと、数値データが読めなかった場合に、自ら積極的にエラーが起きていないか
 * 確認しないと、エラーの有無を知ることができない。シミュレーションプログラムでは数値データが
 * 一つでも読めなかったら致命的な問題なので、本クラスでは、エラーチェックがしっかり入った
 * データ読み込みメソッドを提供する。また、計算条件ファイルに便利なように、データの意味を表す
 * ラベルや、ファイルのパス名などの文字列を読むための機能も提供する。
 *
 * 数値データファイルで、データの作成者とプログラマの間でデータ形式の理解のズレがあると致命的である。
 * 特に、一行に含まれる項目の個数を誤解して、改行を無視して次の行のデータを間違って取得してしまうことは
 * 防ぎたい。ifstreamを直接使うと、改行を読みとばしてしまう。
 * そこで、本クラスでは、ファイルのデータを一旦、一行分、バッファ(cur_line_)に読み込んで、
 * このバッファから項目を取り出す。もしも一行に含まれているよりも多くのデータを取り出そうとすると
 * その行にはもうデータがないことを検知できる。
 */
class FileReader {
private:
    // ファイル名。ファイルを開く時点だけでなく、エラーに遭遇したときにファイル名をエラーメッセージに
    // 含めるためにも、メンバ変数として保持しておく必要がある。
    std::string file_name_;

    // 入力ストリーム
    std::fstream in_;

    // 一行分のデータを保持するバッファ。このクラスの働きについては入門書を参照のこと。
    std::stringstream cur_line_;

    // ファイルの中の行番号
    int line_no_;

public:

    FileReader();
    ~FileReader();

    // ファイルを開く
    // 例外：
    //   IoException : ファイルを開けない
    void open(const char *file_name);

    // ファイルを開く（ファイル名をstringオブジェクトで渡せる他は、上記と同一）
    void open(const std::string &file_name) {
        open(file_name.c_str());
    }

    // ファイルを閉じる
    void close();

    ///
    /// (a) 一行単位のメソッド。これらのメソッドは一行をバッファに読み込んだ上で、
    ///     それぞれのメソッドに応じたデータをその行から読み出す。
    ///

    // "ラベル double値" の形式の行を読む。例 "time 1.0e-5"
    // 期待しているラベル値をlabelに渡す。ファイルの中のラベル値が期待と異なると例外を上げる。
    // 読み取ったdouble値をvalに格納する。
    // 例外:
    //   IoException : End of Fileなど
    //   DataException : ラベル値が期待と違う、double値を読み取れなかった
    void readLabeledDoubleLine(const char *label, double &val);

    // "ラベル int値" の形式の行を読む。例 "count 100"
    // 期待しているラベル値をlabelに渡す。ファイルの中のラベル値が期待と異なると例外を上げる。
    // 読み取ったint値をvalに格納する。
    // 例外:
    //   IoException : End of Fileなど
    //   DataException : ラベル値が期待と違う、int値を読み取れなかった
    void readLabeledIntLine(const char *label, int &val);

    // "ラベル 文字列" の形式の行を読む。例 "filename data.txt"
    // 期待しているラベル値をlabelに渡す。ファイルの中のラベル値が期待と異なると例外を上げる。
    // 読み取った文字列をvalに格納する。
    // 例外:
    //   IoException : End of Fileなど
    //   DataException : ラベル値が期待と違う、文字列を読み取れなかった
    void readLabeledStringLine(const char *label, std::string &val);

    ///
    /// (b) 一行の中身を個別に扱うためのメソッド。これらのメソッドを使う場合は、
    ///     一行をバッファに読む処理を明示的に呼んでから、希望するデータ項目を
    ///     取得するメソッドを呼ぶ。
    ///

    // ファイルから一行をバッファに読み込む。
    // 行があればtrueを返す。EOF(end of file)でこのメソッドを呼ぶとfalseを返す。
    // 例外:
    //   IoException : 入力に失敗
    bool readLine();

    // バッファからキーワードを読み込み、引数と比較する
    // 期待したキーワードが登場することを確認するメソッド。
    // 例外:
    //   DataException : 期待したキーワードが登場しなかった
    void readKeyword(const char *keyword);

    // バッファからdouble値を読み込む。プログラムにとっての値の意味を示す文字列（変数名など）をlabelに渡す
    // 値の読み込みに失敗した場合にそのラベルがエラーメッセージに含められる。
    // 例外:
    //   DataException : double 値の読み出しに失敗した
    void readDouble(double &val, const char *label);

    // バッファからint値を読み込む。
    // 例外:
    //   DataException : int 値の読み出しに失敗した
    void readInt(int &val, const char *label);

    // バッファからint値を読み込み、期待している値と一致していることを確認する。
    // 通し番号など、ファイルの中の値が規則的に決まっている場合に、ずれが生じていないことを確認する場合に使う。
    // 例外:
    //   DataException : int 値の読み出しに失敗した。読み出せたが、期待したint値と異なった
    void readExpectedInt(int val, const char *label);

    // バッファから文字列を読み込む。
    // 例外:
    //   DataException : 読み込みに失敗した
    void readString(std::string &val, const char *label);

private:

    // stringstreamに、読み込み中のファイル名と行番号を、エラーメッセージに適する形式で書き加える。
    void addFileNameAndLineNoTo(std::stringstream &ss);
};
#endif
