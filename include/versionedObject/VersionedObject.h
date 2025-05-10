/*
 * versionedObject.h
 *
 * URL:      https://github.com/panchaBhuta/dataStructure
 * Version:  v2.3.13
 *
 * Copyright (C) 2023-2023 Gautam Dhar
 * All rights reserved.
 *
 * dataStructure is distributed under the BSD 3-Clause license, see LICENSE for details.
 *
 */

#pragma once

#include <map>
#include <set>
#include <tuple>
#include <variant>
#include <string>
#include <sstream>
#include <chrono>

#include <optional>
#include <iterator>
#include <stdexcept>
#include <type_traits>

#include <converter/converter.h>

#include <dataStructure.h>

#if FLAG_VERSIONEDOBJECT_debug_log == 1
  #define VERSIONEDOBJECT_DEBUG_LOG(aMessage) { std::cout << aMessage << " :: file:" << DATASTRUCTURE_PREFERRED_PATH << ":" << __LINE__ << std::endl; }
  #define VERSIONEDOBJECT_DEBUG_MSG(aMessage) { std::cout << aMessage << std::endl; }
  #define VERSIONEDOBJECT_DEBUG_TRY_START try {
  #define VERSIONEDOBJECT_DEBUG_TRY_END   }
  #define VERSIONEDOBJECT_DEBUG_TRY_CATCH(EXCEPTION_TYPE)                                 \
      catch(const EXCEPTION_TYPE& ex) {                                                   \
        VERSIONEDOBJECT_DEBUG_LOG( "ERROR: " << #EXCEPTION_TYPE << " : " << ex.what() );  \
        throw ex;                                                                         \
      }
#else
  #define VERSIONEDOBJECT_DEBUG_LOG(aMessage)
  #define VERSIONEDOBJECT_DEBUG_MSG(aMessage)
  #define VERSIONEDOBJECT_DEBUG_TRY_START
  #define VERSIONEDOBJECT_DEBUG_TRY_END
  #define VERSIONEDOBJECT_DEBUG_TRY_CATCH(EXCEPTION_TYPE)
#endif


namespace datastructure { namespace versionedObject
{
  //using t_versionDate    = std::chrono::year_month_day;


  // SYMBOL,NAME OF COMPANY, SERIES, DATE OF LISTING, PAID UP VALUE, MARKET LOT, ISIN NUMBER, FACE VALUE
  // 20MICRONS,20 Microns Limited,BE,06-OCT-2008,5,1,INE144J01027,5

  template <typename, typename = void>
  struct is_MetaData : std::false_type {};

  template <typename MD>
  struct is_MetaData<MD, std::void_t< typename MD::isMetaData     // check for the presence of type-def MD::isMetaData
                                    >
                    >
           : std::is_same<typename MD::isMetaData, std::true_type>
  {};

  template <typename M>
  concept c_MetaData = is_MetaData<M>::value;

  template <typename M>
  concept c_noMetaData = !is_MetaData<M>::value;

  enum eModificationPatch {   //   ModficationType
    DELTACHANGE = '%',    //  DELTA_CHANGE=2
    SNAPSHOT    = '@',    //  SNAPSHOT=1
    FullRECORD  = '*'     //  NO_MODIFICATION=0
  };

  enum eBuildDirection {
    FORWARD   = '+',
    REVERSE   = '-',
    IsRECORD  = '*'
  };

    // this is optional. A user can define their own MetaData class and pass it to "DataSet<>"
  // A user defined MetaData class needs to define three components as below:
  //      1. using isMetaData = std::true_type;
  //      2. void merge(const MetaDataSource& other) { ... }
  //      3. assignment operator
  class MetaDataSource
  {
  public:
    using isMetaData = std::true_type;

    MetaDataSource(const std::string& source, eBuildDirection prefixBuildType, eModificationPatch dataPatch)
      : _source{source},
        _prefixBuildType{prefixBuildType},
        _dataPatch{dataPatch},
        _mergedSources{}
    {
      if( ( prefixBuildType == eBuildDirection::IsRECORD && dataPatch != eModificationPatch::FullRECORD ) ||
          ( prefixBuildType != eBuildDirection::IsRECORD && dataPatch == eModificationPatch::FullRECORD ) )
      {
        throw std::domain_error{"MetaDataSource() : prefixBuildType and dataPatch are both of RECORD type, OR neither are of RECORD type"};
      }
      if( dataPatch == eModificationPatch::SNAPSHOT && prefixBuildType != eBuildDirection::FORWARD )
      {
        throw std::domain_error{"MetaDataSource() : if dataPatch==SNAPSHOT; then prefixBuildType should be FORWARD"};
      }
    }

    //MetaDataSource() = default;
    MetaDataSource(MetaDataSource const&) = default;
    MetaDataSource& operator=(MetaDataSource const&) = default;
    bool operator==(MetaDataSource const& other) const = default;

    inline void toCSV(std::ostream& oss, const char delimiter = '#') const
    {
      if (_source.empty()) return;

      oss << char(_prefixBuildType) << delimiter << char(_dataPatch) << _source;

      for (const auto& sr : _mergedSources)
        oss << delimiter << sr;
    }

    inline std::string toCSV(const char delimiter = '#') const
    {
      std::ostringstream oss;
      toCSV(oss, delimiter);
      return oss.str();
    }

    void merge(MetaDataSource const& other)
    {
      if( _dataPatch == eModificationPatch::DELTACHANGE &&
          other._dataPatch == eModificationPatch::DELTACHANGE &&
          _prefixBuildType != other._prefixBuildType)
      {
        throw std::domain_error{"MetaDataSource::merge() expects same Build-type when dataPatch == DELTACHANGE"};
      }

      std::set<std::string> oSourceCopy {other._mergedSources}; // why : refer https://en.cppreference.com/w/cpp/container/set/merge
      std::string otherMain{char(other._dataPatch)};
      otherMain += other._source;
      oSourceCopy.insert(otherMain);
      _mergedSources.merge(oSourceCopy);

      std::string thisMain{char(_dataPatch)};
      thisMain += _source;
      _mergedSources.erase(thisMain);
    }

    eBuildDirection getBuildDirection() const { return _prefixBuildType; }

    private:
      const std::string             _source;
      const eBuildDirection         _prefixBuildType;
      const eModificationPatch      _dataPatch;
      std::set<std::string>         _mergedSources;
  };


  template <typename M, typename ... T>
  class DataSet;

  /*
   * Specialization which uses metadata
  */
  template <c_MetaData M, typename ... T>
  class DataSet<M, T...>
  {
  public:
    using t_record   = typename std::tuple<T ...>;
    using t_metaData = M;

    DataSet(const M& metaData, T&& ... args)
      : _metaData(metaData),
        _record(args...)
    {}

    DataSet(const M& metaData, const t_record& record)
      : _metaData(metaData),
        _record(record)
    {}

    DataSet(DataSet<M, T...> const& other)
      : _metaData(other._metaData),
        _record(other._record)
    {}

    DataSet() = delete;
    //DataSet(DataSet<M, T...> const&) = default;
    DataSet& operator=(DataSet<M, T...> const&) = delete;
    bool operator==(DataSet<M, T...> const& other) const // = default;
    {
      // we match only the record-data (and not the meta-info)
      return _record == other._record;
    }

    inline const M&           getMetaData() const { return _metaData; }
    inline const t_record&    getRecord()   const { return _record; }

    inline void toCSV(std::ostream& oss, const char delimiterMetaData = '#') const
    {
      oss << _metaData.toCSV(delimiterMetaData) << "," << converter::ConvertFromTuple<T...>::ToStr(_record);
    }

    inline std::string toCSV(const char delimiterMetaData = '#') const
    {
      std::ostringstream oss;
      toCSV(oss, delimiterMetaData);
      return oss.str();
    }

    inline std::string toLog(const char delimiterMetaData = '#') const
    {
      std::ostringstream oss;
      oss << " metaData=[" << _metaData.toCSV(delimiterMetaData)
          << "] ; record=[" << converter::ConvertFromTuple<T...>::ToStr(_record) << "]";
      return oss.str();
    }

    constexpr static bool hasMetaData() { return true; }

  private:
    const M           _metaData;     // metaData-id of a change instance
    const t_record    _record;       // value(s) of elements after  change
  };

  /*
   * Specialization which does NOT uses metadata
  */
  template <c_noMetaData T1, typename ... TR>  // if meta-data is not needed
  class DataSet<T1, TR...>
  {
  public:
    using t_record   = typename std::tuple<T1, TR...>;
    using t_metaData = void*;

    DataSet(T1&& arg1, TR&& ... args)
      : _record(arg1, args...)
    {}

    DataSet(const t_record& record)
      : _record(record)
    {}

    DataSet(DataSet<T1, TR...> const& other)
      : _record(other._record)
    {}

    DataSet() = delete;
    //DataSet(DataSet<T1, TR...> const&) = default;
    DataSet& operator=(DataSet<T1, TR...> const&) = delete;
    bool operator==(DataSet<T1, TR...> const&) const = default;

    inline const t_record&    getRecord() const { return _record; }

    inline void toCSV(std::ostream& oss) const
    {
      oss << converter::ConvertFromTuple<T1, TR...>::ToStr(_record);
    }

    inline std::string toCSV() const
    {
      std::ostringstream oss;
      toCSV(oss);
      return oss.str();
    }

    inline std::string toLog() const
    {
      std::ostringstream oss;
      oss << " record=[" << converter::ConvertFromTuple<T1, TR...>::ToStr(_record) << "]";
      return oss.str();
    }

    constexpr static bool hasMetaData() { return false; }

  private:
    const t_record    _record;       // value(s) of elements after change
  };



  template<size_t expIdx>
  class VO_exception : public std::invalid_argument
  {
  public :
    VO_exception(const std::string& msg) : std::invalid_argument(msg) {}
    VO_exception(const char*        msg) : std::invalid_argument(msg) {}
    virtual ~VO_exception() {}
  };

  using VO_Record_Mismatch_exception = VO_exception<0>;




  template <typename VDT, typename ... MT>
  class VersionedObject
  {
  public:
    using t_versionDate    = VDT;
    using t_dataset        = DataSet<MT ...>;
    using t_datasetLedger  = std::map< t_versionDate, t_dataset >;
    using t_record         = typename t_dataset::t_record;

    VersionedObject() : _datasetLedger() {}
    virtual ~VersionedObject()
    {
      _datasetLedger.clear();
    }

    //VersionedObject() = delete;
    VersionedObject(VersionedObject<VDT, MT...> const&) = default;
    VersionedObject& operator=(VersionedObject<VDT, MT...> const&) = default;
    bool operator==(VersionedObject<VDT, MT...> const&) const = default;

    // throws an error if for a particular date existing-record doesn't match the new-record
    // returns false if same record exists
    inline bool insertVersion(const t_versionDate& forDate, const t_dataset& newEntry)
    {
      //VERSIONEDOBJECT_DEBUG_LOG( "DEBUG_LOG:  versionDate=" << forDate << ", newEntry={ " << newEntry.toLog() << " }");  // this is too verbose
      const auto [ iter, success ] = _datasetLedger.emplace(forDate, newEntry);
      if( (!success) && (iter->second != newEntry) )  // different record exits in _datasetLedger
      {
        static std::string errMsg("ERROR : failure in VersionedObject<VDT, MT...>::insertVersion() : different record exits in _datasetLedger");
#if FLAG_VERSIONEDOBJECT_debug_log == 1
        std::ostringstream eoss;
        eoss << "DEBUG_LOG:  " << errMsg << " : forDate=" << forDate << " : prevEntry={ " << iter->second.toLog();
        eoss << " } : newEntry={ metaData=" << newEntry.toLog() << " }";
        VERSIONEDOBJECT_DEBUG_LOG(eoss.str());
#endif
        throw VO_Record_Mismatch_exception(errMsg);
      }
      return success;
    }

    inline typename t_datasetLedger::const_iterator
    getVersionAt(const t_versionDate& forDate) const
    {
      if(_datasetLedger.empty())
      {
        return _datasetLedger.cend();
      }

      // upper_bound -> returns an iterator to the first element greater than the given key
      auto iterC = _datasetLedger.upper_bound(forDate); // iterC points to first element that is after 'forDate'
      if(iterC == _datasetLedger.cbegin()) // no record before the 'forDate'
      {
        return _datasetLedger.cend();
      }
      --iterC;
      return iterC;
    }

    inline typename t_datasetLedger::const_iterator
    getVersionBefore(const t_versionDate& forDate) const
    {
      if(_datasetLedger.empty())
      {
        return _datasetLedger.cend();
      }

      // lower_bound -> Returns an iterator pointing to the first element that is not less than (i.e. greater or equal to) key
      auto iterC = _datasetLedger.lower_bound(forDate); // iterC points to first element that is greater or equal to 'forDate'
      if(iterC == _datasetLedger.cend() ||
         forDate <= iterC->first)
      {
        if(iterC == _datasetLedger.cbegin()) // no record before the 'forDate'
        {
          return _datasetLedger.cend();
        }
        return --iterC;
      }

      return iterC;
    }

    /*   TODO
    inline typename t_datasetLedger::const_iterator
    getVersionBefore(const t_versionDate& forDate) const
    {}
    */

    inline const t_datasetLedger& getDatasetLedger() const
    {
      return _datasetLedger;
    }

  private:
    t_datasetLedger  _datasetLedger;

  };

} }   //  namespace datastructure::versionedObject
