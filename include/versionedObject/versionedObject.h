/*
 * versionedObject.h
 *
 * Version:  v1.0.0
 *
 * Copyright (C) 2023-2023 Gautam Dhar
 * All rights reserved.
 * 
 * versionedObject is private and NOT licensed for public use.
 */

#pragma once

#include <map>
#include <array>
#include <tuple>
#include <variant>
#include <string>
#include <sstream>
#include <chrono>

#include <iterator>
#include <stdexcept>

#include <converter/converter.h>

#define VERSIONEDOBJECT_VERSION_MAJOR 1
#define VERSIONEDOBJECT_VERSION_MINOR 0
#define VERSIONEDOBJECT_VERSION_PATCH 0




namespace versionedObject
{
  /**
   * @brief     Class representing Version number of the project.
  */
  static constexpr struct {
    uint16_t major, minor, patch;
  } version = {
    VERSIONEDOBJECT_VERSION_MAJOR,
    VERSIONEDOBJECT_VERSION_MINOR,
    VERSIONEDOBJECT_VERSION_PATCH
  };



  // SYMBOL,NAME OF COMPANY, SERIES, DATE OF LISTING, PAID UP VALUE, MARKET LOT, ISIN NUMBER, FACE VALUE
  // 20MICRONS,20 Microns Limited,BE,06-OCT-2008,5,1,INE144J01027,5

  template <typename ... T>
  class DataSet
  {
  public:
    using t_record  = typename std::tuple<T ...>;

    DataSet(const std::string& source, T&& ... args)
      : _source(source),
        _record(args...)
    {}

    DataSet(const std::string& source, const t_record& record)
      : _source(source),
        _record(record)
    {}

    DataSet() = delete;
    DataSet(DataSet const&) = default;
    DataSet& operator=(DataSet const&) = delete;
    bool operator==(DataSet const&) const = default;

    inline const std::string& getSource() const { return _source; }
    inline const t_record&    getRecord() const { return _record; }

    inline void toCSV(std::ostream& oss) const
    {
      //std::ostringstream oss;
      oss << _source << "," << converter::ConvertFromTuple<T...>::ToStr(_record);
    }

  private:
    const std::string _source;                                    // source-id of a change instance
    const t_record    _record;                                    // value(s) of elements after  change
  };

  template <typename ... T>
  class VersionedObject;

  template <typename ... T>
  class ChangesInDataSet
  {
  public:
    using t_record  = typename DataSet<T ...>::t_record;

    ChangesInDataSet(const std::string& source,
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
    inline void         getRecord(t_record& updateRecord,
                                  std::array <bool, sizeof...(T)>& previouslyModifiedElements) const
    {
      _getLatestValue<sizeof...(T) -1, true>(updateRecord, previouslyModifiedElements);
    }

    inline void         getRecord(t_record& updateRecord) const
    {
      std::array <bool, sizeof...(T)> dummyModifiedElements;
      _getLatestValue<sizeof...(T) -1, false>(updateRecord, dummyModifiedElements);
    }

    inline void toCSV(std::ostream& oss) const
    {
      //std::ostringstream oss;
      oss << _source;
      _toCSV<0>(oss);
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
                                std::array <bool, sizeof...(T)>& previouslyModifiedElements) const
    {
      if( _modifiedElements.at(IDX) ) // check if element is marked for change
      {
        if constexpr(VALIDATE)
        {
          if( previouslyModifiedElements[IDX] ) // check if element was previously modified
          {
            std::ostringstream eoss;
            eoss << "ERROR(1) : in function ChangesInDataSet<T ...>::_getLatestValue() : ";
            eoss << " at tuple-index[" << IDX << "] : updateRecord-value{" << std::get<IDX>(updateRecord);
            eoss << "} was previously modified." << std::endl;
            throw std::invalid_argument(eoss.str());
          }
          previouslyModifiedElements[IDX] = true;

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
        _getLatestValue< ((IDX>0)?(IDX-1):0), VALIDATE >(updateRecord, previouslyModifiedElements);
      }
    }

    const std::string _source;                                     // source-id of a change instance

    const std::array <bool, sizeof...(T)> _modifiedElements;       // elements that has changed is indicated by 'true'
    const DataSet<T...> _oldValues;                                      // value(s) of elements before change
    const DataSet<T...> _newValues;                                      // value(s) of elements after  change

    friend class VersionedObject<T...>;
  };

  template <typename ... T>
  class VersionedObject
  {
  private:
    /*
      key 'year_month_day' is xOR in containers '_changeCompleteEntries' and '_changeFragmentEntries'.
      That is, for a particular key of type 'year_month_day', either there can be 'one' entry in 
      '_changeCompleteEntries'  xOR 'one-or-more' entry(ies) in '_changeFragmentEntries'.
      But there cannot be entries in containers '_changeCompleteEntries' and '_changeFragmentEntries'
      for the same key 'year_month_day'.

      There can be multiple entries for the same date in '_changeFragmentEntries',
      but those entries can collectively modify any individual tuple-element at the most once.
      Property 'ChangesInDataSet._modifiedElements' is used for tracking those changes.
    */
    std::map      < std::chrono::year_month_day, DataSet<T...> >           _changeCompleteEntries;
    std::multimap < std::chrono::year_month_day, ChangesInDataSet<T...> >  _changeFragmentEntries;

    static constexpr char _dbY_fmt[] = "%d-%b-%Y";  // string literal object with static storage duration

    static constexpr std::string (*_To_dbY)(const std::chrono::year_month_day& val) =
                      &converter::ConvertFromVal< std::chrono::year_month_day,
                                                  converter::T2S_Format_StreamYMD< _dbY_fmt >
                                                >::ToStr;

  public:
    using t_record  = typename DataSet<T ...>::t_record;

    VersionedObject() : _changeCompleteEntries(), _changeFragmentEntries() {}

    inline bool insertVersion(const std::chrono::year_month_day& forDate,
                              const DataSet<T...>& newEntry)
    {
      auto range = _changeFragmentEntries.equal_range(forDate);
      if(range.first != _changeFragmentEntries.end())  // change-Details exits in _changeFragmentEntries
      {
        std::ostringstream eoss;
        eoss << "ERROR(1) : failure in VersionedObject<T...>::insertVersion(year_month_day,DataSet) : ";
        eoss << "change-Detail's(count=" << std::distance(range.first,range.second);
        eoss << ") exits in _changeFragmentEntries : forDate=" << _To_dbY(forDate);
        eoss << " : existChange[0]={ source=" << range.first->second.getSource();
        eoss << " , oldValues=" << converter::ConvertFromTuple<T...>::ToStr(range.first->second._oldValues.getRecord());
        eoss << " , newValues=" << converter::ConvertFromTuple<T...>::ToStr(range.first->second._newValues.getRecord());
        eoss << " } : newEntry={ source=" << newEntry.getSource() << " , record=";
        eoss << converter::ConvertFromTuple<T...>::ToStr(newEntry.getRecord()) << " }";
        throw std::invalid_argument(eoss.str());
      }

      const auto [ iter, success ] = _changeCompleteEntries.insert({forDate, newEntry});
      if( (!success) && (iter->second != newEntry) )  // different record exits in _changeCompleteEntries
      {
        std::ostringstream eoss;
        eoss << "ERROR(2) : failure in VersionedObject<T...>::insertVersion(year_month_day,DataSet) : ";
        eoss << "different record exits in _changeCompleteEntries : forDate=";
        eoss << _To_dbY(forDate) << " : prevEntry={ source=" << iter->second.getSource() << " , record=";
        eoss << converter::ConvertFromTuple<T...>::ToStr(iter->second.getRecord()) << " } : newEntry={ source=";
        eoss << newEntry.getSource() << " , record=" << converter::ConvertFromTuple<T...>::ToStr(newEntry.getRecord()) << " }";
        throw std::invalid_argument(eoss.str());
      }
      return success;
    }

    inline bool insertVersion(const std::chrono::year_month_day& forDate,
                              const ChangesInDataSet<T...>& chgEntry)
    {
      auto findForDate = _changeCompleteEntries.find(forDate);
      if(findForDate != _changeCompleteEntries.end())  // record exits in _changeCompleteEntries
      {
        std::ostringstream eoss;
        eoss << "ERROR : failure in VersionedObject<T...>::insertVersion(year_month_day,ChangesInDataSet) : ";
        eoss << "record exits in _changeCompleteEntries : forDate=" << _To_dbY(forDate);
        eoss << " : existEntry={ source=" << findForDate->second.getSource() << " , record=";
        eoss << converter::ConvertFromTuple<T...>::ToStr(findForDate->second.getRecord()) << " } : newChange={ source=" << chgEntry.getSource();
        eoss << " , oldValues=" << converter::ConvertFromTuple<T...>::ToStr(chgEntry._oldValues.getRecord());
        eoss << " , newValues=" << converter::ConvertFromTuple<T...>::ToStr(chgEntry._newValues.getRecord()) << " }";
        throw std::invalid_argument(eoss.str());
      }

      auto range = _changeFragmentEntries.equal_range(forDate);
      for (auto iter = range.first; iter != range.second; ++iter)
      {
        if( iter->second == chgEntry )
        {
          return false;  // Same Change Entry is available, no need to insert again
        }
      }

      [[maybe_unused]] const auto iter = _changeFragmentEntries.insert({forDate, chgEntry});
      return true;
    }

    inline void validateChanges()
    {
      if( _changeFragmentEntries.empty() )
        return;

      if( _changeCompleteEntries.empty() ) // &&  !_changeFragmentEntries.empty()
      {
        std::ostringstream eoss;
        eoss << "ERROR(1) : failure in VersionedObject<T...>::validateChanges() : 'changeFragmentEntries' ";
        eoss << "has entries(" << _changeFragmentEntries.size() << "), where as 'changeCompleteEntries' is empty.";
        throw std::invalid_argument(eoss.str());
      }

      // no need to check for ">=" as check for "==" happens in 'insertVersion()'
      if( (_changeCompleteEntries.begin()->first) > (_changeFragmentEntries.begin()->first) )
      {
        std::ostringstream eoss;
        eoss << "ERROR(2) : failure in VersionedObject<T...>::validateChanges() : first-entry of changeFragmentEntries[";
        eoss << _To_dbY(_changeFragmentEntries.begin()->first);
        eoss << "] can not be before first-entry of changeCompleteEntries[";
        eoss << _To_dbY(_changeCompleteEntries.begin()->first) << "]";
        toCSV(std::cout);
        throw std::invalid_argument(eoss.str());
      }

      std::multimap < std::chrono::year_month_day,
                      std::variant<DataSet<T...>,ChangesInDataSet<T...>>
                    > allChangeEntries(_changeFragmentEntries.begin(),
                                       _changeFragmentEntries.end());
      allChangeEntries.insert(_changeCompleteEntries.begin(),
                              _changeCompleteEntries.end());

      t_record record;
      std::array <bool, sizeof...(T)> previouslyModifiedElements;
      std::chrono::year_month_day previousRecordDate;
      for(auto iter : allChangeEntries)
      {
        if( iter.second.index() == 0 ) // DataSet
        {
          record = std::get<DataSet<T...> >(iter.second).getRecord();
          previousRecordDate = iter.first;
        } else { //  ChangesInDataSet
          if(previousRecordDate != iter.first)
          {
            // two consecutive changes , but on different dates.
            // Hence treat it as new Record with no previousModifiedElements.
            for(size_t i = 0; i < sizeof...(T); ++i)
            {
              previouslyModifiedElements[i] = false;
            }
          }
          try {
            // 'getRecord()' is called with VALIDATE=true
            std::get<ChangesInDataSet<T...> >(iter.second).getRecord(record, previouslyModifiedElements);
          } catch (const std::exception& err) {
            toCSV(std::cout);
            std::ostringstream eoss;
            eoss << "ERROR(3) : failure in VersionedObject<T...>::validateChanges() : " << err.what();
            throw std::invalid_argument(eoss.str());
          }
          previousRecordDate = iter.first;
        }
      }
    }

    inline void toCSV(std::ostream& oss) const
    {
      std::multimap < std::chrono::year_month_day,
                      std::variant<DataSet<T...>,ChangesInDataSet<T...>>
                    > allChangeEntries(_changeFragmentEntries.begin(),
                                       _changeFragmentEntries.end());
      allChangeEntries.insert(_changeCompleteEntries.begin(),
                              _changeCompleteEntries.end());

      for(auto iter : allChangeEntries)
      {
        oss << _To_dbY(iter.first) << ",";
        if( iter.second.index() == 0 ) // DataSet
        {
          std::get<DataSet<T...> >(iter.second).toCSV(oss);
        } else { //  ChangesInDataSet
          std::get<ChangesInDataSet<T...> >(iter.second).toCSV(oss);
        }
        oss << std::endl;
      }
    }

    inline std::variant<DataSet<T...>,bool> getVersionAt(const std::chrono::year_month_day& forDate) const
    {
      auto iterC = _changeCompleteEntries.upper_bound(forDate); // iterC points to first element that is after 'forDate'
      if(iterC == _changeCompleteEntries.begin()) // no record before the 'forDate'
      {
        return false;
      }
      --iterC;

      // iterFbegin points to first element that is after 'iterC' entry-date
      auto iterFbegin = _changeFragmentEntries.upper_bound(iterC->first);

      // iterFend points to first element that is after 'forDate'
      auto iterFend   = _changeFragmentEntries.upper_bound(forDate);

      if( iterFbegin == iterFend)
      {
        // no change-entries to be applied
        return iterC->second;
      }

      t_record record = iterC->second.getRecord();
      for(auto iterF = iterFbegin; iterF != iterFend; ++iterF)
      {
        // 'getRecord()' is called with VALIDATE=false here,
        // as the assumption is that 'validateChanges()' has
        // been called before any calls made to
        // this function'getVersionAt()'
        iterF->second.getRecord(record);
      }
      return DataSet<T...>("getVersionAt", record);
    }

    inline void getAllVersions(std::map < std::chrono::year_month_day, DataSet<T...> >& applicableVersions) const
    {
      std::multimap < std::chrono::year_month_day,
                      std::variant<DataSet<T...>,ChangesInDataSet<T...>>
                    > allChangeEntries(_changeFragmentEntries.begin(),
                                       _changeFragmentEntries.end());
      allChangeEntries.insert(_changeCompleteEntries.begin(),
                              _changeCompleteEntries.end());

      t_record record;
      for(auto iter : allChangeEntries)
      {
        if( iter.second.index() == 0 ) // DataSet
        {
          DataSet ds = std::get<DataSet<T...> >(iter.second);
          record = ds.getRecord(); // needed for creating 'DataSet' in section 'ChangesInDataSet' below
          applicableVersions.insert( {iter.first, ds} );
        } else { //  ChangesInDataSet
          // 'getRecord()' is called with VALIDATE=false here,
          // as the assumption is that 'validateChanges()' has
          // been called before any calls made to 
          // this function 'getAllVersions()'
          std::get<ChangesInDataSet<T...> >(iter.second).getRecord(record);

          // NOTE: donot use 'applicableVersions.insert()' here.
          //       If there are multiple changes entries for the same date,
          //       we overwrite previous entry if any with the same date.
          //       'insert()' prevents overwritting.
          applicableVersions[iter.first] = DataSet("getAllVersions", record);
        }
      }
    }
  };

}