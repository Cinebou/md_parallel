#
# Lennard-Jones PotentialのMDのMakefile（スパコン向け）
#
# 使い方:
# (1) スパコン上でdebug バイナリのみビルドする
# make または make debug
# (2) スパコン上でdebug, release 双方のバイナリをビルドする
# make all
#
# (3) 生成したバイナリを削除する
# make clean
#

#
# 共通変数定義
#

# C++ コンパイラのコマンド名 :
CXX = icpc
# MPI用のC++コンパイラのコマンド名
MPICXX = mpiicpc
# C++コンパイラに渡す、debug/release 共通のコンパイルオプション
CXXFLAGS = -Iinclude -MMD -MF $@.d -qopenmp

# C++ コンパイラに渡す、debug版のコンパイルオプション（共通オプションを含む）
DEBUG_CXXFLAGS=$(CXXFLAGS) -g
# C++ コンパイラに渡す、release版のコンパイルオプション（共通オプション含む）
RELEASE_CXXFLAGS=$(CXXFLAGS) -O2 -axCORE-AVX512 -DNDEBUG

# リンク時にコンパイラに渡すオプション（ライブラリのリンク指定）
LDFLAGS = -qopenmp -lm

DEBUG_LDFLAGS=$(LDFLAGS)
RELEASE_LDFLAGS=$(LDFLAGS)

default: debug

all : debug release

# target definitions
#
# mdlj : マルチプロセス版
#
# mdlj_sp : シングルプロセス版

PROGS = mdlj mdlj_sp

DEBUG_TARGETS = $(PROGS:%=Debug/%)

RELEASE_TARGETS = $(PROGS:%=Release/%)

TEST_PROGS = test_CaseData test_Cell test_GridIterator3d test_LJParams \
  test_MdCommData test_MdProcData test_ParticleList test_VectorXYZ \
  test_MdCommunicator

TEST_TARGETS = $(TEST_PROGS:%=Debug/%)

debug : Debug $(DEBUG_TARGETS) $(TEST_TARGETS)

release : Release $(RELEASE_TARGETS)

#
# build instructions
#

Debug :
	mkdir Debug

Debug/%.o : src-nompi/%.cpp
	$(CXX) -c $(DEBUG_CXXFLAGS) -o $@ $<

Debug/%.o : src-mpi/%.cpp
	$(MPICXX) -c $(DEBUG_CXXFLAGS) -o $@ $<

Release :
	mkdir Release

Release/%.o : src-nompi/%.cpp
	$(CXX) -c $(RELEASE_CXXFLAGS) -o $@ $<

Release/%.o : src-mpi/%.cpp
	$(MPICXX) -c $(RELEASE_CXXFLAGS) -o $@ $<

#
# mdlj
#

mdlj_OBJS = mdlj.o MdDriver.o MdCommunicator.o \
  CaseData.o Cell.o FileReader.o LJParams.o \
  Logger.o MdCommData.o MdProcData.o MdDriver_dostepWithOutput.o \
	MdDriver_dostepWithoutOutput.o MdDriver_doInitialStep.o

Debug/mdlj : $(mdlj_OBJS:%=Debug/%)
	$(MPICXX) -o $@ $^ $(DEBUG_LDFLAGS)

Release/mdlj : $(mdlj_OBJS:%=Release/%)
	$(MPICXX) -o $@ $^ $(RELEASE_LDFLAGS)

#
# mdlj_sp
#

mdlj_sp_OBJS = mdlj_sp.o MdDriver_sp.o MdCommunicator_sp.o \
  CaseData.o Cell.o FileReader.o LJParams.o \
  Logger.o MdCommData.o MdProcData.o

Debug/mdlj_sp : $(mdlj_sp_OBJS:%=Debug/%)
	$(MPICXX) -o $@ $^ $(DEBUG_LDFLAGS)

Release/mdlj_sp : $(mdlj_sp_OBJS:%=Release/%)
	$(MPICXX) -o $@ $^ $(RELEASE_LDFLAGS)

#
# tests
#

test_GridIterator3d_OBJS = test_GridIterator3d.o TestBase.o
Debug/test_GridIterator3d : $(test_GridIterator3d_OBJS:%=Debug/%)
	$(CXX) -o $@ $^ $(DEBUG_LDFLAGS)

test_VectorXYZ_OBJS = test_VectorXYZ.o TestBase.o
Debug/test_VectorXYZ : $(test_VectorXYZ_OBJS:%=Debug/%)
	$(CXX) -o $@ $^ $(DEBUG_LDFLAGS)

test_LJParams_OBJS = test_LJParams.o TestBase.o LJParams.o
Debug/test_LJParams : $(test_LJParams_OBJS:%=Debug/%)
	$(CXX) -o $@ $^ $(DEBUG_LDFLAGS)

test_ParticleList_OBJS = test_ParticleList.o TestBase.o
Debug/test_ParticleList : $(test_ParticleList_OBJS:%=Debug/%)
	$(CXX) -o $@ $^ $(DEBUG_LDFLAGS)

test_CaseData_OBJS = test_CaseData.o TestBase.o CaseData.o FileReader.o Logger.o
Debug/test_CaseData : $(test_CaseData_OBJS:%=Debug/%)
	$(CXX) -o $@ $^ $(DEBUG_LDFLAGS)

test_Cell_OBJS = test_Cell.o TestBase.o Cell.o LJParams.o Logger.o
Debug/test_Cell : $(test_Cell_OBJS:%=Debug/%)
	$(CXX) -o $@ $^ $(DEBUG_LDFLAGS)

test_MdCommData_OBJS = test_MdCommData.o TestBase.o Cell.o MdCommData.o LJParams.o \
  CaseData.o FileReader.o Logger.o
Debug/test_MdCommData : $(test_MdCommData_OBJS:%=Debug/%)
	$(CXX) -o $@ $^ $(DEBUG_LDFLAGS)

test_MdProcData_OBJS = test_MdProcData.o TestBase.o MdProcData.o Cell.o \
  MdCommData.o LJParams.o CaseData.o FileReader.o Logger.o
Debug/test_MdProcData : $(test_MdProcData_OBJS:%=Debug/%)
	$(CXX) -o $@ $^ $(DEBUG_LDFLAGS)

#
# note: this one needs $(MPICXX) to link.
#

test_MdCommunicator_OBJS = test_MdCommunicator.o MpiTestBase.o TestBase.o \
  MdCommunicator.o MdCommData.o LJParams.o \
  CaseData.o FileReader.o Logger.o

Debug/test_MdCommunicator : $(test_MdCommunicator_OBJS:%=Debug/%)
	$(MPICXX) -o $@ $^ $(DEBUG_LDFLAGS)

clean:
	rm -rf Debug Release
	rm -f *.sh.o* *.log.*


#
# dependencies file
#
-include Debug/*.d Release/*.d
