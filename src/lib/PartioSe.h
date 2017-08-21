/**
PARTIO SOFTWARE
Copyright 202 Disney Enterprises, Inc. All rights reserved

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

* Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in
the documentation and/or other materials provided with the
distribution.

* The names "Disney", "Walt Disney Pictures", "Walt Disney Animation
Studios" or the names of its contributors may NOT be used to
endorse or promote products derived from this software without
specific prior written permission from Walt Disney Pictures.

Disclaimer: THIS SOFTWARE IS PROVIDED BY WALT DISNEY PICTURES AND
CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING,
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE, NONINFRINGEMENT AND TITLE ARE DISCLAIMED.
IN NO EVENT SHALL WALT DISNEY PICTURES, THE COPYRIGHT HOLDER OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND BASED ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
*/
#include <Partio.h>
#include <SeExpr2/Expression.h>
#include <map>

namespace Partio{

template<class T>
class AttribVar:public SeExpr2::ExprVarRef
{
    Partio::ParticlesDataMutable* parts;
    Partio::ParticleAttribute attr;
    int& currentIndex;
public:
    AttribVar(Partio::ParticlesDataMutable* parts,
        Partio::ParticleAttribute attr,int& currentIndex):
	  SeExpr2::ExprVarRef(SeExpr2::ExprType().FP(std::min(attr.count,3)).Varying()),
	  parts(parts),attr(attr),currentIndex(currentIndex) {}
    
    bool isVec(){return attr.count!=1;}
    void eval(double* result) override {
        const T* ptr=parts->data<T>(attr,currentIndex);
        const int clampedCount = type().dim();
        //std::cerr<<"in eval for "<<attr.name<<" count is "<<clampedCount<<" cur "<<currentIndex<<std::endl;
        for(int k=0;k<clampedCount;k++){
            result[k]=ptr[k];
        }
        // set any remaining fields (i.e. if clampedCount is 2)
        for(int k=clampedCount;k<3;k++){
            result[k]=0;
        }
    }
    void eval(const char**) override {}
};

struct SimpleVar:public SeExpr2::ExprVarRef{
    double val;
    SimpleVar():
      SeExpr2::ExprVarRef(SeExpr2::ExprType().FP(1).Varying()), val(0){}
    bool isVec(){return false;}
    void eval(double* result) override {
      result[0]=result[1]=result[2]=val;
    }
    void eval(const char**) override {}
};

/// Class that maps back to the partio data
template<class T> class VarToPartio;

/// NOTE: This class is experimental and may be deleted/modified in future versions
class PartioSe:public SeExpr2::Expression{
    bool isPaired;
    int currentIndex;
    Partio::ParticleAttribute pairH1,pairH2;
    int pairIndex1,pairIndex2;
    Partio::ParticlesDataMutable* parts;
    Partio::ParticlesDataMutable* partsPairing;
    typedef std::map<std::string,AttribVar<int>*> IntVarMap;
    mutable IntVarMap intVars;
    typedef std::map<std::string,AttribVar<float>*> FloatVarMap;
    mutable FloatVarMap floatVars;

    typedef std::vector<VarToPartio<int>*> IntVarToPartio;
    typedef std::vector<VarToPartio<float>*> FloatVarToPartio;
    IntVarToPartio intVarToPartio;
    FloatVarToPartio floatVarToPartio;

    mutable SimpleVar indexVar,countVar,timeVar;

    typedef SeExpr2::ExprVarRef* Var;
    mutable std::map<std::string, Var> vars;

public:
    typedef std::map<std::string, Var>::const_iterator LocalVarTableIterator;
    
    PartioSe(Partio::ParticlesDataMutable* parts,const char* expr);
    PartioSe(Partio::ParticlesDataMutable* partsPairing,Partio::ParticlesDataMutable* parts,const char* expr);
    void addSet(const char* prefix,Partio::ParticlesDataMutable* parts,int& setIndex);
    void addExport(const std::string& name,LocalVarTableIterator it,Partio::ParticlesDataMutable* parts,int& setIndex);
    virtual ~PartioSe();
    bool runAll();
    bool runRandom();
    void run(int i);
    bool runRange(int istart,int iend);
    void setTime(float val);
    SeExpr2::ExprVarRef*  resolveVar(const std::string& s) const;
private:
    PartioSe(const PartioSe&);
    PartioSe& operator=(const PartioSe&);
};
}
