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

#include <versionedObject/VersionedObject.h>
#include <versionedObject/ChangesInDataSet.h>


namespace datastructure { namespace versionedObject
{

  using FirstDeltaChange_Before_StartDate_Timeline_exception             =  VO_exception<1>;
  using Empty_VersionObject_exception                                    =  VO_exception<4>;
  using FirstVersion_Before_LastDeltaChange_ReverseTimeline_exception    =  VO_exception<5>;
  using LastVersion_After_FirstDeltaChange_ForwardTimeline_exception     =  VO_exception<6>;
  using StartDate_After_FirstVersion_ReverseTimeline_exception           =  VO_exception<7>;
  using Unexpected_ApplicableChangeDirection_exception                   =  VO_exception<8>;

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

    t_deltaEntriesMap  _deltaEntries;

    _VersionedObjectBuilderBase() : _deltaEntries() {}
    virtual ~_VersionedObjectBuilderBase()
    {
      _deltaEntries.clear();
    }

    void _buildForwardTimeline( // with dataSet
                  const t_versionDate& startDate,
                  const t_dataset& firstVersion,
                  t_versionedObject& vo,
                  const t_metaData* const metaDataResetCloner = nullptr) // nullptr when MetaData is NOT used
    {
      /*
      _buildForwardTimeline
      vo{
        optional[ Version Objects, timeline ... ]
        { startDate, firstVersion },
      }
      Expected :: startDate < first-ChangeDelta-date
      _deltaEntries{
        first ChangeDelta Version,
        remaining Change Delta Versions, timeline ...
        last-ChangeDelta version
      }
      */

      if( vo.getDatasetLedger().size() > 0 &&
          startDate >= (vo.getDatasetLedger().rbegin()->first) )
      {
        std::ostringstream eoss;
        eoss << "ERROR(1) : failure in _VersionedObjectBuilderBase<VDT, MT...>::_buildReverseTimeline(4) : startDate[";
        eoss << startDate << "] should be more than last-VersionDate[" << vo.getDatasetLedger().rbegin()->first << "]" << std::endl;
        vo.toStr("VersionObject :: ", eoss);
        toStr("VersionObjectBuilder :: ", eoss);
        throw FirstVersion_Before_LastDeltaChange_ReverseTimeline_exception(eoss.str());
      }

      vo.insertVersion(startDate, firstVersion);
      _buildForwardTimeline(vo, metaDataResetCloner);
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

      if(vo.getDatasetLedger().size() == 0)
      {
        std::ostringstream eoss;
        eoss << "ERROR(1) : failure in _VersionedObjectBuilderBase<VDT, MT...>::_buildForwardTimeline(2) : "
                "VersionObject cannot be empty." << std::endl;
        toStr("VersionObjectBuilder :: ", eoss);
        throw Empty_VersionObject_exception(eoss.str());
      }

      auto iterDelta = _deltaEntries.begin();
      t_versionDate nextRecordDate = iterDelta->first;
      if( nextRecordDate <= (vo.getDatasetLedger().rbegin()->first) )
      {
        std::ostringstream eoss;
        eoss << "ERROR(2) : failure in _VersionedObjectBuilderBase<VDT, MT...>::_buildForwardTimeline(2) : first-changeDate[";
        eoss << nextRecordDate << "] should be more than last-VersionDate[" << vo.getDatasetLedger().rbegin()->first << "]" << std::endl;
        vo.toStr("VersionObject :: ", eoss);
        toStr("VersionObjectBuilder :: ", eoss);
        throw LastVersion_After_FirstDeltaChange_ForwardTimeline_exception(eoss.str());
      }
      for( ; iterDelta != _deltaEntries.end(); ++iterDelta )
      {
        if(iterDelta->second.getApplicableChangeDirection() == ApplicableChangeDirection::REVERSE)
        {
          std::ostringstream eoss;
          eoss << "ERROR(3) : failure in _VersionedObjectBuilderBase<VDT, MT...>::_buildForwardTimeline(2) : for changeDate[";
          eoss << iterDelta->first << "] should NOT be marked as 'ApplicableChangeDirection::REVERSE'" << std::endl;
          toStr("VersionObjectBuilder :: ", eoss);
          vo.toStr("VersionObject :: ", eoss);
          throw Unexpected_ApplicableChangeDirection_exception(eoss.str());
        }
      }
      iterDelta = _deltaEntries.begin();

      [[maybe_unused]] t_metaData metaData = (t_dataset::hasMetaData()) ?
                                                  (*metaDataResetCloner):
                                                  t_metaData();

      std::array <bool, std::tuple_size_v< t_record > > hitheroProcessedElements;
      for(size_t i = 0; i < hitheroProcessedElements.size(); ++i)
      {
        hitheroProcessedElements[i] = false;
      }

      typename t_versionedObject::t_datasetLedger::const_iterator
          ledgerIter = vo.getVersionAt(iterDelta->first);
      //t_dataset     firstVersion        = ledgerIter->second;
      t_record record { ledgerIter->second.getRecord() };

      while( iterDelta != _deltaEntries.end() )
      {
        /*
        std::cout << "deltaEntryDate[" << iterDelta->first << " =?= nextRecordDate(" << nextRecordDate << ") " << std::flush;
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

        if(nextRecordDate != iterDelta->first)
        {
          if constexpr ( t_dataset::hasMetaData() )
          {
            //std::cout << "vo.insertVersion -> versionDate: " << rIterDelta->first << ":" << t_dataset(metaData, record).toCSV() << std::endl;
            vo.insertVersion(nextRecordDate, t_dataset(metaData, record) );
            metaData = (*metaDataResetCloner); // reset of metaData values
          } else {
            vo.insertVersion(nextRecordDate, t_dataset(record) );
          }
          if( vo.getVersionAt(iterDelta->first) != vo.getDatasetLedger().end() )
          {
            ledgerIter = vo.getVersionAt(iterDelta->first);
            ////firstVersion        = &(ledgerIter->second);
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
          eoss << "ERROR(4) : failure in _VersionedObjectBuilderBase<VDT, MT...>::_buildForwardTimeline(2) : "
               << err.what() << std::endl;
          eoss << "DataSet :: ";
          ledgerIter->second.toCSV(eoss);
          toStr("VersionObjectBuilder :: ", eoss);
          throw std::invalid_argument(eoss.str());
        }

        nextRecordDate = iterDelta->first;
        ++iterDelta;
      }

      // Insert last Version
      if constexpr ( t_dataset::hasMetaData() )
      {
        //std::cout << "vo.insertVersion -> versionDate: " << startDate << ":" << t_dataset(metaData, record).toCSV() << std::endl;
        vo.insertVersion(nextRecordDate, t_dataset(metaData, record) );
      } else {
        vo.insertVersion(nextRecordDate, t_dataset(record) );
      }
      //VERSIONEDOBJECT_DEBUG_LOG("LAST change record=" << converter::ConvertFromTuple<MT...>::ToStr(record));
      _deltaEntries.clear();
    }


    void _buildReverseTimeline( // with DataSet
                  const t_versionDate& startDate,
                  const t_dataset& lastVersion,
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
        optional[ first-VersionObject
                  Version Objects, timeline ... ]
      }
      */

      if(_deltaEntries.empty())
      {
        if( vo.getDatasetLedger().size() > 0 &&
            startDate >= (vo.getDatasetLedger().begin()->first) )
        {
          std::ostringstream eoss;
          eoss << "ERROR(1) : failure in _VersionedObjectBuilderBase<VDT, MT...>::_buildReverseTimeline(4) : startDate[";
          eoss << startDate << "] should be less than first-VersionDate[" << vo.getDatasetLedger().begin()->first << "]" << std::endl;
          vo.toStr("VersionObject :: ", eoss);
          toStr("VersionObjectBuilder :: ", eoss);
          throw StartDate_After_FirstVersion_ReverseTimeline_exception(eoss.str());
        }
        vo.insertVersion(startDate, lastVersion);
        return;
      }

      if( startDate >= (_deltaEntries.begin()->first) )
      {
        std::ostringstream eoss;
        eoss << "ERROR(2) : failure in _VersionedObjectBuilderBase<VDT, MT...>::_buildReverseTimeline(4) : startDate[";
        eoss << startDate << "] should be less than first-changeDate[" << _deltaEntries.begin()->first << "]" << std::endl;
        toStr("VersionObjectBuilder :: ", eoss);
        eoss << "DataSet :: ";
        lastVersion.toCSV(eoss);
        throw FirstDeltaChange_Before_StartDate_Timeline_exception(eoss.str());
      }

      auto rIterDelta = _deltaEntries.rbegin();
      t_versionDate previousRecordDate = rIterDelta->first;
      if( vo.getDatasetLedger().size() > 0 &&
          previousRecordDate >= (vo.getDatasetLedger().begin()->first) )
      {
        std::ostringstream eoss;
        eoss << "ERROR(3) : failure in _VersionedObjectBuilderBase<VDT, MT...>::_buildReverseTimeline(4) : last-changeDate[";
        eoss << previousRecordDate << "] should be less than first-VersionDate[" << vo.getDatasetLedger().begin()->first << "]" << std::endl;
        vo.toStr("VersionObject :: ", eoss);
        toStr("VersionObjectBuilder :: ", eoss);
        throw FirstVersion_Before_LastDeltaChange_ReverseTimeline_exception(eoss.str());
      }

      for( auto iterDelta = _deltaEntries.begin(); iterDelta != _deltaEntries.end(); ++iterDelta )
      {
        if(iterDelta->second.getApplicableChangeDirection() == ApplicableChangeDirection::FORWARD)
        {
          std::ostringstream eoss;
          eoss << "ERROR(4) : failure in _VersionedObjectBuilderBase<VDT, MT...>::_buildReverseTimeline(4) : for changeDate[";
          eoss << iterDelta->first << "] should NOT be marked as 'ApplicableChangeDirection::FORWARD'" << std::endl;
          toStr("VersionObjectBuilder :: ", eoss);
          vo.toStr("VersionObject :: ", eoss);
          throw Unexpected_ApplicableChangeDirection_exception(eoss.str());
        }
      }

      vo.insertVersion(_deltaEntries.rbegin()->first, lastVersion);

      [[maybe_unused]] t_metaData metaData = (t_dataset::hasMetaData()) ?
                                                  (*metaDataResetCloner):
                                                  t_metaData();

      std::array <bool, std::tuple_size_v< t_record > > hitheroProcessedElements;
      for(size_t i = 0; i < hitheroProcessedElements.size(); ++i)
      {
        hitheroProcessedElements[i] = false;
      }
      t_record record { lastVersion.getRecord() };

      while( rIterDelta != _deltaEntries.rend() )
      {
        //std::cout << "deltaEntryDate[" << rIterDelta->first << " =?= previousRecordDate(" << previousRecordDate << ") " << std::flush;
        //std::cout << "] << deltaChange{" << rIterDelta->second.toCSV() << "}" << std::endl;
        if(previousRecordDate != rIterDelta->first)
        {
          if constexpr ( t_dataset::hasMetaData() )
          {
            //std::cout << "vo.insertVersion -> versionDate: " << rIterDelta->first << ":" << t_dataset(metaData, record).toCSV() << std::endl;
            vo.insertVersion(rIterDelta->first, t_dataset(metaData, record) );
            metaData = (*metaDataResetCloner); // reset of metaData values
          } else {
            vo.insertVersion(rIterDelta->first, t_dataset(record) );
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
          rIterDelta->second.getPreviousRecord(record, hitheroProcessedElements);
        } catch (const std::exception& err) {
          std::ostringstream eoss;
          eoss << "ERROR(5) : failure in _VersionedObjectBuilderBase<VDT, MT...>::_buildReverseTimeline(4) : "
               << err.what() << std::endl;
          toStr("VersionObjectBuilder :: ", eoss);
          eoss << "DataSet :: ";
          lastVersion.toCSV(eoss);
          throw std::invalid_argument(eoss.str());
        }

        previousRecordDate = rIterDelta->first;
        ++rIterDelta;
      }

      if constexpr ( t_dataset::hasMetaData() )
      {
        //std::cout << "vo.insertVersion -> versionDate: " << startDate << ":" << t_dataset(metaData, record).toCSV() << std::endl;
        vo.insertVersion(startDate, t_dataset(metaData, record) );
      } else {
        vo.insertVersion(startDate, t_dataset(record) );
      }
      _deltaEntries.clear();
    }

    void _buildReverseTimeline( // with filled VersionObject
                  const t_versionDate& startDate,
                  //const t_dataset& lastVersion,
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
        eoss << "ERROR(1) : failure in _VersionedObjectBuilderBase<VDT, MT...>::_buildReverseTimeline(3) : "
                "VersionObject cannot be empty." << std::endl;
        toStr("VersionObjectBuilder :: ", eoss);
        throw Empty_VersionObject_exception(eoss.str());
      }

      if( startDate >= (_deltaEntries.begin()->first) )
      {
        std::ostringstream eoss;
        eoss << "ERROR(2) : failure in _VersionedObjectBuilderBase<VDT, MT...>::_buildReverseTimeline(3) : startDate[";
        eoss << startDate << "] should be less than first-changeDate[" << _deltaEntries.begin()->first << "]" << std::endl;
        eoss << "DataSet :: ";
        _deltaEntries.begin()->second.toCSV(eoss);
        toStr("VersionObjectBuilder :: ", eoss);
        throw FirstDeltaChange_Before_StartDate_Timeline_exception(eoss.str());
      }

      auto rIterDelta = _deltaEntries.rbegin();
      t_versionDate previousRecordDate = rIterDelta->first;
      if( previousRecordDate != (vo.getDatasetLedger().begin()->first) )
      {
        std::ostringstream eoss;
        eoss << "ERROR(3) : failure in _VersionedObjectBuilderBase<VDT, MT...>::_buildReverseTimeline(3) : last-changeDate[";
        eoss << previousRecordDate << "] should be equal to first-VersionDate[" << vo.getDatasetLedger().begin()->first << "]" << std::endl;
        vo.toStr("VersionObject :: ", eoss);
        toStr("VersionObjectBuilder :: ", eoss);
        throw FirstVersion_Before_LastDeltaChange_ReverseTimeline_exception(eoss.str());
      }

      for( auto iterDelta = _deltaEntries.begin(); iterDelta != _deltaEntries.end(); ++iterDelta )
      {
        if(iterDelta->second.getApplicableChangeDirection() == ApplicableChangeDirection::FORWARD)
        {
          std::ostringstream eoss;
          eoss << "ERROR(4) : failure in _VersionedObjectBuilderBase<VDT, MT...>::_buildReverseTimeline(3) : for changeDate[";
          eoss << iterDelta->first << "] should NOT be marked as 'ApplicableChangeDirection::FORWARD'" << std::endl;
          toStr("VersionObjectBuilder :: ", eoss);
          vo.toStr("VersionObject :: ", eoss);
          throw Unexpected_ApplicableChangeDirection_exception(eoss.str());
        }
      }

      [[maybe_unused]] t_metaData metaData = (t_dataset::hasMetaData()) ?
                                                  (*metaDataResetCloner):
                                                  t_metaData();

      std::array <bool, std::tuple_size_v< t_record > > hitheroProcessedElements;
      for(size_t i = 0; i < hitheroProcessedElements.size(); ++i)
      {
        hitheroProcessedElements[i] = false;
      }

      typename t_versionedObject::t_datasetLedger::const_iterator
          ledgerIter = vo.getVersionAt(rIterDelta->first);
      //t_dataset     lastVersion        = ledgerIter->second;
      t_record record { ledgerIter->second.getRecord() };

      while( rIterDelta != _deltaEntries.rend() )
      {
        /*
        std::cout << "deltaEntryDate[" << rIterDelta->first << " =?= previousRecordDate(" << previousRecordDate << ") " << std::flush;
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

        if(previousRecordDate != rIterDelta->first)
        {
          if constexpr ( t_dataset::hasMetaData() )
          {
            //std::cout << "vo.insertVersion -> versionDate: " << rIterDelta->first << ":" << t_dataset(metaData, record).toCSV() << std::endl;
            vo.insertVersion(rIterDelta->first, t_dataset(metaData, record) );
            metaData = (*metaDataResetCloner); // reset of metaData values
          } else {
            vo.insertVersion(rIterDelta->first, t_dataset(record) );
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
          rIterDelta->second.getPreviousRecord(record, hitheroProcessedElements);
        } catch (const std::exception& err) {
          std::ostringstream eoss;
          eoss << "ERROR(5) : failure in _VersionedObjectBuilderBase<VDT, MT...>::_buildReverseTimeline(3) : "
               << err.what() << std::endl;
          toStr("VersionObjectBuilder :: ", eoss);
          eoss << "DataSet :: ";
          ledgerIter->second.toCSV(eoss);
          throw std::invalid_argument(eoss.str());
        }

        previousRecordDate = rIterDelta->first;
        ++rIterDelta;
      }

      // Insert first Version
      if constexpr ( t_dataset::hasMetaData() )
      {
        //std::cout << "vo.insertVersion -> versionDate: " << startDate << ":" << t_dataset(metaData, record).toCSV() << std::endl;
        vo.insertVersion(startDate, t_dataset(metaData, record) );
      } else {
        vo.insertVersion(startDate, t_dataset(record) );
      }
      _deltaEntries.clear();
    }

/*
    void _buildBiDirectionalTimeline( // with filled VersionObject
                  const t_versionDate& startDate,
                  //const t_dataset& lastVersion,
                  t_versionedObject& vo,
                  const t_metaData* const metaDataResetCloner = nullptr) // nullptr when MetaData is NOT used
    {
      t_versionedObject resultVO{};

      typename t_datasetLedger::const_iterator   iterVO           = vo.getDatasetLedger().cbegin();
      typename t_deltaEntriesMap::const_iterator iterDeltaEntries = _deltaEntries.cbegin();

      if( iterVO != vo.getDatasetLedger().cend() &&
          iterDeltaEntries != _deltaEntries.cend())
      {
        while(iterDeltaEntries != _deltaEntries.cend()  &&
              iterDeltaEntries->first < iterVO->first)
        {
          tempVOB.insertDeltaVersion(iterDeltaEntries->first, iterDeltaEntries->second);
          ++iterDeltaEntries;
        }
        resultVO.insertVersion(iterVO->first, iterVO->second);
        // TODO startDate applicable only on first call, on subsequent calls have to fgure out
        tempVOB._buildReverseTimeline(startDate, resultVO, metaDataResetCloner);
      }

      while(iterVO != vo.getDatasetLedger().cend() &&
            iterDeltaEntries != _deltaEntries.cend())
      {
        if(iterDeltaEntries->first > iterVO->first)
        {
          while(iterDeltaEntries != _deltaEntries.cend()  &&
                iterDeltaEntries->first > iterVO->first)
          {
            tempVOB.insertDeltaVersion(iterDeltaEntries->first, iterDeltaEntries->second);
            ++iterDeltaEntries;
          }
          tempVOB._buildForwardTimeline(startDate, resultVO, metaDataResetCloner);
        } else {
          c
        }
        ++iterVO;
        resultVO.insertVersion(iterVO->first, iterVO->second);
      }

    }
*/

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
      return true;
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

    inline const t_deltaEntriesMap& getContainer() const { return _deltaEntries; }
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

    inline void buildForwardTimeline( // with DataSet
            const t_versionDate& startDate,
            const t_dataset& firstVersion,
            VersionedObject<VDT, M, T...>&  vo,
            const t_metaData& metaDataResetCloner) // when MetaData is used
    {
      this->_buildForwardTimeline(startDate, firstVersion, vo, &metaDataResetCloner);
    }

    inline void buildForwardTimeline( // with filled VersionObject
            VersionedObject<VDT, M, T...>&  vo,
            const t_metaData& metaDataResetCloner) // when MetaData is used
    {
      this->_buildForwardTimeline(vo, &metaDataResetCloner);
    }

    inline void buildReverseTimeline( // with DataSet
            const t_versionDate& startDate,
            const t_dataset& lastVersion,
            VersionedObject<VDT, M, T...>& vo,
            const t_metaData& metaDataResetCloner) // when MetaData is used
    {
      this->_buildReverseTimeline(startDate, lastVersion, vo, &metaDataResetCloner);
    }

    inline void buildReverseTimeline( // with filled VersionObject
            const t_versionDate& startDate,
            VersionedObject<VDT, M, T...>& vo,
            const t_metaData& metaDataResetCloner) // when MetaData is used
    {
      this->_buildReverseTimeline(startDate, vo, &metaDataResetCloner);
    }


    inline void buildBiDirectionalTimeline( // with filled VersionObject
            VersionedObject<VDT, M, T...>& vo,
            const t_metaData& metaDataResetCloner) // when MetaData is used
    {
      this->_buildBiDirectionalTimeline(vo, &metaDataResetCloner);
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

    inline void buildForwardTimeline( // with DataSet
            const t_versionDate& startDate,
            const t_dataset& firstVersion,
            VersionedObject<VDT, T1, TR...>& vo) // when MetaData is NOT used
    {
      this->_buildForwardTimeline(startDate, firstVersion, vo);
    }

    inline void buildForwardTimeline( // with filled VersionObject
            VersionedObject<VDT, T1, TR...>& vo) // when MetaData is NOT used
    {
      this->_buildForwardTimeline(vo);
    }

    inline void buildReverseTimeline( // with DataSet
            const t_versionDate& startDate,
            const t_dataset& lastVersion,
            VersionedObject<VDT, T1, TR...>& vo) // when MetaData is NOT used
    {
      this->_buildReverseTimeline(startDate, lastVersion, vo);
    }

    inline void buildReverseTimeline(
            const t_versionDate& startDate,
            VersionedObject<VDT, T1, TR...>& vo) // when MetaData is NOT used
    {
      this->_buildReverseTimeline(startDate, vo);
    }


    inline void buildBiDirectionalTimeline( // with filled VersionObject
            VersionedObject<VDT, T1, TR...>& vo) // when MetaData is NOT used
    {
      this->_buildBiDirectionalTimeline(vo);
    }

  };
} }  //  datastructure::versionedObject
