/*
 * Cell.cpp
 *
 *      Author: Hideo Takahashi
 */

#include <Cell.h>
#include <LJParams.h>
#include <Logger.h>

void Cell::clearForces() {
    for (Particle *pi = list_.head(); pi ; pi = pi->next_) {
        // clear the force value of pi.
        pi->a_dt2_half_.clear();
        //pi->vel_dt_.clear();
    }
}

void Cell::clearUp() {
    up_ = 0;
}

VectorXYZ Cell::calcLJforce(VectorXYZ const *dist, double r_2, LJScaledMoleculePairParam const *pair){
  double r_8 = r_2 * r_2 * r_2 * r_2;
  VectorXYZ f = *dist * ((pair->a_*r_2)/(r_8*r_8) + pair->b_/r_8);
  //std::cout << "coefficeisnt  :" << pair->a_ << "  " << pair->b_<< std::endl;
  //std::cout << "force a b  :" << (pair->a_*r_2)/(r_8*r_8) << "  " << (pair->b_/r_8)<< std::endl;
  //std::cout << " force " << f.x_ << "  " << f.y_ << " " << f.z_ << std::endl;
  return f;
}

void Cell::calcForceWithinSelf() {
    for (Particle *pi = list_.head(); pi != NULL; pi = pi->next_) {
        LJScaledMoleculeParam *parami = &LJParams::MOLECULE_PARAMS_[pi->kind_];
        for (Particle *pj = pi->next_; pj!=NULL; pj = pj->next_) {
            VectorXYZ dispij = pj->pos_ - pi->pos_; // displacement
            double r2 = dispij.square();
            if (r2 < LJParams::CUTOFF_SQ_) {
                // The two molecules are near enough.
                // Calculate force between pi and pj.
                // HINT : positions are stored as pi->pos_ and pj->pos_
                // molecule kinds are stored as pi->kind_ and pj->kind_
                // the coefficients to use to calculate force
                // is determined by the pair pi->kind_ and pj->kind_
                LJScaledMoleculeParam *paramj = &LJParams::MOLECULE_PARAMS_[pj->kind_];
                // use parami and paramj
                LJScaledMoleculePairParam *pair_ij = &LJParams::PAIR_PARAMS_[pi->kind_][pj->kind_];
                //powは使わない、14乗変数を作って再利用
                //同じものは再利用、符号をマイナス、ポテンシャル計算を関数化

                VectorXYZ force = calcLJforce(&dispij, r2, pair_ij);
                pi->a_dt2_half_ += force*parami->dt2_by_2m_;
                pj->a_dt2_half_ += force*(-1)*paramj->dt2_by_2m_;
            }
        }
    }
}

void Cell::calcForceWithLocalCell(Cell *otherCell) {
  for (Particle *pi = list_.head(); pi != NULL; pi = pi->next_) {
      LJScaledMoleculeParam *parami = &LJParams::MOLECULE_PARAMS_[pi->kind_];
      for (Particle *pj_local = otherCell->getParticleListHead(); pj_local != NULL; pj_local = pj_local->next_) {
          VectorXYZ dispij = pj_local->pos_ - pi->pos_; // displacement
          double r2 = dispij.square();
          if (r2 < LJParams::CUTOFF_SQ_) {  // The two molecules are near enough.
              LJScaledMoleculeParam *paramj = &LJParams::MOLECULE_PARAMS_[pj_local->kind_];
              // use parami and paramj
              LJScaledMoleculePairParam *pair_ij = &LJParams::PAIR_PARAMS_[pi->kind_][pj_local->kind_];

              VectorXYZ force = calcLJforce(&dispij, r2, pair_ij);
              pi->a_dt2_half_ += force*parami->dt2_by_2m_;
              pj_local->a_dt2_half_ += force*(-1)*paramj->dt2_by_2m_;
            }
          }
        }
}

void Cell::calcForceWithSurroundingCell(Cell *otherCell) {
  for (Particle *pi = list_.head(); pi != NULL; pi = pi->next_) {
      LJScaledMoleculeParam *parami = &LJParams::MOLECULE_PARAMS_[pi->kind_];
      for (Particle *pj_surround = otherCell->getParticleListHead(); pj_surround != NULL; pj_surround = pj_surround->next_) {
          VectorXYZ dispij = pj_surround->pos_ - pi->pos_; // displacement
          double r2 = dispij.square();
          if (r2 < LJParams::CUTOFF_SQ_) {  // The two molecules are near enough.
              LJScaledMoleculeParam *paramj = &LJParams::MOLECULE_PARAMS_[pj_surround->kind_];
              // use parami and paramj
              LJScaledMoleculePairParam *pair_ij = &LJParams::PAIR_PARAMS_[pi->kind_][pj_surround->kind_];


              pi->a_dt2_half_ += calcLJforce(&dispij, r2, pair_ij) * parami->dt2_by_2m_;
            }
          }
        }
}



void Cell::updatePosition() {
    for (Particle *pi = list_.head(); pi ; pi = pi->next_) {
        pi->pos_ += pi->vel_dt_;
        // HINT: The following line will produce a log file entry, for
        // debugging purposes. This will produce a log, each time
        // every molecule moves, and this will slow down the program significantly.
        // Once you have confirmed that the code works, try inhibiting
        // the log output by surrounding the following lines with an #ifdef ... #endif pair.
        //Logger::out << "molecule " << pi->serial_
        //          << " moves to " << pi->pos_ << std::endl;
    }
}


void Cell::updateVelocityHalf() {
  Particle *pi;
    for (pi = list_.head(); pi ; pi = pi->next_) {
        pi->vel_dt_ += pi->a_dt2_half_;
        // HINT: The following line will produce a log file entry, for
        // debugging purposes. This will produce a log, each time
        // every molecule moves, and this will slow down the program significantly.
        // Once you have confirmed that the code works, try inhibiting
        // the log output by surrounding the following lines with an #ifdef ... #endif pair.
      //  Logger::out << "velocity*dt of molecule " << pi->serial_
        //           << " is" << pi->vel_dt_<< std::endl;
    }
}

void Cell::migrateToNeighbor() {
    Particle *pi, *nexti;
    pi = list_.head();
    while (pi) {
        // Check if the particle has moved out of the cell.
        GridIndex3d idx = cellBox_.getRelativeIndexFor(pi->pos_);
        if (idx.equals(1,1,1)) {
            // it is still in our cell. let's move on to the next particle.
            pi = pi->next_;
        } else {
            // it has moved out of the cell.
            // find out which neighbor cell it should be sent to.
            Cell *destCell = neighborCellFor(idx);
          //  Logger::out << "molecule " << pi->serial_
          //              << " moves to cell : " << destCell->cellBox() << std::endl;
            // record the next item in the list, before we remove pi.
            nexti = pi->next_;
            // remove pi from our list
            list_.remove(pi);
            // hand it to the destination cell.
            destCell->addParticle(pi);
            // now move on to the particle that followed pi.
            pi = nexti;
        }
    }
}

//Force and Potential are calculated in the method below.

void Cell::calcForceWithinSelfAndUp() {
    for (Particle *pi = list_.head(); pi != NULL; pi = pi->next_) {
        LJScaledMoleculeParam *parami = &LJParams::MOLECULE_PARAMS_[pi->kind_];
        for (Particle *pj = pi->next_; pj; pj = pj->next_) {
            VectorXYZ dispij = pj->pos_ - pi->pos_; // displacement
            double r2 = dispij.square();
            if (r2 < LJParams::CUTOFF_SQ_) {
                // The two molecules are near enough.
                // Calculate force between pi and pj.
                // HINT : positions are stored as pi->pos_ and pj->pos_
                // molecule kinds are stored as pi->kind_ and pj->kind_
                // the coefficients to use to calculate force
                // is determined by the pair pi->kind_ and pj->kind_
                LJScaledMoleculeParam *paramj = &LJParams::MOLECULE_PARAMS_[pj->kind_];
                // use parami and paramj
                LJScaledMoleculePairParam *pair_ij = &LJParams::PAIR_PARAMS_[pi->kind_][pj->kind_];


                VectorXYZ force = calcLJforce(&dispij, r2, pair_ij);
                pi->a_dt2_half_ += force*parami->dt2_by_2m_;
                pj->a_dt2_half_ += force*(-1)*paramj->dt2_by_2m_;

                double r6 = r2*r2*r2;
                up_ += -pair_ij->a_ / (r6*r6*12) - pair_ij->b_ / (r6*6);
            }
        }
    }
}

void Cell::calcForceWithLocalCellAndUp(Cell *otherCell) {
  for (Particle *pi = list_.head(); pi != NULL; pi = pi->next_) {
      LJScaledMoleculeParam *parami = &LJParams::MOLECULE_PARAMS_[pi->kind_];
      for (Particle *pj_local = otherCell->getParticleListHead(); pj_local != NULL; pj_local = pj_local->next_) {
          VectorXYZ dispij = pj_local->pos_ - pi->pos_; // displacement
          double r2 = dispij.square();
          if (r2 < LJParams::CUTOFF_SQ_) {  // The two molecules are near enough.
              LJScaledMoleculeParam *paramj = &LJParams::MOLECULE_PARAMS_[pj_local->kind_];
              // use parami and paramj
              LJScaledMoleculePairParam *pair_ij = &LJParams::PAIR_PARAMS_[pi->kind_][pj_local->kind_];

              //LJポテンシャル計算

              VectorXYZ force = calcLJforce(&dispij, r2, pair_ij);
              pi->a_dt2_half_ += force*parami->dt2_by_2m_;
              pj_local->a_dt2_half_ += force*(-1)*paramj->dt2_by_2m_;

              //up_計算
              double r6 = r2*r2*r2;
              up_ += -pair_ij->a_ / (r6*r6*12) - pair_ij->b_ / (r6*6);
            }
          }
        }
}

void Cell::calcForceWithSurroundingCellAndUp(Cell *otherCell) {
  for (Particle *pi = list_.head(); pi != NULL; pi = pi->next_) {
      LJScaledMoleculeParam *parami = &LJParams::MOLECULE_PARAMS_[pi->kind_];
      for (Particle *pj_surround = otherCell->getParticleListHead(); pj_surround != NULL; pj_surround = pj_surround->next_) {
          VectorXYZ dispij = pj_surround->pos_ - pi->pos_; // displacement
          double r2 = dispij.square();
          if (r2 < LJParams::CUTOFF_SQ_) {  // The two molecules are near enough.
              LJScaledMoleculeParam *paramj = &LJParams::MOLECULE_PARAMS_[pj_surround->kind_];
              // use parami and paramj
              LJScaledMoleculePairParam *pair_ij = &LJParams::PAIR_PARAMS_[pi->kind_][pj_surround->kind_];

              //LJポテンシャル計算

              pi->a_dt2_half_ += calcLJforce(&dispij, r2, pair_ij) * parami->dt2_by_2m_;
              //up計算
              double r6 = r2*r2*r2;
              up_ += (-pair_ij->a_ / (r6*r6*12) - pair_ij->b_ / (r6*6))/2.0;
            }
          }
        }
}

void Cell::updateVelocityHalfAndCalcUk() {
  uk_ = 0; // 運動エネルギーの初期化
  Particle *pi;
    for (pi = list_.head(); pi ; pi = pi->next_) {
        pi->vel_dt_ += pi->a_dt2_half_;

        LJScaledMoleculeParam *parami = &LJParams::MOLECULE_PARAMS_[pi->kind_];
        //運動エネルギーの計算
        uk_ += pi->vel_dt_.square() * parami->m_by_2dt2_;
        // HINT: The following line will produce a log file entry, for
        // debugging purposes. This will produce a log, each time
        // every molecule moves, and this will slow down the program significantly.
        // Once you have confirmed that the code works, try inhibiting
        // the log output by surrounding the following lines with an #ifdef ... #endif pair.
        //Logger::out << "velocity*dt of molecule " << pi->serial_
        //           << " is" << pi->vel_dt_<< std::endl;
    }
}
