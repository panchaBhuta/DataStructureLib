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
  class ChangesInDataSet
  {
  public:
    using t_record  = typename DataSet<T ...>::t_record;

    ChangesInDataSet( const std::string& source,
                      const std::array <bool, sizeof...(T)> modifiedElements,
                      const t_record& oldValues,
                      const t_record& newValues)
      : _source(source),
        _modifiedElements(modifiedElements),
        _oldValues(source, oldValues),
        _newValues(source, newValues)
    {}

    ChangesInDataSet() = delete;
    ChangesInDataSet(ChangesInDataSet const&) = default;
    ChangesInDataSet& operator=(ChangesInDataSet const&) = delete;
    bool operator==(ChangesInDataSet const&) const = default;

    inline const std::string& getSource() const { return _source; }

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
      oss << _source;
      _toCSV<0>(oss);
    }

    inline std::string toCSV() const
    {
      std::ostringstream oss;
      toCSV(oss);
      return oss.str();
    }

  private:
    template<size_t IDX>
    inline void _toCSV(std::ostream& oss) const
    {
      oss << ",";
      if( _modifiedElements.at(IDX) ) // check if element is marked for change
      {
        // NOTE :: will fail for types such as std::chrono::year_month_day
        oss << std::get<IDX>(_oldValues.getRecord()) << "->"
            << std::get<IDX>(_newValues.getRecord());
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
            eoss << "ERROR(1) : in function ChangesInDataSet<T ...>::_getLatestValue() : ";
            eoss << " at tuple-index[" << IDX << "] : updateRecord-value{" << std::get<IDX>(updateRecord);
            eoss << "} was previously modified." << std::endl;
            throw std::invalid_argument(eoss.str());
          }
          hitheroProcessedElements[IDX] = true;

          if( std::get<IDX>(updateRecord) != std::get<IDX>(_oldValues.getRecord()) )
          {
            std::ostringstream eoss;
            eoss << "ERROR(2) : in function ChangesInDataSet<T ...>::_getLatestValue() : ";
            eoss << " at tuple-index[" << IDX << "] : updateRecord-value{" << std::get<IDX>(updateRecord);
            eoss << "} doesn't match with expected old-Value{" << std::get<IDX>(_oldValues.getRecord()) << "}" << std::endl;
            throw std::invalid_argument(eoss.str());
          }
        }

        std::get<IDX>(updateRecord) = std::get<IDX>(_newValues.getRecord());
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
            eoss << "ERROR(1) : in function ChangesInDataSet<T ...>::_getPreviousValue() : ";
            eoss << " at tuple-index[" << IDX << "] : updateRecord-value{" << std::get<IDX>(updateRecord);
            eoss << "} was previously modified." << std::endl;
            throw std::invalid_argument(eoss.str());
          }
          hitheroProcessedElements[IDX] = true;

          if( std::get<IDX>(updateRecord) != std::get<IDX>(_newValues.getRecord()) )
          {
            std::ostringstream eoss;
            eoss << "ERROR(2) : in function ChangesInDataSet<T ...>::_getPreviousValue() : ";
            eoss << " at tuple-index[" << IDX << "] : updateRecord-value{" << std::get<IDX>(updateRecord);
            eoss << "} doesn't match with expected old-Value{" << std::get<IDX>(_newValues.getRecord()) << "}" << std::endl;
            throw std::invalid_argument(eoss.str());
          }
        }

        std::get<IDX>(updateRecord) = std::get<IDX>(_oldValues.getRecord());
      }

      if constexpr( IDX > 0 )
      {
        // "((IDX>0)?(IDX-1):0)" eliminates infinite compile time looping,
        // and we don't have to define function specialization for _getPreviousValue<0, VALIDATE>()
        _getPreviousValue< ((IDX>0)?(IDX-1):0), VALIDATE >(updateRecord, hitheroProcessedElements);
      }
    }

    const std::string _source;                                     // source-id of a change instance

    const std::array <bool, sizeof...(T)> _modifiedElements;       // elements that has changed is indicated by 'true'
    const DataSet<T...> _oldValues;                                      // value(s) of elements before change
    const DataSet<T...> _newValues;                                      // value(s) of elements after  change
  };

  template <typename ... T>
  class VersionedObjectBuilder
  {
  private:
    /*
      key 'year_month_day' is xOR in containers '_changeCompleteEntries' and '_deltaEntries'.
      That is, for a particular key of type 'year_month_day', either there can be 'one' entry in 
      '_changeCompleteEntries'  xOR 'one-or-more' entry(ies) in '_deltaEntries'.
      But there cannot be entries in containers '_changeCompleteEntries' and '_deltaEntries'
      for the same key 'year_month_day'.

      There can be multiple entries for the same date in '_deltaEntries',
      but those entries can collectively modify any individual tuple-element at the most once.
      Property 'ChangesInDataSet._modifiedElements' is used for tracking those changes.
    */
    std::multimap < std::chrono::year_month_day, ChangesInDataSet<T...> >  _deltaEntries;

    inline void _toCSV(std::ostream& oss) const
    {
      for(auto iter : _deltaEntries)
      {
        oss << VersionedObject<T...>::_To_dbY(iter.first) << ",";
        iter.second.toCSV(oss);
        oss << std::endl;
      }
    }

  public:
    using t_record  = typename DataSet<T ...>::t_record;

    VersionedObjectBuilder() : _deltaEntries() {}

    inline bool insertDeltaVersion(const std::chrono::year_month_day& forDate,
                                   const ChangesInDataSet<T...>& chgEntry)
    {
      auto range = _deltaEntries.equal_range(forDate);
      for (auto iter = range.first; iter != range.second; ++iter)
      {
        if( iter->second == chgEntry )
        {
          return false;  // Same Change Entry is available, no need to insert again
        }
      }

      [[maybe_unused]] const auto iter = _deltaEntries.insert({forDate, chgEntry});
      return true;
    }

    VersionedObject<T...> buildForwardTimeline(const std::chrono::year_month_day& startDate,
                                               const DataSet<T...>& firstVersion)
    {
      VersionedObject<T...> vo;
      vo.insertVersion(startDate,firstVersion);

      if(_deltaEntries.empty())
      {
        return vo;
      }

      if( startDate >= (_deltaEntries.begin()->first) )
      {
        std::ostringstream eoss;
        eoss << "ERROR(1) : failure in VersionedObjectBuilder<T...>::buildForwardTimeline() : startDate[";
        eoss << VersionedObject<T...>::_To_dbY(startDate) << "] should be less than first-changeDate[";
        eoss << VersionedObject<T...>::_To_dbY(_deltaEntries.begin()->first) << std::endl;
        firstVersion.toCSV(eoss);
        _toCSV(eoss);
        throw std::invalid_argument(eoss.str());
      }


      //VERSIONEDOBJECT_DEBUG_LOG("record=" << converter::ConvertFromTuple<T...>::ToStr(record));
      std::array <bool, sizeof...(T)> hitheroProcessedElements;
      for(size_t i = 0; i < sizeof...(T); ++i)
      {
        hitheroProcessedElements[i] = false;
      }
      std::chrono::year_month_day nextRecordDate = _deltaEntries.begin()->first;
      std::string source = "";
      t_record record = firstVersion.getRecord();
      for(auto iterDelta : _deltaEntries)
      {
        if(nextRecordDate != iterDelta.first)
        {
          vo.insertVersion(nextRecordDate, DataSet<T...>(source, record) );
          source = "";

          // two consecutive changes , but on different dates.
          // Hence treat it as new Record with no hitheroProcessedElements.
          for(size_t i = 0; i < sizeof...(T); ++i)
          {
            hitheroProcessedElements[i] = false;
          }
        }

        source = source + "+" + iterDelta.second.getSource();
        try {
          // 'getRecord()' is called with VALIDATE=true
          iterDelta.second.getLatestRecord(record, hitheroProcessedElements);
          //VERSIONEDOBJECT_DEBUG_LOG("record=" << converter::ConvertFromTuple<T...>::ToStr(record));
        } catch (const std::exception& err) {
          std::ostringstream eoss;
          eoss << "ERROR(2) : failure in VersionedObjectBuilder<T...>::buildForwardTimeline() : " << err.what() << std::endl;
          firstVersion.toCSV(eoss);
          _toCSV(eoss);
          throw std::invalid_argument(eoss.str());
        }

        nextRecordDate = iterDelta.first;
      }
      vo.insertVersion(nextRecordDate, DataSet<T...>(source, record) );
      //VERSIONEDOBJECT_DEBUG_LOG("record=" << converter::ConvertFromTuple<T...>::ToStr(record));
      _deltaEntries.clear();
      return vo;
    }

    VersionedObject<T...> buildReverseTimeline(const std::chrono::year_month_day& startDate,
                                               const DataSet<T...>& lastVersion)
    {
      VersionedObject<T...> vo;

      if(_deltaEntries.empty())
      {
        vo.insertVersion(startDate, lastVersion);
        return vo;
      }

      if( startDate >= (_deltaEntries.begin()->first) )
      {
        std::ostringstream eoss;
        eoss << "ERROR(1) : failure in VersionedObjectBuilder<T...>::buildReverseTimeline() : startDate[";
        eoss << VersionedObject<T...>::_To_dbY(startDate) << "] should be less than first-changeDate[";
        eoss << VersionedObject<T...>::_To_dbY(_deltaEntries.begin()->first) << std::endl;
        _toCSV(eoss);
        lastVersion.toCSV(eoss);
        throw std::invalid_argument(eoss.str());
      }

      vo.insertVersion(_deltaEntries.rbegin()->first, lastVersion);

      std::array <bool, sizeof...(T)> hitheroProcessedElements;
      for(size_t i = 0; i < sizeof...(T); ++i)
      {
        hitheroProcessedElements[i] = false;
      }
      auto rIterDelta = _deltaEntries.rbegin();
      std::chrono::year_month_day previousRecordDate = rIterDelta->first;
      std::string source = "";
      t_record record = lastVersion.getRecord();
      while( rIterDelta != _deltaEntries.rend() )
      {
        if(previousRecordDate != rIterDelta->first)
        {
          vo.insertVersion(rIterDelta->first, DataSet<T...>(source, record) );
          source = "";

          // two consecutive changes , but on different dates.
          // Hence treat it as new Record with no hitheroProcessedElements.
          for(size_t i = 0; i < sizeof...(T); ++i)
          {
            hitheroProcessedElements[i] = false;
          }
        }

        source = source + "-" + rIterDelta->second.getSource();
        try {
          // 'getRecord()' is called with VALIDATE=true
          rIterDelta->second.getPreviousRecord(record, hitheroProcessedElements);
        } catch (const std::exception& err) {
          std::ostringstream eoss;
          eoss << "ERROR(2) : failure in VersionedObject<T...>::buildReverseTimeline() : " << err.what() << std::endl;
          _toCSV(eoss);
          lastVersion.toCSV(eoss);
          throw std::invalid_argument(eoss.str());
        }

        previousRecordDate = rIterDelta->first;
        ++rIterDelta;
      }
      vo.insertVersion(startDate, DataSet<T...>(source, record) );
      _deltaEntries.clear();
      return vo;
    }

  };

}
