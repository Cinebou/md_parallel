/*
 * CaseData.cpp
 */

#include <CaseData.h>
#include <FileReader.h>

#include <Logger.h>

void CaseData::init(const char *file_name, int my_rank, int num_procs) {
    assert(my_rank >= 0);
    assert(my_rank < num_procs);
    my_rank_ = my_rank;
    num_procs_ = num_procs;
    // 計算条件ファイルを読み込む
    readCaseFile(file_name);
    /*
     * ファイルに指定されているプロセス数と、MPIを起動するときに指定されている
     * プロセス数が整合しているか確認する。
     */
    if (npx_ * npy_ * npz_ != num_procs) {
        /*
         * ずれているので、メッセージ文字列を組み立てて、例外を投げる。
         *
         * 以下はC++で文字列を組み立てる時の、比較的性能のよい、安全な方法。
         */
        std::stringstream msg;
        msg << "num_procs = " << num_procs << ", does not match npx*npy*npz = ";
        msg << npx_ << "*" << npy_ << "*" << npz_ << std::endl;
        /*
         * 組み立てた文字列を取り出して例外を投げる
         */
        throw DataException(__FILE__, __LINE__, msg.str());
    }
    /*
     * プロセス座標の取りうるレンジを設定しておく。
     * root rankにおいて「全プロセスに関して」というタイプのループを記述する局面で使える。
     */
    allProcessesRange_.setRange(0,0,0, npx_-1, npy_-1, npz_-1);
    /*
     * 自身のプロセス座標を計算しておく。
     */
    setProcessIteratorForRank(&localProcess_, my_rank);
    /*
     * 自身のプロセス座標に基づいて、自身のプロセスセルの物理座標の範囲を計算しておく。
     */
    setBoxForProcess(&localBox_, localProcess_);

    /*
     * 時間発展ループの回次と時刻を初期化しておく。
     */
    t_ = 0;
    step_count_ = 0;
}

void CaseData::readCaseFile(const char *file_name) {
    /*
     * 入力ストリームを直接使うのではなく、エラー検知＆例外発生処理を実装した
     * FileReaderクラスを使ってファイルを読む。
     */
    FileReader rdr;
    rdr.open(file_name); // ファイルがなければここで IoException
    rdr.readLabeledStringLine("initial_state_file", initial_state_file_path_);
    rdr.readLabeledStringLine("restart_file", restart_file_path_);
    rdr.readLabeledStringLine("trajectory_file", trajectory_file_path_);
    rdr.readLabeledStringLine("energy_file", energy_file_path_);

    /*
     * シミュレーション全空間のサイズを読み込む
     * 読んだ後、値に無理がないかifでチェックしておかしければ例外を投げるようにしておくとベター
     */
    rdr.readLine();
    rdr.readKeyword("box_size");
    rdr.readDouble(lx_, "Lx");
    rdr.readDouble(ly_, "Ly");
    rdr.readDouble(lz_, "Lz");

    /*
     * プロセス分割数を読み込む。
     * 読んだ後で、0 < n < 1000 に収まっているか？と言ったチェックをしたほうが安全。
     * 特に、何かの間違いで0が読み込まれたのを見逃してしまうと、この後の除算でゼロ除算が生じて、
     * なにが問題を引き起こしたのか、なんの説明もなくプロセスがアボートしてしまう。
     */
    rdr.readLine();
    rdr.readKeyword("process_division");
    rdr.readInt(npx_, "Npx");
    rdr.readInt(npy_, "Npy");
    rdr.readInt(npz_, "Npz");

    /*
     * セル分割数を読み込む。
     * これも、値の上限を定めて範囲チェックを実装すべき。
     */
    rdr.readLine();
    rdr.readKeyword("cell_division");
    rdr.readInt(ncx_, "Ncx");
    rdr.readInt(ncy_, "Ncy");
    rdr.readInt(ncz_, "Ncz");

    rdr.readLabeledDoubleLine("delta_t", delta_t_);
    rdr.readLabeledDoubleLine("duration", duration_);
    rdr.readLabeledIntLine("output_interval", output_interval_);
    rdr.readLabeledDoubleLine("cutoff_radius", cutoff_radius_);
    /*
     * ファイルをクローズする
     */
    rdr.close();

    /*
     * プロセス分割セルの物理的大きさを求める
     */
    plx_ = lx_ / npx_;
    ply_ = ly_ / npy_;
    plz_ = lz_ / npz_;
    /*
     * ローカルセルの物理的大きさを求める
     */
    clx_ = plx_ / ncx_;
    cly_ = ply_ / ncy_;
    clz_ = plz_ / ncz_;
}

void CaseData::setProcessIteratorForRank(GridIndex3d *procIdx, int rank) const {
    assert(rank >= 0 && rank < num_procs_);
    int ipx     = rank / (npy_*npz_);
    int proc_yz = rank % (npy_*npz_);
    int ipy     = proc_yz / npz_;
    int ipz     = proc_yz % npz_;

    // 計算結果の検算
    assert(ipx >= 0 && ipx < npx_);
    assert(ipy >= 0 && ipy < npy_);
    assert(ipz >= 0 && ipz < npz_);
    assert(ipx*npy_*npz_ + ipy*npz_ + ipz == rank);

    // 計算結果の格納
    assert(procIdx != NULL);
    procIdx->ix_ = ipx;
    procIdx->iy_ = ipy;
    procIdx->iz_ = ipz;
}

int CaseData::getRankForProcess(const GridIndex3d &procIdx) const {
    int ipx = procIdx.ix_;
    int ipy = procIdx.iy_;
    int ipz = procIdx.iz_;
    assert(ipx >= 0 && ipx < npx_);
    assert(ipy >= 0 && ipy < npy_);
    assert(ipz >= 0 && ipz < npz_);
    int rank = ipx*npy_*npz_ + ipy*npz_ + ipz;
    return rank;
}

void CaseData::setBoxForProcess(BoxXYZ *box, const GridIndex3d &procIdx) const {
    assert(box != NULL);
    int ipx, ipy, ipz;
    ipx = procIdx.ix_;
    ipy = procIdx.iy_;
    ipz = procIdx.iz_;
    double xl, xh, yl, yh, zl, zh;
    xl = ipx * plx_;
    xh = xl + plx_;
    yl = ipy * ply_;
    yh = yl + ply_;
    zl = ipz * plz_;
    zh = zl + plz_;
    box->set(xl,yl,zl,xh,yh,zh);
}

void CaseData::setBoxForCell(BoxXYZ *box, const GridIndex3d &cellIdx) const {
    assert(box != NULL);
    int icx, icy, icz;
    icx = cellIdx.ix_ - 1; // Note -1
    icy = cellIdx.iy_ - 1;
    icz = cellIdx.iz_ - 1;
    double xl, xh, yl, yh, zl, zh;
    xl = localBox_.p1_.x_ + icx * clx_;
    xh = xl + clx_;
    yl = localBox_.p1_.y_ + icy * cly_;
    yh = yl + cly_;
    zl = localBox_.p1_.z_ + icz * clz_;
    zh = zl + clz_;
    box->set(xl,yl,zl,xh,yh,zh);
}
