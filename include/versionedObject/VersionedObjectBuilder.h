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

  using Change_Before_Start_Timeline_exception  =  VO_exception<1>;

  template <typename VDT, typename ... MT>
  class _VersionedObjectBuilderBase
  {
  protected:
    using t_versionDate      = VDT;
    using t_versionedObject  = VersionedObject<VDT, MT...>;
    using t_dataset          = DataSet<MT...>;
    using t_record           = typename t_dataset::t_record;
    using t_metaData         = typename t_dataset::t_metaData;

    std::multimap < t_versionDate, ChangesInDataSet<MT...> >  _deltaEntries;

    _VersionedObjectBuilderBase() : _deltaEntries() {}

    void _buildForwardTimeline(
                  const t_versionDate& startDate,
                  const t_dataset& firstVersion,
                  t_versionedObject& vo,
                  const t_metaData* const metaDataResetCloner = nullptr) // nullptr when MetaData is NOT used
    {
      vo.insertVersion(startDate,firstVersion);

      if(_deltaEntries.empty())
      {
        return;
      }

      if( startDate >= (_deltaEntries.begin()->first) )
      {
        std::ostringstream eoss;
        eoss << "ERROR(1) : failure in _VersionedObjectBuilderBase<VDT, MT...>::_buildForwardTimeline() : startDate[";
        eoss << startDate << "] should be less than first-changeDate[" << _deltaEntries.begin()->first << "]" << std::endl;
        firstVersion.toCSV(eoss);
        toCSV(eoss);
        throw Change_Before_Start_Timeline_exception(eoss.str());
      }

      [[maybe_unused]] t_metaData metaData = (t_dataset::hasMetaData()) ?
                                                  (*metaDataResetCloner):
                                                  t_metaData();

      std::array <bool, std::tuple_size_v< t_record > > hitheroProcessedElements;
      for(size_t i = 0; i < hitheroProcessedElements.size(); ++i)
      {
        hitheroProcessedElements[i] = false;
      }
      t_versionDate nextRecordDate = _deltaEntries.begin()->first;
      t_record record { firstVersion.getRecord() };
      //VERSIONEDOBJECT_DEBUG_LOG("FIRST record=" << converter::ConvertFromTuple<MT...>::ToStr(record));

      for(auto iterDelta : _deltaEntries)
      {
        if(nextRecordDate != iterDelta.first)
        {
          if constexpr ( t_dataset::hasMetaData() )
          {
            vo.insertVersion(nextRecordDate, t_dataset(metaData, record) );
            metaData = (*metaDataResetCloner); // reset of metaData values
          } else {
            vo.insertVersion(nextRecordDate, t_dataset(record) );
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
          metaData.appendMetaInfo(iterDelta.second.getMetaData());
        }

        try {
          // 'getRecord()' is called with VALIDATE=true
          iterDelta.second.getLatestRecord(record, hitheroProcessedElements);
          //VERSIONEDOBJECT_DEBUG_LOG("after delta-change record=" << converter::ConvertFromTuple<MT...>::ToStr(record));
        } catch (const std::exception& err) {
          std::ostringstream eoss;
          eoss << "ERROR(2) : failure in _VersionedObjectBuilderBase<VDT, MT...>::_buildForwardTimeline() : "
               << err.what() << std::endl;
          firstVersion.toCSV(eoss);
          toCSV(eoss);
          throw std::invalid_argument(eoss.str());
        }

        nextRecordDate = iterDelta.first;
      }

      if constexpr ( t_dataset::hasMetaData() )
      {
        vo.insertVersion(nextRecordDate, t_dataset(metaData, record) );
      } else {
        vo.insertVersion(nextRecordDate, t_dataset(record) );
      }
      //VERSIONEDOBJECT_DEBUG_LOG("LAST change record=" << converter::ConvertFromTuple<MT...>::ToStr(record));
      _deltaEntries.clear();
    }

    void _buildReverseTimeline(
                  const t_versionDate& startDate,
                  const t_dataset& lastVersion,
                  t_versionedObject& vo,
                  const t_metaData* const metaDataResetCloner = nullptr) // nullptr when MetaData is NOT used
    {

      if(_deltaEntries.empty())
      {
        vo.insertVersion(startDate, lastVersion);
        return;
      }

      if( startDate >= (_deltaEntries.begin()->first) )
      {
        std::ostringstream eoss;
        eoss << "ERROR(1) : failure in _VersionedObjectBuilderBase<VDT, MT...>::_buildReverseTimeline() : startDate[";
        eoss << startDate << "] should be less than first-changeDate[" << _deltaEntries.begin()->first << "]" << std::endl;
        toCSV(eoss);
        lastVersion.toCSV(eoss);
        throw Change_Before_Start_Timeline_exception(eoss.str());
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
      auto rIterDelta = _deltaEntries.rbegin();
      t_versionDate previousRecordDate = rIterDelta->first;
      t_record record { lastVersion.getRecord() };

      while( rIterDelta != _deltaEntries.rend() )
      {
        if(previousRecordDate != rIterDelta->first)
        {
          if constexpr ( t_dataset::hasMetaData() )
          {
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
          eoss << "ERROR(2) : failure in _VersionedObjectBuilderBase<VDT, MT...>::_buildReverseTimeline() : "
               << err.what() << std::endl;
          toCSV(eoss);
          lastVersion.toCSV(eoss);
          throw std::invalid_argument(eoss.str());
        }

        previousRecordDate = rIterDelta->first;
        ++rIterDelta;
      }

      if constexpr ( t_dataset::hasMetaData() )
      {
        vo.insertVersion(startDate, t_dataset(metaData, record) );
      } else {
        vo.insertVersion(startDate, t_dataset(record) );
      }
      _deltaEntries.clear();
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

    inline void buildForwardTimeline(
            const t_versionDate& startDate,
            const t_dataset& firstVersion,
            VersionedObject<VDT, M, T...>&  vo,
            const t_metaData& metaDataResetCloner) // when MetaData is used
    {
      return this->_buildForwardTimeline(startDate, firstVersion, vo, &metaDataResetCloner);
    }

    inline void buildReverseTimeline(
            const t_versionDate& startDate,
            const t_dataset& lastVersion,
            VersionedObject<VDT, M, T...>& vo,
            const t_metaData& metaDataResetCloner) // when MetaData is used
    {
      return this->_buildReverseTimeline(startDate, lastVersion, vo, &metaDataResetCloner);
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

    inline void buildForwardTimeline(
            const t_versionDate& startDate,
            const t_dataset& firstVersion,
            VersionedObject<VDT, T1, TR...>& vo) // when MetaData is NOT used
    {
      return this->_buildForwardTimeline(startDate, firstVersion, vo);
    }

    inline void buildReverseTimeline(
            const t_versionDate& startDate,
            const t_dataset& lastVersion,
            VersionedObject<VDT, T1, TR...>& vo) // when MetaData is NOT used
    {
      return this->_buildReverseTimeline(startDate, lastVersion, vo);
    }
  };

} }  //  datastructure::versionedObject
