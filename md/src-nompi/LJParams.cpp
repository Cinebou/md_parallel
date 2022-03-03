/*
 * LJParams.cpp
 *
 *      Author: Hideo Takahashi
 */

#include <LJParams.h>
#include <cmath>
#include <cstring>

LJMoleculeParam LJParams::SOURCE_PARAMS_[LJ_MOLECULE_TYPES] = {
    {
            /* 0 */
            "He",
            4.0026022,
            0.141e-21,
            2.56 //平行地点　r = 2^(1/6)sigma, = 2.87
    }, {
            /* 1 */
            "Ne",
            20.17976,
            0.492e-21,
            2.75
    }, {
            /* 2 */
            "Ar",
            39.948,
            1.70e-21,
            3.40
    }, {
            /* 3 */
            "Kr",
            83.7982,
            2.30e-21,
            3.68
    }, {
            /* 4 */
            "Xe",
            131.2936,
            3.1e-21,
            4.07
    }, {
            /* 5 */
            "N2",
            28.01344,
            1.25e-21,
            3.70
    }, {
            /* 6 */
            "I2",
            253.808946,
            7.6e-21,
            4.98
    }, {
            /* 7 */
            "Hg",
            200.592,
            11.74e-21,
            2.90
    }, {
            /* 8 */
            "CCl4",
            153.82358,
            4.51e-21,
            5.88
    }
};

LJScaledMoleculeParam LJParams::MOLECULE_PARAMS_[LJ_MOLECULE_TYPES];

LJScaledMoleculePairParam LJParams::PAIR_PARAMS_[LJ_MOLECULE_TYPES][LJ_MOLECULE_TYPES];

double LJParams::CUTOFF_SQ_; /* square of cutoff distance */

void LJParams::initParams(CaseData *caseData) {
    double dt = caseData->delta_t_; // [fs]
    int i, j;
    int n = LJ_MOLECULE_TYPES;
    for (i = 0; i < n; i++) {
        /*
         * Source values for the first atom can be accessed as alpha->mass_ etc.
         */
        LJMoleculeParam *alpha = &SOURCE_PARAMS_[i];
        /*
         * The scaled molecule params can be accessed as scaled_single->dt2_by_2m_ etc.
         */
        LJScaledMoleculeParam *scaled_single = &MOLECULE_PARAMS_[i];
        /*
         * Set the scaled molecule params here:
         */
        scaled_single->dt2_by_2m_ = dt*dt/(2*alpha->mass_);
        scaled_single->m_by_2dt2_ = alpha->mass_/(2*dt*dt);

        /*
         * Compute the pair molecule params.
         */
        for (j = 0; j < n; j++) {
            /*
             * Source values for the second atom can be accessed as beta->mass_ etc.
             */
            LJMoleculeParam *beta = &SOURCE_PARAMS_[j];
            /*
             * Scaled pair params can be accessed as scaled_pair->a_
             */
            LJScaledMoleculePairParam *scaled_pair = &PAIR_PARAMS_[i][j];
            /*
             * set any values to pair_param here.
             */
            double eps =  sqrt(alpha->epsilon_ * beta->epsilon_);
            double sig = (alpha->sigma_ + beta->sigma_)/2;
            double sig6 = sig*sig*sig*sig*sig*sig;
            scaled_pair->a_ = -48*eps* 6.02e+16 * sig6*sig6;
            scaled_pair->b_ = 24*eps* 6.02e+16 * sig6;



        }
    }
    // don't forget to set CUTOFF_SQ_
    CUTOFF_SQ_ = caseData->cutoff_radius_ * caseData->cutoff_radius_;
}

int LJParams::nameToMoleculeKind(const char *name) {
    // search for the name in the known molecule types list.
    for (int i = 0; i < LJ_MOLECULE_TYPES; i++) {
        if (strcmp(SOURCE_PARAMS_[i].label_, name) == 0) {
            return i;
        }
    }
    // not found.
    std::stringstream msg;
    msg << "Molecule name \"" << name << "\" not found.\n";
    msg << "Supported names are : ";
    for (int i = 0; i < LJ_MOLECULE_TYPES; i++) {
        if (i > 0) {
            msg << ", ";
        }
        msg << "\"" << SOURCE_PARAMS_[i].label_ << "\"";
    }
    msg << ".\n";
    throw DataException(__FILE__,__LINE__, msg.str());
}
