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

#include <type_traits>
#include <array>

#include <versionedObject/VersionedObject.h>


namespace datastructure { namespace versionedObject
{
  enum ModficationType {
    NO_MODIFICATION=0,
    SNAPSHOT=1,
    DELTA_CHANGE=2
  };


  enum ApplicableChangeDirection {
    FORWARD,
    REVERSE
  };

  template <typename ... T>
  class _SnapshotDataSetBase
  {
  public:
    using t_record  = typename std::tuple<T ...>;

    _SnapshotDataSetBase() = delete;
    _SnapshotDataSetBase(_SnapshotDataSetBase const&) = default;
    _SnapshotDataSetBase& operator=(_SnapshotDataSetBase const&) = delete;
    bool operator==(_SnapshotDataSetBase const&) const = default;

    // elements that has changed is indicated by 'true'
    inline const std::array<ModficationType, sizeof...(T)>& getModifiedIndexes() const { return _modifiedElements; }
    inline ApplicableChangeDirection getApplicableChangeDirection() const { return _applicableChangeDirection; }
    inline const t_record& getNewRecord() const { return _newValues; }


    bool isSnapshotChgValueEqual(const t_record& matchRecord) const
    {
      return _isNextChgValueEqual<sizeof...(T) -1>(matchRecord);
    }

    template<bool VALIDATE>
    void getSnapshotRecord( t_record& updateRecord,
            std::array <bool, sizeof...(T)>& hitheroProcessedElements) const
    {
      _getLatestValue<sizeof...(T) -1, VALIDATE>(updateRecord, hitheroProcessedElements);
    }

    template<bool VALIDATE>
    void getSnapshotRecord( t_record& updateRecord) const
    {
      std::array <bool, sizeof...(T)> hitheroProcessedElements;
      hitheroProcessedElements.fill(false);
      _getLatestValue<sizeof...(T) -1, VALIDATE>(updateRecord, hitheroProcessedElements);
    }


    int isSubset(const _SnapshotDataSetBase<T...>& other) const // if greater than 0 then yes
    {
      std::array <int, sizeof...(T)> mergeableElements;
      mergeableElements.fill(0);
      _isMergeable<sizeof...(T) -1>(other, mergeableElements);  // 0, 1, 2

      size_t mergeableCount1 = 0;
      for(size_t iii = 0; iii < sizeof...(T); ++iii )
      {
        if(mergeableElements.at(iii) == 2)
          return -mergeableElements.at(iii);

        if(mergeableElements.at(iii) == 1) // skip the 0's
          ++mergeableCount1;
      }

      return mergeableCount1;
    }

    inline void toCSV(std::ostream& oss) const
    {
      //oss << _source;
      _toCSV<0>(oss);
    }

    inline std::string toCSV() const
    {
      std::ostringstream oss;
      _SnapshotDataSetBase<T... >::toCSV(oss);
      return oss.str();
    }

  protected:
    _SnapshotDataSetBase( const std::array<bool, sizeof...(T)>& modifiedElements,
                           const t_record& snapshotValues,
                                 ApplicableChangeDirection applicableChangeDirection = ApplicableChangeDirection::FORWARD)
      : _modifiedElements(),
        _newValues(snapshotValues),
        _applicableChangeDirection(applicableChangeDirection)
    {
      for(size_t iii = 0; iii < sizeof...(T); ++iii)
      {
        _modifiedElements.at(iii) = (modifiedElements.at(iii) ? SNAPSHOT : NO_MODIFICATION);
      }
    }

    template<size_t IDX>
    inline void _toCSV(std::ostream& oss) const
    {
      if constexpr( IDX == 0 )
      {
        if(_applicableChangeDirection == ApplicableChangeDirection::FORWARD)
        {
          oss << "[FORWARD]:";
        } else  {
          oss << "[REVERSE]:";
        }
      } else {
        oss << ",";
      }

      if( _modifiedElements.at(IDX) == SNAPSHOT ) // check if element is marked for change
      {
        // NOTE :: will fail for types that donot support "operator<<"
        oss << std::get<IDX>(_newValues);
      }

      if constexpr( IDX < (sizeof...(T)-1) )
      {
        _toCSV< ( (IDX < (sizeof...(T)-1)) ? (IDX+1) : (sizeof...(T)-1) ) >(oss);
      }
    }

    template<size_t IDX>
    inline bool _isNextChgValueEqual(const t_record& matchRecord) const
    {
      if( _modifiedElements.at(IDX) != NO_MODIFICATION && 
          ( std::get<IDX>(matchRecord) != std::get<IDX>(_newValues) ) )
      {
        return false;
      }

      if constexpr( IDX > 0 )
      {
        // "((IDX>0)?(IDX-1):0)" eliminates infinite compile time looping,
        // and we don't have to define function specialization for _isNextChgValueEqual<0>()
        return _isNextChgValueEqual<((IDX>0)?(IDX-1):0)>(matchRecord);
      } else {
        return true;
      }
    }

    template<size_t IDX, bool VALIDATE>
    inline void _getLatestValue(t_record& updateRecord,
                                std::array <bool, sizeof...(T)>& hitheroProcessedElements) const
    {
      if( _modifiedElements.at(IDX) != NO_MODIFICATION ) // check if element is marked for change
      {
        if constexpr(VALIDATE)
        {
          if( hitheroProcessedElements[IDX] ) // check if element was previously modified
          {
            std::ostringstream eoss;
            eoss << "ERROR(1) : in function _SnapshotDataSetBase<T ...>::_getLatestValue() : ";
            eoss << " at tuple-index[" << IDX << "] : updateRecord-value{" << std::get<IDX>(updateRecord);
            eoss << "} was previously modified." << std::endl;
            throw std::invalid_argument(eoss.str());
          }
          hitheroProcessedElements[IDX] = true;
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

    int _merge(const _SnapshotDataSetBase<T...>& other)
    {
      std::array <int, sizeof...(T)> mergeableElements;
      mergeableElements.fill(0);
      _isMergeable<sizeof...(T) -1>(other, mergeableElements);

      size_t mergeableCount1 = 0;
      size_t mergeableCount2 = 0;
      for(size_t iii = 0; iii < sizeof...(T); ++iii )
      {
        if(mergeableElements.at(iii) < 0)
          return mergeableElements.at(iii);

        if(mergeableElements.at(iii) == 1) // this.element == other.element
          ++mergeableCount1;

        if(mergeableElements.at(iii) == 2) // this.element is Null : AND : other.element has some value
          ++mergeableCount2;
      }

      if(mergeableCount2 > 0)
        _merge<sizeof...(T) -1>(other, mergeableElements);

      return mergeableCount1 + mergeableCount2;
    }

    template<size_t IDX>
    void _merge(const _SnapshotDataSetBase<T...>& other, std::array <int, sizeof...(T)>& mergeableElements)
    {
      if(mergeableElements.at(IDX) == 2)  // copy where applicable
      {
        std::get<IDX>(_newValues) = std::get<IDX>(other._newValues);
        _modifiedElements.at(IDX) = SNAPSHOT;
      }

      if constexpr( IDX > 0 )
      {
        // "((IDX>0)?(IDX-1):0)" eliminates infinite compile time looping,
        // and we don't have to define function specialization for _merge<0>()
        _merge< ((IDX>0)?(IDX-1):0) >(other, mergeableElements);
      }
    }


    template<size_t IDX>
    void _isMergeable(const _SnapshotDataSetBase<T...>& other, std::array <int, sizeof...(T)>& mergeableElements) const
    {
      if( _modifiedElements.at(IDX) != NO_MODIFICATION ) // check if element is marked for change
      {
        if( other._modifiedElements.at(IDX) != NO_MODIFICATION )
        {
          if constexpr((sizeof...(T) -1) == IDX)
          {
            if(other._applicableChangeDirection != _applicableChangeDirection)
            {
              std::ostringstream eoss;
              eoss << "ERROR : in function _SnapshotDataSetBase<T ...>::_isMergeable(const _SnapshotDataSetBase<T...>&) : ";
              eoss << " other._applicableChangeDirection{" << (other._applicableChangeDirection==FORWARD?"FORWARD":"REVERSE");
              eoss << "} doesn't match with expected this._applicableChangeDirection{" << (_applicableChangeDirection==FORWARD?"FORWARD":"REVERSE") << "}" << std::endl;
              throw std::invalid_argument(eoss.str());
            }
          }

          if(std::get<IDX>(other._newValues) != std::get<IDX>(_newValues))
          {
            std::ostringstream eoss;
            eoss << "ERROR : in function _SnapshotDataSetBase<T ...>::_isMergeable(const _SnapshotDataSetBase<T...>&) : ";
            eoss << " at tuple-index[" << IDX << "] : other-value{" << std::get<IDX>(other._newValues);
            eoss << "} doesn't match with expected this-Value{" << std::get<IDX>(_newValues) << "}" << std::endl;
            throw std::invalid_argument(eoss.str());
          }
          mergeableElements.at(IDX) = 1;  // elements of both tuple matches
        // } else { do nothing
        }
      } else {
        if( other._modifiedElements.at(IDX) != NO_MODIFICATION )
        {
          mergeableElements.at(IDX) = 2;  // element of other-tuple exists and is copyable to this-tuple.
        // } else { do nothing
        }
      }

      if constexpr( IDX > 0 )
      {
        // "((IDX>0)?(IDX-1):0)" eliminates infinite compile time looping,
        // and we don't have to define function specialization for _isMergeable<0>()
        _isMergeable< ((IDX>0)?(IDX-1):0) >(other, mergeableElements);
      }
    }

    std::array <ModficationType, sizeof...(T)> _modifiedElements;        // elements that has changed is indicated by 'true'
    t_record _newValues;                                      // value(s) of elements after  change
    const ApplicableChangeDirection _applicableChangeDirection;
  };



  template <typename M, typename ... T>
  class SnapshotDataSet;

  template <c_MetaData M, typename ... T>
  class SnapshotDataSet<M, T...> : public _SnapshotDataSetBase<T...>
  {
  public:
    using t_record  = typename std::tuple<T ...>;

    SnapshotDataSet(const M& metaData,
                    const std::array<bool, sizeof...(T)>& modifiedElements,
                    const t_record& snapshotValues,
                    ApplicableChangeDirection applicableChangeDirection = ApplicableChangeDirection::FORWARD)
      : _SnapshotDataSetBase<T...>(modifiedElements, snapshotValues, applicableChangeDirection),
        _metaData(metaData)
    {}

    SnapshotDataSet() = delete;
    SnapshotDataSet(SnapshotDataSet const&) = default;
    SnapshotDataSet& operator=(SnapshotDataSet const&) = delete;
    bool operator==(SnapshotDataSet const&) const = default;

    inline const M&           getMetaData() const { return _metaData; }

    int merge(const SnapshotDataSet<M, T...>& other)
    {
      // this->_metaData.merge(other._metaData); call append instead
      this->_metaData.appendMetaInfo(other._metaData);
      return this->template _merge<sizeof...(T) -1>(other);
    }

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
      SnapshotDataSet<M, T...>::toCSV(oss);
      return oss.str();
    }

  private:
    const M           _metaData;     // metaData-id of a change instance
  };

  template <c_noMetaData T1, typename ... TR>
  class SnapshotDataSet<T1, TR...> : public _SnapshotDataSetBase<T1, TR...>
  {
  public:
    using t_record  = typename std::tuple<T1, TR...>;

    SnapshotDataSet(const std::array<bool, std::tuple_size_v<t_record>>& modifiedElements,
                    const t_record& snapshotValues,
                    ApplicableChangeDirection applicableChangeDirection = ApplicableChangeDirection::FORWARD)
      : _SnapshotDataSetBase<T1, TR...>(modifiedElements, snapshotValues, applicableChangeDirection)
    {}

    int merge(const SnapshotDataSet<T1, TR...>& other)
    {
      return this->template _merge<sizeof...(TR) /*-1*/>(other);
    }

    SnapshotDataSet() = delete;
    SnapshotDataSet(SnapshotDataSet const&) = default;
    SnapshotDataSet& operator=(SnapshotDataSet const&) = delete;
    bool operator==(SnapshotDataSet const&) const = default;
  };

} }  //  datastructure::versionedObject
