/*
 * VersionedObjectBuilder.h
 *
 * URL:      https://github.com/panchaBhuta/dataStructure
 * Version:  v2.2.6
 *
 * Copyright (C) 2023-2024 Gautam Dhar
 * All rights reserved.
 * 
 * dataStructure is distributed under the BSD 3-Clause license, see LICENSE for details. 
 *
 */

#pragma once

#include <array>
#include <vector>
#include <set>
#include <stdexcept>

#include <versionedObject/VersionedObject.h>
#include <versionedObject/ChangesInDataSet.h>


namespace datastructure { namespace versionedObject
{

  using FirstDeltaChange_Before_StartDate_Timeline_exception             =  VO_exception<1>;
  using Empty_VersionObject_exception                                    =  VO_exception<4>;
  using FirstVersion_Before_LastDeltaChange_ReverseTimeline_exception    =  VO_exception<5>;
  using LastVersion_After_FirstDeltaChange_ForwardTimeline_exception     =  VO_exception<6>;
  using Unexpected_ApplicableChangeDirection_exception                   =  VO_exception<7>;

  template <typename VDT, typename ... MT>
  class _VersionedObjectBuilderBase
  {
  public:
    using t_deltaEntriesMap  = typename std::multimap < VDT, ChangesInDataSet<MT...> >;

  protected:
    using t_versionDate      = VDT;
    using t_versionedObject  = VersionedObject<VDT, MT...>;
    using t_dataset          = DataSet<MT...>;
    using t_record           = typename t_dataset::t_record;
    using t_metaData         = typename t_dataset::t_metaData;
    using t_datasetLedger    = typename t_versionedObject::t_datasetLedger;

    t_deltaEntriesMap  _deltaEntries;

    _VersionedObjectBuilderBase() : _deltaEntries() {}
    virtual ~_VersionedObjectBuilderBase()
    {
      _deltaEntries.clear();
    }

    inline static t_dataset _datasetFactory(
                    [[maybe_unused]]  const t_metaData* const ptrMetaData,
                                      const t_record& record)
    {
      if constexpr ( t_dataset::hasMetaData() )
      {
        return t_dataset(*ptrMetaData, record);
      } else {
        return t_dataset(record);
      }
    }

    void _buildForwardTimeline( // with filled VersionObject
                  t_versionedObject& vo,
                  const t_metaData* const metaDataResetCloner = nullptr) // nullptr when MetaData is NOT used
    {
      /*
      _buildForwardTimeline
      vo{
        Version Objects, timeline ...
        last-VersionObject
      }
      Expected ::   first-ChangeDelta version-date > last-VersionObject-date
      _deltaEntries{
        first ChangeDelta Version,
        remaining Change Delta Versions, timeline ...
        last-ChangeDelta version
      }
      */

      if(_deltaEntries.empty())
      {
        return;
      }

      if(vo.getDatasetLedger().empty())
      {
        std::ostringstream eoss;
        eoss << "ERROR(1) : failure in _VersionedObjectBuilderBase<VDT, MT...>::_buildForwardTimeline() : "
                "VersionObject cannot be empty." << std::endl;
        toStr("VersionObjectBuilder :: ", eoss);
        throw Empty_VersionObject_exception(eoss.str());
      }

      for( auto iterDelta = _deltaEntries.begin(); iterDelta != _deltaEntries.end(); ++iterDelta )
      {
        if(iterDelta->second.getApplicableChangeDirection() == ApplicableChangeDirection::REVERSE)
        {
          std::ostringstream eoss;
          eoss << "ERROR(2) : failure in _VersionedObjectBuilderBase<VDT, MT...>::_buildForwardTimeline() : for changeDate[";
          eoss << iterDelta->first << "] should NOT be marked as 'ApplicableChangeDirection::REVERSE', expected FORWARD" << std::endl;
          toStr("VersionObjectBuilder :: ", eoss);
          vo.toStr("VersionObject :: ", eoss);
          throw Unexpected_ApplicableChangeDirection_exception(eoss.str());
        }
      }

      auto iterDelta = _deltaEntries.begin();

      auto lastVersionIter = vo.getDatasetLedger().rbegin();
      t_versionDate firstDeltaChangeDate = iterDelta->first;
      t_versionDate lastVersionDate = lastVersionIter->first;
      if( firstDeltaChangeDate <= lastVersionDate ) //  no overlap in ranges
      {
        std::ostringstream eoss;
        eoss << "ERROR(3) : failure in _VersionedObjectBuilderBase<VDT, MT...>::_buildForwardTimeline() : first-deltaChangeDate[";
        eoss << firstDeltaChangeDate << "] should be more than last-VersionDate[" << lastVersionDate << "]" << std::endl;
        vo.toStr("VersionObject :: ", eoss);
        toStr("VersionObjectBuilder :: ", eoss);
        throw LastVersion_After_FirstDeltaChange_ForwardTimeline_exception(eoss.str());
      }


      std::array <bool, std::tuple_size_v< t_record > > hitheroProcessedElements;
      for(size_t i = 0; i < hitheroProcessedElements.size(); ++i)
      {
        hitheroProcessedElements[i] = false;
      }

      t_metaData metaData = t_dataset::hasMetaData() ?
                                      (*metaDataResetCloner):
                                      t_metaData();

      //previous checks gaurantees firstDeltaChangeDate > lastVersionDate
      typename t_versionedObject::t_datasetLedger::const_iterator
          ledgerIter = vo.getVersionAt(firstDeltaChangeDate);
      t_record record { ledgerIter->second.getRecord() };

      t_versionDate pastDeltaChangeDate = iterDelta->first;
      while( iterDelta != _deltaEntries.end() )
      {
        /*
        std::cout << "deltaEntryDate[" << iterDelta->first << " =?= pastDeltaChangeDate(" << pastDeltaChangeDate << ") " << std::flush;
        if( vo.getVersionAt(iterDelta->first) != vo.getDatasetLedger().end() )
        {
          std::cout << "]->versionDate[" << std::flush;
          //ledgerIter = vo.getVersionAt(iterDelta->first);
          ////lastVersion        = &(ledgerIter->second);
          //record = ledgerIter->second.getRecord();
          std::cout << vo.getVersionAt(iterDelta->first)->first << std::flush;
        }
        std::cout << "] -> data{" << ledgerIter->second.toCSV() 
                  << "} << deltaChange{" << iterDelta->second.toCSV() << "}" << std::endl;
        */

        if(pastDeltaChangeDate != iterDelta->first)
        {
          t_dataset dataset = _datasetFactory(&metaData, record);
          //std::cout << "vo.insertVersion -> versionDate: " << rIterDelta->first << ":" << dataset.toCSV() << std::endl;
          vo.insertVersion( pastDeltaChangeDate, dataset );
          if constexpr ( t_dataset::hasMetaData() )
          {
            metaData = (*metaDataResetCloner); // reset of metaData values
          }
          if( vo.getVersionAt(iterDelta->first) != vo.getDatasetLedger().end() )
          {
            ledgerIter = vo.getVersionAt(iterDelta->first);
            record = ledgerIter->second.getRecord();
          }

          // two consecutive changes , but on different dates.
          // Hence treat it as new Record with no hitheroProcessedElements.
          for(size_t i = 0; i < hitheroProcessedElements.size(); ++i)
          {
            hitheroProcessedElements[i] = false;
          }
        }

        if constexpr ( t_dataset::hasMetaData() )
        {
          metaData.appendMetaInfo(iterDelta->second.getMetaData());
        }

        try {
          // 'getRecord()' is called with VALIDATE=true
          iterDelta->second.getLatestRecord(record, hitheroProcessedElements);
          //VERSIONEDOBJECT_DEBUG_LOG("after delta-change record=" << converter::ConvertFromTuple<MT...>::ToStr(record));
        } catch (const std::exception& err) {
          std::ostringstream eoss;
          eoss << "ERROR(4) : failure in _VersionedObjectBuilderBase<VDT, MT...>::_buildForwardTimeline() : "
               << err.what() << std::endl;
          eoss << "DataSet :: ";
          ledgerIter->second.toCSV(eoss);
          toStr("VersionObjectBuilder :: ", eoss);
          throw std::invalid_argument(eoss.str());
        }

        pastDeltaChangeDate = iterDelta->first;
        ++iterDelta;
      }

      t_dataset dataset = _datasetFactory(&metaData, record);
      //std::cout << "vo.insertVersion -> versionDate: " << pastDeltaChangeDate << ":" << dataset.toCSV() << std::endl;
      vo.insertVersion( pastDeltaChangeDate, dataset );

      //VERSIONEDOBJECT_DEBUG_LOG("LAST change record=" << converter::ConvertFromTuple<MT...>::ToStr(record));
      _deltaEntries.clear();
    }


    void _buildReverseTimeline( // with filled VersionObject
                  const t_versionDate& startDate,
                  t_versionedObject& vo,
                  const t_metaData* const metaDataResetCloner = nullptr) // nullptr when MetaData is NOT used
    {
      /*
      _buildReverseTimeline
      _deltaEntries{
        startDate of first ChangeDelta Version,
        remaining Change Delta Versions, timeline ...
        last-ChangeDelta version
      }
      Expected :: last-ChangeDelta version-date == first-VersionObject-date
      vo{
        first-VersionObject
        Version Objects, timeline ...
      }
      */

      if(_deltaEntries.empty())
      {
        return;
      }

      if(vo.getDatasetLedger().size() == 0)
      {
        std::ostringstream eoss;
        eoss << "ERROR(1) : failure in _VersionedObjectBuilderBase<VDT, MT...>::_buildReverseTimeline() : "
                "VersionObject cannot be empty." << std::endl;
        toStr("VersionObjectBuilder :: ", eoss);
        throw Empty_VersionObject_exception(eoss.str());
      }

      for( auto iterDelta = _deltaEntries.begin(); iterDelta != _deltaEntries.end(); ++iterDelta )
      {
        if(iterDelta->second.getApplicableChangeDirection() == ApplicableChangeDirection::FORWARD)
        {
          std::ostringstream eoss;
          eoss << "ERROR(2) : failure in _VersionedObjectBuilderBase<VDT, MT...>::_buildReverseTimeline() : for changeDate[";
          eoss << iterDelta->first << "] should NOT be marked as 'ApplicableChangeDirection::FORWARD', expected REVERSE" << std::endl;
          toStr("VersionObjectBuilder :: ", eoss);
          vo.toStr("VersionObject :: ", eoss);
          throw Unexpected_ApplicableChangeDirection_exception(eoss.str());
        }
      }

      { // Similar check not applicable for  _buildForwardTimeline
        auto iterDelta = _deltaEntries.begin();
        t_versionDate firstDeltaChangeDate = iterDelta->first;
        if( startDate >= firstDeltaChangeDate )
        {
          std::ostringstream eoss;
          eoss << "ERROR(3) : failure in _VersionedObjectBuilderBase<VDT, MT...>::_buildReverseTimeline() : startDate[";
          eoss << startDate << "] should be less than first-changeDate[" << firstDeltaChangeDate << "]" << std::endl;
          eoss << "DataSet :: ";
          _deltaEntries.begin()->second.toCSV(eoss);
          toStr("VersionObjectBuilder :: ", eoss);
          throw FirstDeltaChange_Before_StartDate_Timeline_exception(eoss.str());
        }
      }

      t_versionDate lastDeltaChangeDate = _deltaEntries.rbegin()->first;
      t_versionDate firstVersionDate = vo.getDatasetLedger().begin()->first;
      if( lastDeltaChangeDate != firstVersionDate )
      {
        std::ostringstream eoss;
        eoss << "ERROR(4) : failure in _VersionedObjectBuilderBase<VDT, MT...>::_buildReverseTimeline() : last-changeDate[";
        eoss << lastDeltaChangeDate << "] should be equal to first-VersionDate[" << vo.getDatasetLedger().begin()->first << "]" << std::endl;
        vo.toStr("VersionObject :: ", eoss);
        toStr("VersionObjectBuilder :: ", eoss);
        throw FirstVersion_Before_LastDeltaChange_ReverseTimeline_exception(eoss.str());
      }
      //std::cout << eoss.str();

      std::array <bool, std::tuple_size_v< t_record > > hitheroProcessedElements;
      for(size_t i = 0; i < hitheroProcessedElements.size(); ++i)
      {
        hitheroProcessedElements[i] = false;
      }

      t_metaData metaData = (t_dataset::hasMetaData()) ?
                                  (*metaDataResetCloner):
                                  t_metaData();

      auto rIterDelta = _deltaEntries.rbegin();
      t_versionDate futureDeltaChangeDate = rIterDelta->first;
      typename t_versionedObject::t_datasetLedger::const_iterator
          ledgerIter = vo.getVersionAt(futureDeltaChangeDate);
      t_record record { ledgerIter->second.getRecord() };

      while( rIterDelta != _deltaEntries.rend() )
      {
        /*
        std::cout << "deltaEntryDate[" << rIterDelta->first << " =?= futureDeltaChangeDate(" << futureDeltaChangeDate << ") " << std::flush;
        if( vo.getVersionAt(rIterDelta->first) != vo.getDatasetLedger().cend() )
        {
          std::cout << "]->versionDate[" << std::flush;
          //ledgerIter = vo.getVersionAt(rIterDelta->first);
          ////lastVersion        = &(ledgerIter->second);
          //record = ledgerIter->second.getRecord();
          std::cout << vo.getVersionAt(rIterDelta->first)->first << std::flush;
        }
        std::cout << "] -> data{" << ledgerIter->second.toCSV() 
                  << "} << deltaChange{" << rIterDelta->second.toCSV() << "}" << std::endl;
        */

        /*
        if(rIterDelta->second.isSnapShot())
        {
          std::ostringstream eoss;
          eoss << "ERROR(5) : in function _VersionedObjectBuilderBase<VDT, MT...>::_buildReverseTimeline() : ";
          eoss << "is applicable only for delta-change and NOT for snapshotChange. {";
          toCSV(eoss);
          eoss << "}" << std::endl;
          throw SnapshotChange_ReverseTimelineBuild_exception(eoss.str());
        }
        */

        if(futureDeltaChangeDate != rIterDelta->first)
        {
          t_dataset dataset = _datasetFactory(&metaData, record);
          //std::cout << "vo.insertVersion -> versionDate: " << rIterDelta->first << ":" << dataset.toCSV() << std::endl;
          vo.insertVersion( rIterDelta->first, dataset );
          if constexpr ( t_dataset::hasMetaData() )
          {
            metaData = (*metaDataResetCloner); // reset of metaData values
          }
          if( vo.getVersionAt(rIterDelta->first) != vo.getDatasetLedger().cend() )
          {
            ledgerIter = vo.getVersionAt(rIterDelta->first);
            ////lastVersion        = &(ledgerIter->second);
            record = ledgerIter->second.getRecord();
          }

          // two consecutive changes , but on different dates.
          // Hence treat it as new Record with no hitheroProcessedElements.
          for(size_t i = 0; i < hitheroProcessedElements.size(); ++i)
          {
            hitheroProcessedElements[i] = false;
          }
        }

        if constexpr ( t_dataset::hasMetaData() )
        {
          metaData.appendMetaInfo(rIterDelta->second.getMetaData());
        }

        try {
          // 'getRecord()' is called with VALIDATE=true
          rIterDelta->second.getPreviousRecord_deltaChange(record, hitheroProcessedElements);
        } catch (const std::exception& err) {
          std::ostringstream eoss;
          eoss << "ERROR(5) : failure in _VersionedObjectBuilderBase<VDT, MT...>::_buildReverseTimeline() : "
               << err.what() << std::endl;
          toStr("VersionObjectBuilder :: ", eoss);
          eoss << "DataSet :: ";
          ledgerIter->second.toCSV(eoss);
          throw std::invalid_argument(eoss.str());
        }

        futureDeltaChangeDate = rIterDelta->first;
        ++rIterDelta;
      }

      // Insert first Version
      t_dataset dataset = _datasetFactory(&metaData, record);
      //std::cout << "vo.insertVersion -> versionDate: " << startDate << ":" << dataset.toCSV() << std::endl;
      vo.insertVersion( startDate, dataset );

      _deltaEntries.clear();
    }

    bool _buildBiDirectionalTimeline( // with filled VersionObject
                  const std::vector<t_versionDate>& startDates,
                  t_versionedObject& vo,
                  const t_metaData* const metaDataResetClonerReverse = nullptr, // nullptr when MetaData is NOT used
                  const t_metaData* const metaDataResetClonerForward = nullptr) // nullptr when MetaData is NOT used
    {
      t_versionedObject initialStateVOcopy{vo};

      typename t_datasetLedger::const_iterator   iterISVOcopy     = initialStateVOcopy.getDatasetLedger().cbegin();
      typename t_deltaEntriesMap::const_iterator iterDeltaEntries = _deltaEntries.cbegin();


      size_t buildReverseIter = 0;

      while(iterISVOcopy != initialStateVOcopy.getDatasetLedger().cend() &&
            iterDeltaEntries != _deltaEntries.cend())
      {
        if( iterDeltaEntries->first <= iterISVOcopy->first )   // Reverse Timeline
        {
          _VersionedObjectBuilderBase<VDT, MT...> tempVOB{};

          while(    iterDeltaEntries != _deltaEntries.cend()
                &&  iterDeltaEntries->first <= iterISVOcopy->first
                // if ChangeDirection is not REVERSE, let it be thrown as exception from call to _buildReverseTimeline()
                //&&  iterDeltaEntries->second.getApplicableChangeDirection() == ApplicableChangeDirection::REVERSE
               )
          {
            tempVOB.insertDeltaVersion(iterDeltaEntries->first, iterDeltaEntries->second);
            ++iterDeltaEntries;
          }

          if(buildReverseIter >= startDates.size())
          {
            std::ostringstream eoss;
            eoss << "ERROR(1) : failure in _VersionedObjectBuilderBase<VDT, MT...>::_buildBiDirectionalTimeline() : list of startDates[";
            for(auto sd : startDates) eoss << sd << ",";
            eoss << "] has insufficient dates." << std::endl;
            vo.toStr("VersionObject :: ", eoss);
            toStr("VersionObjectBuilder :: ", eoss);
            throw std::out_of_range(eoss.str());
          }

          t_versionedObject reverseBuildVO{};
          reverseBuildVO.insertVersion(iterISVOcopy->first, iterISVOcopy->second);

          tempVOB._buildReverseTimeline(startDates[buildReverseIter++], // use 0, then increment to 1
                                        reverseBuildVO,
                                        metaDataResetClonerReverse);

          size_t count = 1; // count is 1 NOT zero, as the last valid element in 'reverseBuildVO' is seed,
                            // and already present in 'vo'
          for(auto  partialResultIter  = reverseBuildVO.getDatasetLedger().cbegin();
                    count < reverseBuildVO.getDatasetLedger().size();
                  ++partialResultIter, ++count)
          {
            vo.insertVersion(partialResultIter->first, partialResultIter->second);
          }
        } // else ::: DONOT club with if(iterDeltaEntries->first > iterISVOcopy->first) ::: keep these 2 blocks disjoint

        if( iterDeltaEntries != _deltaEntries.cend()  &&
            iterDeltaEntries->first > iterISVOcopy->first)   // Forward Timeline
        {
          _VersionedObjectBuilderBase<VDT, MT...> tempVOB{};

          auto iterNextVO = iterISVOcopy;
          ++iterNextVO;
          while(    iterDeltaEntries != _deltaEntries.cend()
                &&  iterDeltaEntries->first > iterISVOcopy->first
                // If ChangeDirection is becomes REVERSE, we stop loading it here in _buildForwardTimeline() .
                // In next iteration of iterISVOcopy, it gets picked up by  _buildReverseTimeline() .
                &&  iterDeltaEntries->second.getApplicableChangeDirection() == ApplicableChangeDirection::FORWARD
                &&
                  (iterNextVO == initialStateVOcopy.getDatasetLedger().cend() ||
                   iterDeltaEntries->first < iterNextVO->first)
               )
          {
            tempVOB.insertDeltaVersion(iterDeltaEntries->first, iterDeltaEntries->second);
            ++iterDeltaEntries;
          }

          t_versionedObject forwardBuildVO{};
          forwardBuildVO.insertVersion(iterISVOcopy->first, iterISVOcopy->second);

          tempVOB._buildForwardTimeline(forwardBuildVO, metaDataResetClonerForward);

          auto  partialResultIter  = forwardBuildVO.getDatasetLedger().cbegin();
          ++partialResultIter; // skip the seed
          for(  ;
                partialResultIter != forwardBuildVO.getDatasetLedger().cend();
              ++partialResultIter)
          {
            vo.insertVersion(partialResultIter->first, partialResultIter->second);
          }
        }

        ++iterISVOcopy;
      }

      return (iterDeltaEntries != _deltaEntries.cend());
    }

  public:
    inline bool insertDeltaVersion(const t_versionDate& forDate,
                                   const ChangesInDataSet<MT...>& chgEntry)
    {
      auto range = _deltaEntries.equal_range(forDate);
      for (auto iter = range.first; iter != range.second; ++iter)
      {
        if( iter->second == chgEntry )
        {
          return false;  // Same Change Entry is available, no need to insert again
        }
      }

      [[maybe_unused]] const auto iter = _deltaEntries.emplace(forDate, chgEntry);
      //return true;
      return iter != _deltaEntries.end();
    }

    inline void removeHistoricalDeltaVersionArtifacts(
                    const t_versionedObject& historicalVO,
                    std::set<t_versionDate>& datesDeltaChangesExistOrg,
                    std::set<t_versionDate>& datesDeltaChangesMismatch)
    {
      std::set<t_versionDate> datesDeltaChangesExist;
      typename t_deltaEntriesMap::const_iterator iterDelta = _deltaEntries.cbegin(); // multimap

      const t_datasetLedger& datasetLedger = historicalVO.getDatasetLedger();
      while( iterDelta != _deltaEntries.cend() )
      {
        t_versionDate deltaChangeDate = iterDelta->first;
        typename t_datasetLedger::const_iterator iterDatasetLedger = datasetLedger.find(deltaChangeDate);
        if( iterDatasetLedger != datasetLedger.cend() )
        {
          if(iterDelta->second.isSnapShot()) 
          {
            if(iterDelta->second.isSnapshotChgValueEqual(iterDatasetLedger->second.getRecord()))
            {
                  //std::cout << "#### PATHTEST : SNAPSHOT : insert : " << deltaChangeDate << std::endl;
              datesDeltaChangesExist.insert(deltaChangeDate);
            } else {
                  //std::cout << "#### PATHTEST : SNAPSHOT : erase : " << deltaChangeDate << std::endl;
              // since _deltaEntries is multi-map, possible that 'datesDeltaChangesExist'
              // has same 'deltaChangeDate' inserted on previous iteration loop
              datesDeltaChangesExist.erase(deltaChangeDate);
              datesDeltaChangesMismatch.insert(deltaChangeDate);
              return;
            }
          } else {  // NOT a snapshot
            if(iterDelta->second.getApplicableChangeDirection() == ApplicableChangeDirection::FORWARD)
            {
              typename t_datasetLedger::const_iterator iterNextDatasetLedger = iterDatasetLedger;
              ++iterNextDatasetLedger;
              if( iterNextDatasetLedger != datasetLedger.cend() )
              {
                t_versionDate deltaChangeNextDate = iterNextDatasetLedger->first;
                if( iterDelta->second.isNextChgValueEqual_deltaChange(iterNextDatasetLedger->second.getRecord()) &&
                    iterDelta->second.isPreviousChgValueEqual_deltaChange(iterDatasetLedger->second.getRecord())
                  )
                {
                  //std::cout << "#### PATHTEST : FORWARD : insert : " << deltaChangeNextDate << std::endl;
                  datesDeltaChangesExist.insert(deltaChangeNextDate);
                } else {
                  //std::cout << "#### PATHTEST : FORWARD : erase : " << deltaChangeNextDate << std::endl;
                  // since _deltaEntries is multi-map, possible that 'datesDeltaChangesExist'
                  // has same 'deltaChangeNextDate' inserted on previous iteration loop
                  datesDeltaChangesExist.erase(deltaChangeNextDate);
                  datesDeltaChangesMismatch.insert(deltaChangeNextDate);
                  return;
                }
              } else { // last iterator
                // do nothing as it's probably new delta that needs to be applied later
                  //std::cout << "#### PATHTEST : FORWARD : donothing : " << deltaChangeDate << std::endl;
              }
            } else { // REVERSE direction
              if( iterDatasetLedger != datasetLedger.cbegin() )
              {
                typename t_datasetLedger::const_iterator iterPrevDatasetLedger = iterDatasetLedger;
                --iterPrevDatasetLedger;
                t_versionDate deltaChangePrevDate = iterPrevDatasetLedger->first;
                if( iterDelta->second.isNextChgValueEqual_deltaChange(iterDatasetLedger->second.getRecord()) &&
                    iterDelta->second.isPreviousChgValueEqual_deltaChange(iterPrevDatasetLedger->second.getRecord())
                  )
                {
                  //std::cout << "#### PATHTEST : REVERSE : insert : " << deltaChangePrevDate << std::endl;
                  datesDeltaChangesExist.insert(deltaChangePrevDate);
                } else {
                  //std::cout << "#### PATHTEST : REVERSE : erase : " << deltaChangePrevDate << std::endl;
                  // since _deltaEntries is multi-map, possible that 'datesDeltaChangesExist'
                  // has same 'deltaChangePrevDate' inserted on previous iteration loop
                  datesDeltaChangesExist.erase(deltaChangePrevDate);
                  datesDeltaChangesMismatch.insert(deltaChangePrevDate);
                  return;
                }
              } else {  // first iterator
                // do nothing as it's probably new delta that needs to be applied later
                  //std::cout << "#### PATHTEST : REVERSE : donothing : " << deltaChangeDate << std::endl;
              }
            }
          }
        } else {
          // do nothing as it's probably new delta that needs to be applied later
                  //std::cout << "#### PATHTEST : No DataSet Found : donothing : " << deltaChangeDate << std::endl;
        }
        ++iterDelta;
      }

      //remove existing&matching deltaChanges from multimap
      for(auto matchedDeltaChangeDate : datesDeltaChangesExist)
      {
        _deltaEntries.erase(matchedDeltaChangeDate);
      }
      datesDeltaChangesExistOrg.merge(datesDeltaChangesExist);
    }

    inline void toCSV(const std::string& prefix, std::ostream& oss) const
    {
      for(auto iter : _deltaEntries)
      {
        oss << prefix << iter.first << ",";
        iter.second.toCSV(oss);
        oss << std::endl;
      }
    }

    inline void toCSV(std::ostream& oss) const
    {
      for(auto iter : _deltaEntries)
      {
        oss << iter.first << ",";
        iter.second.toCSV(oss);
        oss << std::endl;
      }
    }

    inline void toStr(const std::string& prefix, std::ostream& oss) const
    {
      for(auto iter : _deltaEntries)
      {
        oss << prefix << "versionDate=" << iter.first << ", deltaEntry = {";
        iter.second.toCSV(oss);
        oss << "}" << std::endl;
      }
    }

    inline void toStr(std::ostream& oss) const
    {
      for(auto iter : _deltaEntries)
      {
        oss << "versionDate=" << iter.first << ", deltaEntry = {";
        iter.second.toCSV(oss);
        oss << "}" << std::endl;
      }
    }

    inline const t_deltaEntriesMap& getDeltaChangeMap() const { return _deltaEntries; }
  };

  template <typename VDT, typename M, typename ... T>
  class VersionedObjectBuilder;

  template <typename VDT, c_MetaData M, typename ... T>
  class VersionedObjectBuilder<VDT, M, T...> : public _VersionedObjectBuilderBase<VDT, M, T...>
  {
  public:
    using t_versionDate      = VDT;
    using t_versionedObject  = VersionedObject<VDT, M, T...>;
    using t_dataset          = DataSet<M, T...>;
    using t_record           = typename t_dataset::t_record;
    using t_metaData         = typename t_dataset::t_metaData;

    VersionedObjectBuilder() : _VersionedObjectBuilderBase<VDT, M, T...>() {}
    virtual ~VersionedObjectBuilder() {}

    inline void buildForwardTimeline( // with filled VersionObject
            VersionedObject<VDT, M, T...>&  vo,
            const t_metaData& metaDataResetCloner) // when MetaData is used
    {
      this->_buildForwardTimeline(vo, &metaDataResetCloner);
    }

    inline void buildReverseTimeline( // with filled VersionObject
            const t_versionDate& startDate,
            VersionedObject<VDT, M, T...>& vo,
            const t_metaData& metaDataResetCloner) // when MetaData is used
    {
      this->_buildReverseTimeline(startDate, vo, &metaDataResetCloner);
    }


    inline bool buildBiDirectionalTimeline( // with filled VersionObject
            const std::vector<t_versionDate>& startDates,
            VersionedObject<VDT, M, T...>& vo,
            const t_metaData& metaDataResetClonerReverse, // when MetaData is used
            const t_metaData& metaDataResetClonerForward) // when MetaData is used
    {
      return this->_buildBiDirectionalTimeline(startDates, vo,
                            &metaDataResetClonerReverse, &metaDataResetClonerForward);
    }
  };


  template <typename VDT, c_noMetaData T1, typename ... TR>
  class VersionedObjectBuilder<VDT, T1, TR...> : public _VersionedObjectBuilderBase<VDT, T1, TR...>
  {
  public:
    using t_versionDate      = VDT;
    using t_versionedObject  = VersionedObject<T1, TR...>;
    using t_dataset          = DataSet<T1, TR...>;
    using t_record           = typename t_dataset::t_record;
    using t_metaData         = typename t_dataset::t_metaData;

    VersionedObjectBuilder() : _VersionedObjectBuilderBase<VDT, T1, TR...>() {}
    virtual ~VersionedObjectBuilder() {}

    inline void buildForwardTimeline( // with filled VersionObject
            VersionedObject<VDT, T1, TR...>& vo) // when MetaData is NOT used
    {
      this->_buildForwardTimeline(vo);
    }

    inline void buildReverseTimeline(
            const t_versionDate& startDate,
            VersionedObject<VDT, T1, TR...>& vo) // when MetaData is NOT used
    {
      this->_buildReverseTimeline(startDate, vo);
    }

    inline bool buildBiDirectionalTimeline( // with filled VersionObject
            const std::vector<t_versionDate>& startDates,
            VersionedObject<VDT, T1, TR...>& vo) // when MetaData is NOT used
    {
      return this->_buildBiDirectionalTimeline(startDates, vo);
    }
  };
} }  //  datastructure::versionedObject
