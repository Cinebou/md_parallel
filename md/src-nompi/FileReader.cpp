#include <FileReader.h>
#include <Logger.h>

FileReader::FileReader() {

}

FileReader::~FileReader() {
    // FileReaderオブジェクトを使っている関数が途中で例外などで終了した場合、
    // FileReader::closeメソッドを経ずにいきなりデストラクタが呼ばれる場合がある。
    // その場合でもclose漏れが生じないようにデストラクタでcloseを呼ぶ。

    // 一方、例外に見舞われなかった正常ケースでは、FileReaer::closeメソッドが呼ばれた後で、
    // デストラクタが呼ばれる。その場合にはこのclose呼び出しは冗長になるが、危険性はない。
    in_.close();
}


void FileReader::open(const char *file_name) {
    // 今後、エラーメッセージを書く場合に備えて、ファイル名を保存しておく
    file_name_ = file_name;
    // ファイルを開く
    in_.open(file_name_.c_str(), std::ios::in);
    // 成功したか確認する
    if (! in_.is_open()) {
        // 失敗の場合、例外を挙げる。
        throw IoException(__FILE__, __LINE__, file_name_);
    }
    // 今後、エラーメッセージを書く場合に備えて、行番号を数えていく。
    // そのカウンタを初期化しておく。
    line_no_ = 0;
    // ファイルのopen/closeの契機でログを残す。
    Logger::out << "File opened: " << file_name_ << std::endl;
}

void FileReader::close() {
    // FileReaderのユーザーがcloseを呼ぶと、closeを処理をして、さらにログを残す。
    in_.close();
    Logger::out << "File closed: " << file_name_ << std::endl;
}

bool FileReader::readLine() {
    std::string line_buf; // 読み込んだ行を一旦保持するためのstring
    // ファイルから一行読み込む。
    // これは、改行コードに遭遇するまでファイルを読み込むメソッドであり、
    // 間違えてバイナリファイルを開いてしまい、たまたま改行コードに一致するバイトが
    // ないまま、データが延々と続いているような場合、延々と読み込んでしまう。
    // 安全を期するには、読み込む一行の最大長を決めておいて、指定するとよい。
    // （ここで使っているgetlineメソッドで指定できたかどうかは、未確認）
    std::getline(in_, line_buf);
    if (!in_.eof()) {
        // end of file ではなかった場合、メンバ変数の cur_line_ (これはstringとは違うクラス)に文字列を格納する
        cur_line_.str(line_buf);
        cur_line_.clear(); // clear eof state.
        line_no_++; // 入力ファイルを一行消費したので、カウンタを進めておく。
        return true; // yes, we got data.
    } else {
        // end of file だった場合は、入力行バッファを空にして、falseを返す
        cur_line_.str("");
        cur_line_.clear();
        return false; // no, we don't have any more data.
    }
}

void FileReader::readLabeledDoubleLine(const char *label, double &val) {
    // 行を読む
    readLine();
    // 単語を行から取り出し、期待しているlabelと一致しているか確認する
    readKeyword(label);
    // double 値を取り出す。
    readDouble(val, label);
}

void FileReader::readLabeledIntLine(const char *label, int &val) {
    readLine();
    readKeyword(label);
    readInt(val, label);
}

void FileReader::readLabeledStringLine(const char *label, std::string &val) {
    readLine();
    readKeyword(label);
    readString(val, label);
}

void FileReader::readKeyword(const char *keyword) {
    std::string word;
    // 行バッファから、単語（何が「単語」とみなされるかは istream &operator>>(istream &is, string &s) の仕様を参照）
    // http://www.cplusplus.com/reference/string/string/operator%3E%3E/
    cur_line_ >> word;
    // 期待したキーワードが得られたか?
    if (word != keyword || cur_line_.fail()) {
        // 得られなかったので例外を挙げる。文字列の途中にダブルクウォートを書く場合には
        // \ を書く。\に続くバッククウォートは、文字列の終端記号ではなく、単なる文字として扱われる。
        std::stringstream msg;
        msg << "Keyword \"" << keyword <<"\" was excpected, but \"" << word << "\" was found at ";
        addFileNameAndLineNoTo(msg);
        throw DataException(__FILE__, __LINE__, msg.str());
    }
}

void FileReader::readDouble(double &val, const char *label) {
    // double の値をストリームから取り出す
    cur_line_ >> val;
    // 形式エラーや、もう入力の終端に達してしまった場合には fail() が trueになる。
    if (cur_line_.fail()) {
        std::stringstream msg;
        // 引数 label には、読み出そうとしていた値の名前が渡されている。
        // これにより、例えば "x" の値を読もうとして失敗したのか、"y"の値を読もうとして失敗したのかが、
        // エラーメッセージから読み取れるようになる。エラーがなければ、labelは使われずじまい。
        msg << "floating point value for " << label << " was expected at ";
        addFileNameAndLineNoTo(msg); // 読んでいたファイル名と行番号をmsgに追加する。
        // できあがったエラーメッセージを保持した例外を挙げる。
        throw DataException(__FILE__, __LINE__, msg.str());
    }
}

void FileReader::readInt(int &val, const char *label) {
    // int を取り出す。
    cur_line_ >> val;
    if (cur_line_.fail()) {
        std::stringstream msg;
        msg << "integer value for " << label << " was expected at ";
        addFileNameAndLineNoTo(msg);
        throw DataException(__FILE__, __LINE__, msg.str());
    }
}

void FileReader::readString(std::string &val, const char *label) {
    // 文字列（単語）を取り出す。
    cur_line_ >> val;
    if (cur_line_.fail()) {
        std::stringstream msg;
        msg << "string for " << label << " was expected at ";
        addFileNameAndLineNoTo(msg);
        throw DataException(__FILE__, __LINE__, msg.str());
    }
}

void FileReader::readExpectedInt(int expected_val, const char *label) {
    int val;
    // 整数値を取得する
    cur_line_ >> val;
    // 整数値の取得に成功し、かつ、取得できた整数値が期待したものか確認する。
    if (cur_line_.fail() || val != expected_val) {
        std::stringstream msg;
        msg << "integer value " << expected_val << " for " << label << " was expected at ";
        addFileNameAndLineNoTo(msg);
        throw DataException(__FILE__, __LINE__, msg.str());
    }
}

void FileReader::addFileNameAndLineNoTo(std::stringstream &ss) {
    ss << "\"" << file_name_ << "\", line " << line_no_;
}
