/*
 * VersionedObjectBuilder.h
 *
 * Version:  v1.0.0
 *
 * Copyright (C) 2023-2023 Gautam Dhar
 * All rights reserved.
 * 
 * VersionedObjectBuilder is private and NOT licensed for public use.
 */

#pragma once

#include <array>

#include <versionedObject/VersionedObject.h>


namespace versionedObject
{

  template <typename ... T>
  class _ChangesInDataSetBase
  {
  public:
    using t_record  = typename std::tuple<T ...>;

    _ChangesInDataSetBase() = delete;
    _ChangesInDataSetBase(_ChangesInDataSetBase const&) = default;
    _ChangesInDataSetBase& operator=(_ChangesInDataSetBase const&) = delete;
    bool operator==(_ChangesInDataSetBase const&) const = default;

    inline void getLatestRecord(t_record& updateRecord,
                                std::array <bool, sizeof...(T)>& hitheroProcessedElements) const
    {
      _getLatestValue<sizeof...(T) -1, true>(updateRecord, hitheroProcessedElements);
    }

    inline void getLatestRecord(t_record& updateRecord) const
    {
      std::array <bool, sizeof...(T)> hitheroProcessedElements;
      for(size_t i = 0; i < sizeof...(T); ++i)
      {
        hitheroProcessedElements[i] = false;
      }
      _getLatestValue<sizeof...(T) -1, false>(updateRecord, hitheroProcessedElements);
    }

    inline void getPreviousRecord(t_record& updateRecord,
                                  std::array <bool, sizeof...(T)>& hitheroProcessedElements) const
    {
      _getPreviousValue<sizeof...(T) -1, true>(updateRecord, hitheroProcessedElements);
    }

    inline void getPreviousRecord(t_record& updateRecord) const
    {
      std::array <bool, sizeof...(T)> hitheroProcessedElements;
      for(size_t i = 0; i < sizeof...(T); ++i)
      {
        hitheroProcessedElements[i] = false;
      }
      _getPreviousValue<sizeof...(T) -1, false>(updateRecord, hitheroProcessedElements);
    }

    inline void toCSV(std::ostream& oss) const
    {
      //oss << _source;
      _toCSV<0>(oss);
    }

    inline std::string toCSV() const
    {
      std::ostringstream oss;
      _ChangesInDataSetBase<T... >::toCSV(oss);
      return oss.str();
    }

  protected:
    _ChangesInDataSetBase( const std::array <bool, sizeof...(T)> modifiedElements,
                           const t_record& oldValues,
                           const t_record& newValues)
      : _modifiedElements(modifiedElements),
        _oldValues(oldValues),
        _newValues(newValues)
    {}

    template<size_t IDX>
    inline void _toCSV(std::ostream& oss) const
    {
      oss << ",";
      if( _modifiedElements.at(IDX) ) // check if element is marked for change
      {
        // NOTE :: will fail for types such as std::chrono::year_month_day
        oss << std::get<IDX>(_oldValues) << "->" << std::get<IDX>(_newValues);
      }

      if constexpr( IDX < (sizeof...(T)-1) )
      {
        _toCSV< ( (IDX < (sizeof...(T)-1)) ? (IDX+1) : (sizeof...(T)-1) ) >(oss);
      }
    }

    template<size_t IDX, bool VALIDATE>
    inline void _getLatestValue(t_record& updateRecord,
                                std::array <bool, sizeof...(T)>& hitheroProcessedElements) const
    {
      if( _modifiedElements.at(IDX) ) // check if element is marked for change
      {
        if constexpr(VALIDATE)
        {
          if( hitheroProcessedElements[IDX] ) // check if element was previously modified
          {
            std::ostringstream eoss;
            eoss << "ERROR(1) : in function _ChangesInDataSetBase<T ...>::_getLatestValue() : ";
            eoss << " at tuple-index[" << IDX << "] : updateRecord-value{" << std::get<IDX>(updateRecord);
            eoss << "} was previously modified." << std::endl;
            throw std::invalid_argument(eoss.str());
          }
          hitheroProcessedElements[IDX] = true;

          if( std::get<IDX>(updateRecord) != std::get<IDX>(_oldValues) )
          {
            std::ostringstream eoss;
            eoss << "ERROR(2) : in function _ChangesInDataSetBase<T ...>::_getLatestValue() : ";
            eoss << " at tuple-index[" << IDX << "] : updateRecord-value{" << std::get<IDX>(updateRecord);
            eoss << "} doesn't match with expected old-Value{" << std::get<IDX>(_oldValues) << "}" << std::endl;
            throw std::invalid_argument(eoss.str());
          }
        }

        std::get<IDX>(updateRecord) = std::get<IDX>(_newValues);
      }

      if constexpr( IDX > 0 )
      {
        // "((IDX>0)?(IDX-1):0)" eliminates infinite compile time looping,
        // and we don't have to define function specialization for _getLatestValue<0, VALIDATE>()
        _getLatestValue< ((IDX>0)?(IDX-1):0), VALIDATE >(updateRecord, hitheroProcessedElements);
      }
    }

    template<size_t IDX, bool VALIDATE>
    inline void _getPreviousValue(t_record& updateRecord,
                                  std::array <bool, sizeof...(T)>& hitheroProcessedElements) const
    {
      if( _modifiedElements.at(IDX) ) // check if element is marked for change
      {
        if constexpr(VALIDATE)
        {
          if( hitheroProcessedElements[IDX] ) // check if element was previously modified
          {
            std::ostringstream eoss;
            eoss << "ERROR(1) : in function _ChangesInDataSetBase<T ...>::_getPreviousValue() : ";
            eoss << " at tuple-index[" << IDX << "] : updateRecord-value{" << std::get<IDX>(updateRecord);
            eoss << "} was previously modified." << std::endl;
            throw std::invalid_argument(eoss.str());
          }
          hitheroProcessedElements[IDX] = true;

          if( std::get<IDX>(updateRecord) != std::get<IDX>(_newValues) )
          {
            std::ostringstream eoss;
            eoss << "ERROR(2) : in function _ChangesInDataSetBase<T ...>::_getPreviousValue() : ";
            eoss << " at tuple-index[" << IDX << "] : updateRecord-value{" << std::get<IDX>(updateRecord);
            eoss << "} doesn't match with expected old-Value{" << std::get<IDX>(_newValues) << "}" << std::endl;
            throw std::invalid_argument(eoss.str());
          }
        }

        std::get<IDX>(updateRecord) = std::get<IDX>(_oldValues);
      }

      if constexpr( IDX > 0 )
      {
        // "((IDX>0)?(IDX-1):0)" eliminates infinite compile time looping,
        // and we don't have to define function specialization for _getPreviousValue<0, VALIDATE>()
        _getPreviousValue< ((IDX>0)?(IDX-1):0), VALIDATE >(updateRecord, hitheroProcessedElements);
      }
    }

    const std::array <bool, sizeof...(T)> _modifiedElements;        // elements that has changed is indicated by 'true'
    const t_record _oldValues;                                      // value(s) of elements before change
    const t_record _newValues;                                      // value(s) of elements after  change
  };

  template <typename M, typename ... T>
  class ChangesInDataSet;

  template <c_MetaData M, typename ... T>
  class ChangesInDataSet<M, T...> : public _ChangesInDataSetBase<T...>
  {
  public:
    using t_record  = typename std::tuple<T ...>;

    ChangesInDataSet( const M& metaData,
                      const std::array <bool, sizeof...(T)> modifiedElements,
                      const t_record& oldValues,
                      const t_record& newValues)
      : _ChangesInDataSetBase<T...>(modifiedElements, oldValues, newValues),
        _metaData(metaData)
    {}

    ChangesInDataSet() = delete;
    ChangesInDataSet(ChangesInDataSet const&) = default;
    ChangesInDataSet& operator=(ChangesInDataSet const&) = delete;
    bool operator==(ChangesInDataSet const&) const = default;

    inline const M&           getMetaData() const { return _metaData; }

    inline void toCSV(std::ostream& oss) const
    {
      oss << _metaData.toCSV() << ",";

      // _toCSV<0>(oss);   #########  DOESNOT COMPILE : refer urls below
      //  https://stackoverflow.com/questions/9289859/calling-template-function-of-template-base-class
      //  https://stackoverflow.com/questions/610245/where-and-why-do-i-have-to-put-the-template-and-typename-keywords
      this->template _toCSV<0>(oss);
    }

    inline std::string toCSV() const
    {
      std::ostringstream oss;
      ChangesInDataSet<M, T...>::toCSV(oss);
      return oss.str();
    }

  private:
    const M           _metaData;     // metaData-id of a change instance
  };

  template <c_noMetaData T1, typename ... TR>
  class ChangesInDataSet<T1, TR...> : public _ChangesInDataSetBase<T1, TR...>
  {
  public:
    using t_record  = typename std::tuple<T1, TR...>;

    ChangesInDataSet( const std::array <bool, std::tuple_size_v<t_record>> modifiedElements,
                      const t_record& oldValues,
                      const t_record& newValues)
      : _ChangesInDataSetBase<T1, TR...>(modifiedElements, oldValues, newValues)
    {}

    ChangesInDataSet() = delete;
    ChangesInDataSet(ChangesInDataSet const&) = default;
    ChangesInDataSet& operator=(ChangesInDataSet const&) = delete;
    bool operator==(ChangesInDataSet const&) const = default;
  };

  template <typename ... MT>
  class _VersionedObjectBuilderBase
  {
  protected:
    using t_record   = typename DataSet<MT...>::t_record;
    using t_metaData = typename DataSet<MT...>::t_metaData;

    std::multimap < std::chrono::year_month_day, ChangesInDataSet<MT...> >  _deltaEntries;

    _VersionedObjectBuilderBase() : _deltaEntries() {}

    inline void _toCSV(std::ostream& oss) const
    {
      for(auto iter : _deltaEntries)
      {
        oss << VersionedObject<MT...>::_To_dbY(iter.first) << ",";
        iter.second.toCSV(oss);
        oss << std::endl;
      }
    }

    VersionedObject<MT...> _buildForwardTimeline(
                  const std::chrono::year_month_day& startDate,
                  const DataSet<MT...>& firstVersion,
                  const t_metaData* const metaDataResetCloner = nullptr) // nullptr when MetaData is NOT used
    {
      VersionedObject<MT...> vo;
      vo.insertVersion(startDate,firstVersion);

      if(_deltaEntries.empty())
      {
        return vo;
      }

      if( startDate >= (_deltaEntries.begin()->first) )
      {
        std::ostringstream eoss;
        eoss << "ERROR(1) : failure in _VersionedObjectBuilderBase<MT...>::buildForwardTimeline() : startDate[";
        eoss << VersionedObject<MT...>::_To_dbY(startDate) << "] should be less than first-changeDate[";
        eoss << VersionedObject<MT...>::_To_dbY(_deltaEntries.begin()->first) << std::endl;
        firstVersion.toCSV(eoss);
        _toCSV(eoss);
        throw std::invalid_argument(eoss.str());
      }

      [[maybe_unused]] t_metaData metaData = (DataSet<MT...>::hasMetaData()) ?
                                                  (*metaDataResetCloner):
                                                  t_metaData();

      std::array <bool, std::tuple_size_v< t_record > > hitheroProcessedElements;
      for(size_t i = 0; i < hitheroProcessedElements.size(); ++i)
      {
        hitheroProcessedElements[i] = false;
      }
      std::chrono::year_month_day nextRecordDate = _deltaEntries.begin()->first;
      t_record record { firstVersion.getRecord() };
      //VERSIONEDOBJECT_DEBUG_LOG("FIRST record=" << converter::ConvertFromTuple<MT...>::ToStr(record));

      for(auto iterDelta : _deltaEntries)
      {
        if(nextRecordDate != iterDelta.first)
        {
          if constexpr ( DataSet<MT...>::hasMetaData() )
          {
            vo.insertVersion(nextRecordDate, DataSet<MT...>(metaData, record) );
            metaData = (*metaDataResetCloner); // reset of metaData values
          } else {
            vo.insertVersion(nextRecordDate, DataSet<MT...>(record) );
          }

          // two consecutive changes , but on different dates.
          // Hence treat it as new Record with no hitheroProcessedElements.
          for(size_t i = 0; i < hitheroProcessedElements.size(); ++i)
          {
            hitheroProcessedElements[i] = false;
          }
        }

        if constexpr ( DataSet<MT...>::hasMetaData() ) {
          metaData.appendMetaInfo(iterDelta.second.getMetaData());
        }

        try {
          // 'getRecord()' is called with VALIDATE=true
          iterDelta.second.getLatestRecord(record, hitheroProcessedElements);
          //VERSIONEDOBJECT_DEBUG_LOG("after delta-change record=" << converter::ConvertFromTuple<MT...>::ToStr(record));
        } catch (const std::exception& err) {
          std::ostringstream eoss;
          eoss << "ERROR(2) : failure in _VersionedObjectBuilderBase<MT...>::buildForwardTimeline() : "
               << err.what() << std::endl;
          firstVersion.toCSV(eoss);
          _toCSV(eoss);
          throw std::invalid_argument(eoss.str());
        }

        nextRecordDate = iterDelta.first;
      }
      if constexpr ( DataSet<MT...>::hasMetaData() )
      {
        vo.insertVersion(nextRecordDate, DataSet<MT...>(metaData, record) );
      } else {
        vo.insertVersion(nextRecordDate, DataSet<MT...>(record) );
      }
      //VERSIONEDOBJECT_DEBUG_LOG("LAST change record=" << converter::ConvertFromTuple<MT...>::ToStr(record));
      _deltaEntries.clear();
      return vo;
    }

    VersionedObject<MT...> _buildReverseTimeline(
                  const std::chrono::year_month_day& startDate,
                  const DataSet<MT...>& lastVersion,
                  const t_metaData* const metaDataResetCloner = nullptr) // nullptr when MetaData is NOT used
    {
      VersionedObject<MT...> vo;

      if(_deltaEntries.empty())
      {
        vo.insertVersion(startDate, lastVersion);
        return vo;
      }

      if( startDate >= (_deltaEntries.begin()->first) )
      {
        std::ostringstream eoss;
        eoss << "ERROR(1) : failure in _VersionedObjectBuilderBase<MT...>::_buildReverseTimeline() : startDate[";
        eoss << VersionedObject<MT...>::_To_dbY(startDate) << "] should be less than first-changeDate[";
        eoss << VersionedObject<MT...>::_To_dbY(_deltaEntries.begin()->first) << std::endl;
        _toCSV(eoss);
        lastVersion.toCSV(eoss);
        throw std::invalid_argument(eoss.str());
      }

      vo.insertVersion(_deltaEntries.rbegin()->first, lastVersion);

      [[maybe_unused]] t_metaData metaData = (DataSet<MT...>::hasMetaData()) ?
                                                  (*metaDataResetCloner):
                                                  t_metaData();

      std::array <bool, std::tuple_size_v< t_record > > hitheroProcessedElements;
      for(size_t i = 0; i < hitheroProcessedElements.size(); ++i)
      {
        hitheroProcessedElements[i] = false;
      }
      auto rIterDelta = _deltaEntries.rbegin();
      std::chrono::year_month_day previousRecordDate = rIterDelta->first;
      t_record record { lastVersion.getRecord() };

      while( rIterDelta != _deltaEntries.rend() )
      {
        if(previousRecordDate != rIterDelta->first)
        {
          if constexpr ( DataSet<MT...>::hasMetaData() )
          {
            vo.insertVersion(rIterDelta->first, DataSet<MT...>(metaData, record) );
            metaData = (*metaDataResetCloner); // reset of metaData values
          } else {
            vo.insertVersion(rIterDelta->first, DataSet<MT...>(record) );
          }

          // two consecutive changes , but on different dates.
          // Hence treat it as new Record with no hitheroProcessedElements.
          for(size_t i = 0; i < hitheroProcessedElements.size(); ++i)
          {
            hitheroProcessedElements[i] = false;
          }
        }

        if constexpr ( DataSet<MT...>::hasMetaData() ) {
          metaData.appendMetaInfo(rIterDelta->second.getMetaData());
        }

        try {
          // 'getRecord()' is called with VALIDATE=true
          rIterDelta->second.getPreviousRecord(record, hitheroProcessedElements);
        } catch (const std::exception& err) {
          std::ostringstream eoss;
          eoss << "ERROR(2) : failure in _VersionedObjectBuilderBase<MT...>::_buildReverseTimeline() : "
               << err.what() << std::endl;
          _toCSV(eoss);
          lastVersion.toCSV(eoss);
          throw std::invalid_argument(eoss.str());
        }

        previousRecordDate = rIterDelta->first;
        ++rIterDelta;
      }
      if constexpr ( DataSet<MT...>::hasMetaData() )
      {
        vo.insertVersion(startDate, DataSet<MT...>(metaData, record) );
      } else {
        vo.insertVersion(startDate, DataSet<MT...>(record) );
      }
      _deltaEntries.clear();
      return vo;
    }

  public:
    inline bool insertDeltaVersion(const std::chrono::year_month_day& forDate,
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
  };

  template <typename M, typename ... T>
  class VersionedObjectBuilder;

  template <c_MetaData M, typename ... T>
  class VersionedObjectBuilder<M, T...> : public _VersionedObjectBuilderBase<M, T...>
  {
  public:
    using t_record   = typename DataSet<M, T...>::t_record;
    using t_metaData = typename DataSet<M, T...>::t_metaData;

  public:
    VersionedObjectBuilder() : _VersionedObjectBuilderBase<M, T...>() {}

    VersionedObject<M, T...> buildForwardTimeline(
            const std::chrono::year_month_day& startDate,
            const DataSet<M, T...>& firstVersion,
            const t_metaData& metaDataResetCloner) // when MetaData is used
    {
      return this->_buildForwardTimeline(startDate, firstVersion, &metaDataResetCloner);
    }

    VersionedObject<M, T...> buildReverseTimeline(
            const std::chrono::year_month_day& startDate,
            const DataSet<M, T...>& lastVersion,
            const t_metaData& metaDataResetCloner) // when MetaData is used
    {
      return this->_buildReverseTimeline(startDate, lastVersion, &metaDataResetCloner);
    }
  };

  template <c_noMetaData T1, typename ... TR>
  class VersionedObjectBuilder<T1, TR...> : public _VersionedObjectBuilderBase<T1, TR...>
  {
  public:
    using t_record   = typename DataSet<T1, TR...>::t_record;
    using t_metaData = typename DataSet<T1, TR...>::t_metaData;

  public:
    VersionedObjectBuilder() : _VersionedObjectBuilderBase<T1, TR...>() {}

    VersionedObject<T1, TR...> buildForwardTimeline(
            const std::chrono::year_month_day& startDate,
            const DataSet<T1, TR...>& firstVersion) // when MetaData is NOT used
    {
      return this->_buildForwardTimeline(startDate, firstVersion);
    }

    VersionedObject<T1, TR...> buildReverseTimeline(
            const std::chrono::year_month_day& startDate,
            const DataSet<T1, TR...>& lastVersion) // when MetaData is NOT used
    {
      return this->_buildReverseTimeline(startDate, lastVersion);
    }
  };

}
