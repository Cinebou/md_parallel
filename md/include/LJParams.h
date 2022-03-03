/*
 * LJParams.h
 *
 *      Author: Hideo Takahashi
 */

#ifndef LJPARAMS_H_
#define LJPARAMS_H_

#include <PhysicalConsts.h>
#include <DataException.h>
#include <CaseData.h>
#include <Particle.h>

/*
 * 本プログラムがLennard Jonesポテンシャルのためのパラメタを保持している分子の種類の数
 */
const int LJ_MOLECULE_TYPES = 9;

/*
 * 分子の種類ごとのデータ
 */
struct LJMoleculeParam {
    // 分子の名称。この名称がファイル中に記載される。
    const char *label_;

    // 分子の質量
    double mass_;    /* [u] = [M_AMU kg] */
    // LJポテンシャルのε
    double epsilon_; /* [J] = [Kg*m^2*s^-2] */
    /*
     * J = Newton * m
     *   = kg * m * s^-2 * m = kg * m^2 * s^-2
     *   = (1/1.660e-27 u) * (1.0e+10 Angstrom)^2 * (1.0e+15 fs)^-2
     *   = 1/1.660e-27 * 1.0e+20 * 1.0e-30 * u * Angstrom^2 * fs^-2
     *   = 6.02e+26 * 1.0e-10 * u * Angstrom^2 * fs^-2
     *   = 6.02e+16 * u * Angstrom^2 * fs^-2
     *   = 6.02e+16 aeu  [ aeu : atomic scale energy unit ]
     */


    // LJポテンシャルのσ
    double sigma_;   /* [Angstrom] */
};

/*
 * 分子の種類ごとに決まるループ不変量を
 * 本プログラムで利用する単位系のスケールに合わせたもの
 */
struct LJScaledMoleculeParam {
    /* for calculation of force */
    double dt2_by_2m_; /* [fs^2/u] */

    /* for calculation of Uk */
    double m_by_2dt2_; /* [u*fs^-2] */

};

/*
 * 分子の組み合わせごとに決まるループ不変量を
 * 本プログラムで利用する単位系のスケールに合わせたもの
 */
struct LJScaledMoleculePairParam {
    /* for calculation of Up */
    /* phi = (a)*r^-12 - (b)*r^-6 */
    double a_; // [aeu * Angstrom^12]
    double b_; // [aeu * Angstrom^6]
};

class LJParams {
public:

    // 添字は分子種別番号(0～LJ_MOLECULE_TYPES-1)。各番号の意味はLJParams.cpp参照。

    static LJMoleculeParam SOURCE_PARAMS_[LJ_MOLECULE_TYPES];
    static LJScaledMoleculeParam MOLECULE_PARAMS_[LJ_MOLECULE_TYPES];
    static LJScaledMoleculePairParam PAIR_PARAMS_[LJ_MOLECULE_TYPES][LJ_MOLECULE_TYPES];
    static double CUTOFF_SQ_; /* square of cutoff distance */

    static void initParams(CaseData *caseData);

    // 分子の種類を表す文字列から、分子種別番号を見つけ出す。
    // 例外:
    //   DataException: 分子名がみつからなかった場合。
    static int nameToMoleculeKind(const char *name);

};

#endif /* LJPARAMS_H_ */
