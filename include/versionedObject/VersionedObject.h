/*
 * versionedObject.h
 *
 * URL:      https://github.com/panchaBhuta/dataStructure
 * Version:  v3.4.14
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
#include <utility>

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

  enum eModificationPatch {
    DELTACHANGE = '%',
    SNAPSHOT    = '@',
    FullRECORD  = '*'
  };

  enum eBuildDirection {
    FORWARD   = '+',
    REVERSE   = '-',
    IsRECORD  = '*'
  };

  using t_DataType = std::string;



  template<size_t expIdx>
  class VO_exception : public std::invalid_argument
  {
  public :
    VO_exception(const std::string& msg) : std::invalid_argument(msg) {}
    VO_exception(const char*        msg) : std::invalid_argument(msg) {}
    virtual ~VO_exception() {}
  };

  using VO_Record_Mismatch_exception = VO_exception<0>;
  using EnumMismatch_MetaDataSource_exception   =  VO_exception<8>;
  using InvalidEnum_MetaDataSource_exception    =  VO_exception<9>;
  using MergeError_MetaDataSource_exception     =  VO_exception<10>;


  inline eModificationPatch char2ModificationPatch(const char cModificationPatch)
  {
    switch(cModificationPatch)
    {
    case static_cast<char>(eModificationPatch::DELTACHANGE) : // = '%',
    case static_cast<char>(eModificationPatch::SNAPSHOT)    : // = '@',
    case static_cast<char>(eModificationPatch::FullRECORD)  : // = '*'
      return static_cast<eModificationPatch>(cModificationPatch);
    default:
      std::ostringstream oss;
      oss << "datastructure::versionedObject::char2ModificationPatch() : invalid enum symbol '"
          << cModificationPatch << "'";
      throw InvalidEnum_MetaDataSource_exception(oss.str());
    };
  }

  inline eBuildDirection char2BuildDirection(const char cBuildDirection)
  {
    switch(cBuildDirection)
    {
    case static_cast<char>(eBuildDirection::FORWARD)  : // = '+',
    case static_cast<char>(eBuildDirection::REVERSE)  : // = '-',
    case static_cast<char>(eBuildDirection::IsRECORD) : // = '*'
      return static_cast<eBuildDirection>(cBuildDirection);
    default:
      std::ostringstream oss;
      oss << "datastructure::versionedObject::char2BuildDirection() : invalid enum symbol '"
          << cBuildDirection << "'";
      throw InvalidEnum_MetaDataSource_exception(oss.str());
    };
  }

  /*  template<typename T>
  concept has_resize_member_func = requires {
      typename T::size_type;
      { std::declval<T>().resize(std::declval<typename T::size_type>()) } -> std::same_as<void>;
  };
  */

  template<typename T, typename SH>
  concept has_void_toCSV_member_func = requires {
      { std::declval<T>().toCSV(std::declval<std::ostream>(), std::declval<SH>()) } -> std::same_as<void>;
  };

  template<typename CR>
  concept has_insert_member_func = requires {
      typename CR::value_type;
      typename CR::iterator;
      { std::declval<CR>().insert(std::declval<typename CR::value_type>()) } -> std::same_as< std::pair<typename CR::iterator, bool> >;
  };

  class StreamerHelper
  {
    char _delimiterMetaData;
    char _delimiterCSV;

  public:
    StreamerHelper(char pDelimiterMetaData = '|', char pDelimiterCSV = ',')
      : _delimiterMetaData{pDelimiterMetaData},
        _delimiterCSV{pDelimiterCSV}
    {}

    inline char getDelimiterMetaData() const { return _delimiterMetaData; }
    inline char getDelimiterCSV() const { return _delimiterCSV; }
  };

    // this is optional. A user can define their own MetaData class and pass it to "DataSet<>"
  // A user defined MetaData class needs to define three components as below:
  //      1. using isMetaData = std::true_type;
  //      2. void merge(const crtpMetaDataSource& other) { ... }
  //      3. assignment operator
  template<typename M, typename CONTAINER = std::set<t_DataType> >
  class crtpMetaDataSource   /// crtp -> Curiously recurring template pattern
  {
  public:
    using isMetaData = std::true_type;
    using t_Container = CONTAINER;

    crtpMetaDataSource(const t_DataType& dataType, eBuildDirection prefixBuildType, eModificationPatch dataPatch)
      : _dataType{dataType},
        _prefixBuildType{prefixBuildType},
        _dataPatch{dataPatch},
        _mergedDataTypes{}
    {
      if( ( prefixBuildType == eBuildDirection::IsRECORD && dataPatch != eModificationPatch::FullRECORD ) ||
          ( prefixBuildType != eBuildDirection::IsRECORD && dataPatch == eModificationPatch::FullRECORD ) )
      {
        throw EnumMismatch_MetaDataSource_exception{"crtpMetaDataSource() : prefixBuildType and dataPatch are both of RECORD type, OR neither are of RECORD type"};
      }
      if( dataPatch == eModificationPatch::SNAPSHOT && prefixBuildType != eBuildDirection::FORWARD ) // i.e SNAPSHOT change is applicable for FORWARD build only
      {
        throw InvalidEnum_MetaDataSource_exception{"crtpMetaDataSource() : if dataPatch==SNAPSHOT; then prefixBuildType should be FORWARD"};
      }
    }

    crtpMetaDataSource() = delete;
    crtpMetaDataSource(crtpMetaDataSource const&) = default;
    crtpMetaDataSource(crtpMetaDataSource &&) = default;
    crtpMetaDataSource& operator=(crtpMetaDataSource const&) = default;
    bool operator==(crtpMetaDataSource const& other) const = default;

    void merge(const M& otherNew)
    {
      static_cast<M*>(this)->_checkMerge(otherNew);

      CONTAINER oSourceCopy {otherNew._mergedDataTypes}; // why : refer https://en.cppreference.com/w/cpp/container/set/merge
      t_DataType otherMainKey{char(otherNew._dataPatch)};
      otherMainKey += otherNew._dataType;

      static_cast<M*>(this)->_insert(oSourceCopy, otherNew, otherMainKey);

      _mergedDataTypes.merge(oSourceCopy);

      t_DataType thisMainKey{char(_dataPatch)};
      thisMainKey += _dataType;
      _mergedDataTypes.erase(thisMainKey);
    }

    eBuildDirection getBuildDirection() const { return _prefixBuildType; }
    eModificationPatch getModificationPatch() const { return _dataPatch; }
    t_DataType getDataType() const { return _dataType; }

    virtual ~crtpMetaDataSource() { _mergedDataTypes.clear(); }

    template<typename SH = StreamerHelper>
    inline void toCSV(std::ostream& oss, const SH& streamerHelper = SH{}) const
    {
      const SH& sh = streamerHelper;
      oss << char(_prefixBuildType) << sh.getDelimiterMetaData() << char(_dataPatch) << _dataType;

      for (const auto& sr : _mergedDataTypes)
        oss << sh.getDelimiterMetaData() << sr;
    }

    template<typename SH = StreamerHelper>
    inline std::string toCSV(
      typename std::enable_if_t<  !std::is_same_v<SH, std::ostream&>,
                                  const SH& >
      streamerHelper = SH{}) const
    {
      std::ostringstream oss;
      if constexpr(has_void_toCSV_member_func<M, SH>)
      {
        using CM = const M;
        static_cast<CM*>(this)->template toCSV<SH>(oss, streamerHelper);
        return oss.str();
      } else
      if constexpr(has_insert_member_func<CONTAINER>)
      {
        this->template toCSV<SH>(oss, streamerHelper);
        return oss.str();
      }
    }

    protected:
      const t_DataType            _dataType;             // crown, symbolChange, nameChange, lotChange, delisted
      const eBuildDirection       _prefixBuildType;      // '+'  '-'  '*'
      const eModificationPatch    _dataPatch;            // '%'  '@'  '*'
      CONTAINER                   _mergedDataTypes;      // default std::set<t_DataType>

    void _checkMerge(M const& other) const
    {
      if( ( _dataPatch == eModificationPatch::FullRECORD  && other._dataPatch == eModificationPatch::FullRECORD ) &&   //   '*'
          ( _prefixBuildType == eBuildDirection::IsRECORD && other._prefixBuildType == eBuildDirection::IsRECORD ) )   //   '*'
      {
        if(_mergedDataTypes.size() > 0)
          throw MergeError_MetaDataSource_exception{"crtpMetaDataSource<M, CONTAINER>::_checkMerge() : _mergedDataTypes.size.size() should be zero"};

        if(other._mergedDataTypes.size() > 0)
          throw MergeError_MetaDataSource_exception{"crtpMetaDataSource<M, CONTAINER>::_checkMerge() : other._mergedDataTypes.size() should be zero"};

        return;
      }

      if( _dataPatch == eModificationPatch::FullRECORD  || other._dataPatch == eModificationPatch::FullRECORD )   //   '*'
      {
        throw MergeError_MetaDataSource_exception{"crtpMetaDataSource<M, CONTAINER>::_checkMerge() : _dataPatch and other._dataPatch should both be of 'FullRECORD' type, OR neither are of 'FullRECORD' type"};
      }

      if( _prefixBuildType == eBuildDirection::IsRECORD || other._prefixBuildType == eBuildDirection::IsRECORD )   //   '*'
      {
        throw MergeError_MetaDataSource_exception{"crtpMetaDataSource<M, CONTAINER>::_checkMerge() : _prefixBuildType and other._prefixBuildType should both be of 'IsRECORD' type, OR neither are of 'IsRECORD' type"};
      }


/*
20: DEBUG_LOG:  _VersionedObjectBuilderBase<VDT, MT...>::_updateComboDataSet(START)
20: DEBUG_LOG:  +++++before combo+++++++
20: DEBUG_LOG:    _logDeltaEntriesMap(START)
20: DEBUG_LOG:    versionDate[21-Jan-2014] : delta{-|%symbolChange,[REVERSE]:APPAPER->IPAPPM,,,,,,,}
20: DEBUG_LOG:    versionDate[22-Jan-2020] : delta{-|%symbolChange|%nameChange,[REVERSE]:IPAPPM->ANDPAPER,International Paper APPM Limited->ANDHRA PAPER LIMITED,,,,,,}
20: DEBUG_LOG:    versionDate[05-Mar-2020] : delta{-|%symbolChange,[REVERSE]:ANDPAPER->ANDHRAPAP,,,,,,,}
20: DEBUG_LOG:    _logDeltaEntriesMap(END)
20: DEBUG_LOG:  ~~~~~~~~~~~~~~~~~~~~~~~~
20: DEBUG_LOG:    _logSnapEntriesMap(START)
20: DEBUG_LOG:    versionDate[21-Jan-2014] : snap{+|@nameSpot,[FORWARD]:,International Paper APPM Limited,,,,,,}
20: DEBUG_LOG:    versionDate[05-Mar-2020] : snap{+|@nameSpot,[FORWARD]:,ANDHRA PAPER LIMITED,,,,,,}
20: DEBUG_LOG:    _logSnapEntriesMap(END)
20: DEBUG_LOG:  -----before combo-------
*/
      if( _prefixBuildType == other._prefixBuildType )  // '+' or '-'  ; NA '*'
      {
        //  { '+' , '+' } ; { '-' , '-' }
        if( _dataPatch == other._dataPatch )            // '%' or '@'  ; NA '*'
          return;      // { '+|%' , '+|%' } ; { '+|@' , '+|@' } ; { '-|%' , '-|%' } ; { NP , NP }   ;  NotPossible '-|@'

        return;        // { '+|%' , '+|@' } ; { '+|@' , '+|%' } ; { '-|%' , NP } ; { NP , '-|%' }   ;  NotPossible '-|@'
      } else {  //  ( _prefixBuildType != other._prefixBuildType )  // '+' or '-'  ; NA '*'
        //  { '+' , '-' } ; { '-' , '+' }

        if( _dataPatch == other._dataPatch )            // '%' or '@'  ; NA '*'
        {
          // { '+|%' , '-|%' } ; { '+|@' , NP } ; { '-|%' , '+|%' } ; { NP , '+|@' }   ;  NotPossible '-|@'

          if( _dataPatch == eModificationPatch::DELTACHANGE )  // '%'
          {
            // { '+|%' , '-|%' } ; { '-|%' , '+|%' }
            throw MergeError_MetaDataSource_exception{"crtpMetaDataSource<M, CONTAINER>::_checkMerge() : expects same Build-type when dataPatch == 'DELTACHANGE'"};
          }

          // { '+|@' , NP } ; { NP , '+|@' }   ;  NotPossible '-|@'
          return;
        }

        return;        // { '+|%' , NP } ; { '+|@' , '-|%' } ; { '-|%' , '+|@' } ; { NP , '+|%' }   ;  NotPossible '-|@'
      }

      // _dataType ( crown, symbolChange, nameChange, lotChange, delisted)
      // _dataType check is not needed here, as in merge(); we add other's KEY and remove this's KEY
    }

    void _insert(CONTAINER& oSourceCopy, [[maybe_unused]] const M& otherNew, const t_DataType& otherMainKey)
    {
      if(oSourceCopy.insert(otherMainKey).second == false)
      {
        throw MergeError_MetaDataSource_exception{"crtpMetaDataSource<M, CONTAINER>::_insert() : insert failed"};
      }
    }
  };

  class MetaDataSource : public crtpMetaDataSource<MetaDataSource>
  {
  public:
    //using isMetaData = std::true_type;
    //inline static const char metaDataDelimiter   = '|';

    MetaDataSource(const t_DataType& dataType, eBuildDirection prefixBuildType, eModificationPatch dataPatch)
      : crtpMetaDataSource<MetaDataSource>(dataType, prefixBuildType, dataPatch)
    {}

    MetaDataSource() = delete;
    MetaDataSource(MetaDataSource const&) = default;
    MetaDataSource(MetaDataSource &&) = default;
    MetaDataSource& operator=(MetaDataSource const&) = default;
    bool operator==(MetaDataSource const& other) const = default;

    ~MetaDataSource() {}
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

    template<typename SH = StreamerHelper>
    inline void toCSV(std::ostream& oss, const SH& streamerHelper = SH{}) const
    {
      const SH& sh = streamerHelper;
      oss << _metaData.toCSV(streamerHelper) << sh.getDelimiterCSV()
          << converter::ConvertFromTuple<T...>::ToStr(_record, sh.getDelimiterCSV());
    }

    template<typename SH = StreamerHelper>
    inline std::string toCSV(
      typename std::enable_if_t<  !std::is_same_v<SH, std::ostream&>,
                                  const SH& >
      streamerHelper = SH{}) const
    {
      std::ostringstream oss;
      toCSV(oss, streamerHelper);
      return oss.str();
    }

    template<typename SH = StreamerHelper>
    inline std::string toLog(const SH& streamerHelper = SH{}) const
    {
      const SH& sh = streamerHelper;
      std::ostringstream oss;
      oss << " metaData=[" << _metaData.toCSV(streamerHelper)
          << "] ; record=[" << converter::ConvertFromTuple<T...>::ToStr(_record, sh.getDelimiterCSV()) << "]";
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

    template<typename SH = StreamerHelper>
    inline void toCSV(std::ostream& oss, const SH& streamerHelper = SH{}) const
    {
      oss << converter::ConvertFromTuple<T1, TR...>::ToStr(_record, streamerHelper.getDelimiterCSV());
    }

    template<typename SH = StreamerHelper>
    inline std::string toCSV(
      typename std::enable_if_t<  !std::is_same_v<SH, std::ostream&>,
                                  const SH& >
      streamerHelper = SH{}) const
    {
      std::ostringstream oss;
      toCSV(oss, streamerHelper);
      return oss.str();
    }

    template<typename SH = StreamerHelper>
    inline std::string toLog(const SH& streamerHelper = SH{}) const
    {
      std::ostringstream oss;
      oss << " record=[" << converter::ConvertFromTuple<T1, TR...>::ToStr(_record, streamerHelper.getDelimiterCSV()) << "]";
      return oss.str();
    }

    constexpr static bool hasMetaData() { return false; }

  private:
    const t_record    _record;       // value(s) of elements after change
  };




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
