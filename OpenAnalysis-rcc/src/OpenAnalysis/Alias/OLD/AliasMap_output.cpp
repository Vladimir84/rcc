// /!\ ATTENTION:
//
//     THIS IS AN AUTOMATICALLY GENERATED FILE
//     CREATED BY GenOutputTool.
//     DO NOT EDIT THIS FILE DIRECTLY AS IT WILL
//     BE OVERWRITTEN.

#include "AliasMap.hpp"
#include <OpenAnalysis/Utils/OutputBuilder.hpp>
#include <OpenAnalysis/Utils/OutputBuilder.hpp>

namespace OA {
  namespace Alias {

void AliasMap::output(OA::IRHandlesIRInterface& ir)
{
    sOutBuild->objStart("AliasMap");

    sOutBuild->fieldStart("mProcHandle");
    sOutBuild->outputIRHandle(mProcHandle, ir);
    sOutBuild->fieldEnd("mProcHandle");
    sOutBuild->field("mNumSets", OA::int2string(mNumSets));
    sOutBuild->field("mStartId", OA::int2string(mStartId));
    sOutBuild->mapStart("mIdToLocSetMap", "int", "OA::OA_ptr<LocSet> ");
    std::map<int, OA::OA_ptr<LocSet> >::iterator reg_mIdToLocSetMap_iterator;
    for(reg_mIdToLocSetMap_iterator = mIdToLocSetMap.begin();
        reg_mIdToLocSetMap_iterator != mIdToLocSetMap.end();
        reg_mIdToLocSetMap_iterator++)
    {
        const int &first = reg_mIdToLocSetMap_iterator->first;
        OA::OA_ptr<LocSet>  &second = reg_mIdToLocSetMap_iterator->second;
        if ( second.ptrEqual(0) ) continue;
        sOutBuild->mapEntryStart();
        sOutBuild->mapKey(OA::int2string(first));
        sOutBuild->mapValueStart();

        sOutBuild->listStart();
        LocSetIterator locIter(second);
        for (; locIter.isValid(); ++locIter) {
            OA_ptr<Location> loc = locIter.current();
            sOutBuild->listItemStart();
            loc->output(ir);
            sOutBuild->listItemEnd();
        }
        sOutBuild->listEnd();

        sOutBuild->mapValueEnd();
        sOutBuild->mapEntryEnd();
    }
    sOutBuild->mapEnd("mIdToLocSetMap");

    sOutBuild->mapStart("mIdToSetStatusMap", "int", "AliasResultType");
    std::map<int, AliasResultType>::iterator reg_mIdToSetStatusMap_iterator;
    for(reg_mIdToSetStatusMap_iterator = mIdToSetStatusMap.begin();
        reg_mIdToSetStatusMap_iterator != mIdToSetStatusMap.end();
        reg_mIdToSetStatusMap_iterator++)
    {
        const int &first = reg_mIdToSetStatusMap_iterator->first;
        AliasResultType &second = reg_mIdToSetStatusMap_iterator->second;
        sOutBuild->mapEntryStart();
        sOutBuild->mapKey(OA::int2string(first));
        sOutBuild->mapValueStart();
        sOutBuild->mapValue(OA::int2string(second));
        sOutBuild->mapValueEnd();
        sOutBuild->mapEntryEnd();
    }
    sOutBuild->mapEnd("mIdToSetStatusMap");

    sOutBuild->mapStart("mIdToMemRefSetMap", "int", "MemRefSet");
    std::map<int, MemRefSet>::iterator reg_mIdToMemRefSetMap_iterator;
    for(reg_mIdToMemRefSetMap_iterator = mIdToMemRefSetMap.begin();
        reg_mIdToMemRefSetMap_iterator != mIdToMemRefSetMap.end();
        reg_mIdToMemRefSetMap_iterator++)
    {
        const int &first = reg_mIdToMemRefSetMap_iterator->first;
        MemRefSet &second = reg_mIdToMemRefSetMap_iterator->second;
        sOutBuild->mapEntryStart();
        sOutBuild->mapKey(OA::int2string(first));
        sOutBuild->mapValueStart();
        sOutBuild->listStart();
        for (MemRefSet::iterator it = second.begin(); it != second.end(); ++it) {
	    MemRefHandle memRefHandle = *it;
            sOutBuild->listItemStart();
            sOutBuild->outputIRHandle(memRefHandle, ir);
            sOutBuild->listItemEnd();
        }
        sOutBuild->listEnd();
        sOutBuild->mapValueEnd();
        sOutBuild->mapEntryEnd();
    }
    sOutBuild->mapEnd("mIdToMemRefSetMap");

    sOutBuild->mapStart("mIdToMRESetMap", "int", "MemRefExprSet");
    std::map<int, MemRefExprSet>::iterator reg_mIdToMRESetMap_iterator;
    for(reg_mIdToMRESetMap_iterator = mIdToMRESetMap.begin();
        reg_mIdToMRESetMap_iterator != mIdToMRESetMap.end();
        reg_mIdToMRESetMap_iterator++)
    {
        const int &first = reg_mIdToMRESetMap_iterator->first;
        MemRefExprSet &second = reg_mIdToMRESetMap_iterator->second;
        sOutBuild->mapEntryStart();
        sOutBuild->mapKey(OA::int2string(first));
        sOutBuild->mapValueStart();
        sOutBuild->listStart();
        for (MemRefExprSet::iterator it = second.begin(); it != second.end(); ++it) {
	    OA_ptr<MemRefExpr> mre = *it;
            sOutBuild->listItemStart();
            mre->output(ir);
            sOutBuild->listItemEnd();
        }
        sOutBuild->listEnd();
        sOutBuild->mapValueEnd();
        sOutBuild->mapEntryEnd();
    }
    sOutBuild->mapEnd("mIdToMRESetMap");

#if 0
    // This is redundant with mIdToMemRefSetMap
    sOutBuild->mapStart("mMemRefToIdMap", "MemRefHandle", "int");
    std::map<MemRefHandle, int>::iterator reg_mMemRefToIdMap_iterator;
    for(reg_mMemRefToIdMap_iterator = mMemRefToIdMap.begin();
        reg_mMemRefToIdMap_iterator != mMemRefToIdMap.end();
        reg_mMemRefToIdMap_iterator++)
    {
        const MemRefHandle &first = reg_mMemRefToIdMap_iterator->first;
        int &second = reg_mMemRefToIdMap_iterator->second;
        sOutBuild->mapEntryStart();
        sOutBuild->mapKeyStart();
        sOutBuild->outputIRHandle(first, ir);
        sOutBuild->mapKeyEnd();
        sOutBuild->mapValue(OA::int2string(second));
        sOutBuild->mapEntryEnd();
    }
    sOutBuild->mapEnd("mMemRefToIdMap");
#endif

#if 0
    // This is redundant with mIdToMRESetMap.
    sOutBuild->mapStart("mMREToIdMap", "OA::OA_ptr<MemRefExpr> ", "int");
    std::map<OA::OA_ptr<MemRefExpr> , int>::iterator reg_mMREToIdMap_iterator;
    for(reg_mMREToIdMap_iterator = mMREToIdMap.begin();
        reg_mMREToIdMap_iterator != mMREToIdMap.end();
        reg_mMREToIdMap_iterator++)
    {
        const OA::OA_ptr<MemRefExpr>  &first = reg_mMREToIdMap_iterator->first;
        int &second = reg_mMREToIdMap_iterator->second;
        sOutBuild->mapEntryStart();
        sOutBuild->mapKeyStart();
        first->output(ir);
        sOutBuild->mapKeyEnd();
        sOutBuild->mapValue(OA::int2string(second));
        sOutBuild->mapEntryEnd();
    }
    sOutBuild->mapEnd("mMREToIdMap");
#endif

#if 0
    // This is redundant with mIdToLocSetMap.
    sOutBuild->mapStart("mLocToIdMap", "OA::OA_ptr<Location> ", "int");
    std::map<OA::OA_ptr<Location> , int>::iterator reg_mLocToIdMap_iterator;
    for(reg_mLocToIdMap_iterator = mLocToIdMap.begin();
        reg_mLocToIdMap_iterator != mLocToIdMap.end();
        reg_mLocToIdMap_iterator++)
    {
        const OA::OA_ptr<Location>  &first = reg_mLocToIdMap_iterator->first;
        int &second = reg_mLocToIdMap_iterator->second;
        sOutBuild->mapEntryStart();
        sOutBuild->mapKeyStart();
        first->output(ir);
        sOutBuild->mapKeyEnd();
        sOutBuild->mapValue(OA::int2string(second));
        sOutBuild->mapEntryEnd();
    }
    sOutBuild->mapEnd("mLocToIdMap");
#endif
    sOutBuild->objEnd("AliasMap");
}

  } // end of Alias namespace
} // end of OA namespace
