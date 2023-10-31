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
#include <tuple>
#include <variant>
#include <string>
#include <sstream>
#include <chrono>

#include <optional>
#include <iterator>
#include <stdexcept>

#include <converter/converter.h>

#define VERSIONEDOBJECT_VERSION_MAJOR 1
#define VERSIONEDOBJECT_VERSION_MINOR 0
#define VERSIONEDOBJECT_VERSION_PATCH 0



//  Project path is removed from the __FILE__
//  Resulting file-path is relative path from project-root-folder.
#if  VERSIONEDOBJECT_USE_FILEPREFIXMAP == 1
  // the project-prefix-path is removed via compilation directive file-prefix-map
  #define VERSIONEDOBJECT_FILE    __FILE__
#else
  // https://stackoverflow.com/questions/8487986/file-macro-shows-full-path/40947954#40947954
  // the project-prefix-path is skipped by offsetting to length of project-prefix-path
  //#define VERSIONEDOBJECT_FILE   (__FILE__ + VERSIONEDOBJECT_SOURCE_PATH_SIZE)  // gives lot of warnings on windows:clangCL
  #define VERSIONEDOBJECT_FILE   &(__FILE__[VERSIONEDOBJECT_SOURCE_PATH_SIZE])
#endif

// to handle windows back-slash path seperator
#define VERSIONEDOBJECT_PREFERRED_PATH    std::filesystem::path(VERSIONEDOBJECT_FILE).make_preferred().string()


#if ENABLE_VERSIONEDOBJECT_DEBUG_LOG == 1
  #define VERSIONEDOBJECT_DEBUG_LOG(aMessage) { std::cout << aMessage << " :: file:" << VERSIONEDOBJECT_PREFERRED_PATH << ":" << __LINE__ << std::endl; }
  #define VERSIONEDOBJECT_DEBUG_TRY_START try {
  #define VERSIONEDOBJECT_DEBUG_TRY_END   }
  #define VERSIONEDOBJECT_DEBUG_TRY_CATCH(EXCEPTION_TYPE)                             \
      catch(const EXCEPTION_TYPE& ex) {                                               \
        VERSIONEDOBJECT_DEBUG_LOG( "got-ERROR: " << ex.what() );                      \
        throw ex;                                                                     \
      }
#else
  #define VERSIONEDOBJECT_DEBUG_LOG(aMessage)
  #define VERSIONEDOBJECT_DEBUG_TRY_START
  #define VERSIONEDOBJECT_DEBUG_TRY_END
  #define VERSIONEDOBJECT_DEBUG_TRY_CATCH(EXCEPTION_TYPE)
#endif


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
      oss << _source << "," << converter::ConvertFromTuple<T...>::ToStr(_record);
    }

    inline std::string toCSV() const
    {
      std::ostringstream oss;
      toCSV(oss);
      return oss.str();
    }

  private:
    const std::string _source;                                    // source-id of a change instance
    const t_record    _record;                                    // value(s) of elements after  change
  };

  template <typename ... T>
  class VersionedObjectBuilder;

  template <typename ... T>
  class VersionedObject
  {
  private:
    std::map      < std::chrono::year_month_day, DataSet<T...> >           _changeCompleteEntries;

    static constexpr char _dbY_fmt[] = "%d-%b-%Y";  // string literal object with static storage duration

    static constexpr std::string (*_To_dbY)(const std::chrono::year_month_day& val) =
                      &converter::ConvertFromVal< std::chrono::year_month_day,
                                                  converter::T2S_Format_StreamYMD< _dbY_fmt >
                                                >::ToStr;

  public:
    using t_record  = typename DataSet<T ...>::t_record;

    VersionedObject() : _changeCompleteEntries() {}

    //VersionedObject() = delete;
    VersionedObject(VersionedObject const&) = default;
    VersionedObject& operator=(VersionedObject const&) = default;
    bool operator==(VersionedObject const&) const = default;

    inline bool insertVersion(const std::chrono::year_month_day& forDate,
                              const DataSet<T...>& newEntry)
    {
      VERSIONEDOBJECT_DEBUG_LOG( "date=" << _To_dbY(forDate) << ", newEntry={ source=" << newEntry.getSource() << " , record=" << converter::ConvertFromTuple<T...>::ToStr(newEntry.getRecord()) << " }");
      const auto [ iter, success ] = _changeCompleteEntries.insert({forDate, newEntry});
      if( (!success) && (iter->second != newEntry) )  // different record exits in _changeCompleteEntries
      {
        std::ostringstream eoss;
        eoss << "ERROR(1) : failure in VersionedObject<T...>::insertVersion() : ";
        eoss << "different record exits in _changeCompleteEntries : forDate=";
        eoss << _To_dbY(forDate) << " : prevEntry={ source=" << iter->second.getSource() << " , record=";
        eoss << converter::ConvertFromTuple<T...>::ToStr(iter->second.getRecord()) << " } : newEntry={ source=";
        eoss << newEntry.getSource() << " , record=" << converter::ConvertFromTuple<T...>::ToStr(newEntry.getRecord()) << " }";
        throw std::invalid_argument(eoss.str());
      }
      return success;
    }

    inline const std::optional<DataSet<T...>>
    getVersionAt(const std::chrono::year_month_day& forDate) const
    {
      if(_changeCompleteEntries.empty())
      {
        return {};
      }
    
      // upper_bound -> returns an iterator to the first element greater than the given key
      auto iterC = _changeCompleteEntries.upper_bound(forDate); // iterC points to first element that is after 'forDate'
      if(iterC == _changeCompleteEntries.begin()) // no record before the 'forDate'
      {
        return {};
      }
      --iterC;
      return iterC->second;
    }

    inline void getAllVersions(std::map < std::chrono::year_month_day, DataSet<T...> >& applicableVersions) const
    {
      applicableVersions.insert(_changeCompleteEntries.begin(),
                                _changeCompleteEntries.end());
    }

    inline void toCSV(std::ostream& oss) const
    {
      for(auto iter : _changeCompleteEntries)
      {
        oss << _To_dbY(iter->first) << ",";
        iter->second.toCSV(oss);
        oss << std::endl;
      }
    }

    friend class VersionedObjectBuilder<T...>;
  };

}


#include <versionedObject/VersionedObjectBuilder.h>