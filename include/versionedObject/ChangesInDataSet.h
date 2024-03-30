/*
 * ChangesInDataSet.h
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


namespace datastructure { namespace versionedObject
{

  using SnapshotChange_ReverseTimelineBuild_exception  =  VO_exception<3>;

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
      if (_isSnapshot)
      {
        std::ostringstream eoss;
        eoss << "ERROR : in function _ChangesInDataSetBase<T ...>::getPreviousRecord(t_record&, std::array <bool, sizeof...(T)>&) : ";
        eoss << "is applicable only for delta-change and NOT for snapshotChange. {";
        toCSV(eoss);
        eoss << "}" << std::endl;
        throw SnapshotChange_ReverseTimelineBuild_exception(eoss.str());
      }

      _getPreviousValue<sizeof...(T) -1, true>(updateRecord, hitheroProcessedElements);
    }

    inline void getPreviousRecord(t_record& updateRecord) const
    {
      if (_isSnapshot)
      {
        std::ostringstream eoss;
        eoss << "ERROR : in function _ChangesInDataSetBase<T ...>::getPreviousRecord(t_record&) : ";
        eoss << "is applicable only for delta-change and NOT for snapshotChange. { ";
        toCSV(eoss);
        eoss << "}" << std::endl;
        throw SnapshotChange_ReverseTimelineBuild_exception(eoss.str());
      }

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
        _newValues(newValues),
        _isSnapshot(false)
    {}

    _ChangesInDataSetBase( const std::array <bool, sizeof...(T)> modifiedElements,
                           const t_record& snapshotValues)
      : _modifiedElements(modifiedElements),
        _oldValues(),
        _newValues(snapshotValues),
        _isSnapshot(true)
    {}

    template<size_t IDX>
    inline void _toCSV(std::ostream& oss) const
    {
      oss << ",";
      if( _modifiedElements.at(IDX) ) // check if element is marked for change
      {
        if (!_isSnapshot)
        {
          // NOTE :: will fail for types that donot support "operator<<"
          oss << std::get<IDX>(_oldValues) << "->" << std::get<IDX>(_newValues);
        } else {
          // NOTE :: will fail for types that donot support "operator<<"
          oss << std::get<IDX>(_newValues);
        }
      }

      if constexpr( IDX < (sizeof...(T)-1) )
      {
        _toCSV< ( (IDX < (sizeof...(T)-1)) ? (IDX+1) : (sizeof...(T)-1) ) >(oss);
      }
    }

  private:
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

          if (!_isSnapshot)
          {
            if( std::get<IDX>(updateRecord) != std::get<IDX>(_oldValues) )
            {
              std::ostringstream eoss;
              eoss << "ERROR(2) : in function _ChangesInDataSetBase<T ...>::_getLatestValue() : ";
              eoss << " at tuple-index[" << IDX << "] : updateRecord-value{" << std::get<IDX>(updateRecord);
              eoss << "} doesn't match with expected old-Value{" << std::get<IDX>(_oldValues) << "}" << std::endl;
              throw std::invalid_argument(eoss.str());
            }
          } else {
            //TODO :: check if there is something to do here?
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
    const bool     _isSnapshot;
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

    ChangesInDataSet( const M& metaData,
                      const std::array <bool, sizeof...(T)> modifiedElements,
                      const t_record& snapshotValues)
      : _ChangesInDataSetBase<T...>(modifiedElements, snapshotValues),
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

    ChangesInDataSet( const std::array <bool, std::tuple_size_v<t_record>> modifiedElements,
                      const t_record& snapshotValues)
      : _ChangesInDataSetBase<T1, TR...>(modifiedElements, snapshotValues)
    {}

    ChangesInDataSet() = delete;
    ChangesInDataSet(ChangesInDataSet const&) = default;
    ChangesInDataSet& operator=(ChangesInDataSet const&) = delete;
    bool operator==(ChangesInDataSet const&) const = default;
  };

} }  //  datastructure::versionedObject
