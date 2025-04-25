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

#include <concepts>
#include <type_traits>
#include <array>

#include <versionedObject/VersionedObject.h>
#include <versionedObject/SnapshotDataSet.h>


namespace datastructure { namespace versionedObject
{

  template <typename ... T>
  class _ChangesInDataSetBase;

    template<typename OT, typename ... T>
    concept isChangeType =
        (std::is_base_of_v< _SnapshotDataSetBase<T...>, OT> == true ||
         std::is_base_of_v<_ChangesInDataSetBase<T...>, OT> == true );

  template <typename ... T>
  class _ChangesInDataSetBase
  {
  public:
    using t_record  = typename std::tuple<T ...>;

    _ChangesInDataSetBase() = delete;
    _ChangesInDataSetBase(_ChangesInDataSetBase const&) = default;
    _ChangesInDataSetBase& operator=(_ChangesInDataSetBase const&) = delete;
    bool operator==(_ChangesInDataSetBase const&) const = default;

    // elements that has changed is indicated by 'true'
    inline const std::array<ModficationType, sizeof...(T)>& getModifiedIndexes() const { return _modifiedElements; }
    inline ApplicableChangeDirection getApplicableChangeDirection() const { return _applicableChangeDirection; }
    inline const t_record& getNewRecord() const { return _newValues; }


    bool isNextChgValueEqual(const t_record& matchRecord) const
    {
      return _isNextChgValueEqual<sizeof...(T) -1>(matchRecord);
    }

    bool isPreviousChgValueEqual(const t_record& matchRecord) const
    {
      return _isPreviousChgValueEqual<sizeof...(T) -1>(matchRecord);
    }

    template<bool VALIDATE>
    void getLatestRecord( t_record& updateRecord,
                          std::array <bool, sizeof...(T)>& hitheroProcessedElements) const
    {
      _getLatestValue<sizeof...(T) -1, VALIDATE>(updateRecord, hitheroProcessedElements);
    }

    template<bool VALIDATE>
    void getLatestRecord(t_record& updateRecord) const
    {
      std::array <bool, sizeof...(T)> hitheroProcessedElements;
      hitheroProcessedElements.fill(false);
      _getLatestValue<sizeof...(T) -1, VALIDATE>(updateRecord, hitheroProcessedElements);
    }

    template<bool VALIDATE>
    void getPreviousRecord( t_record& updateRecord,
            std::array <bool, sizeof...(T)>& hitheroProcessedElements) const
    {
      _getPreviousValue<sizeof...(T) -1, VALIDATE>(updateRecord, hitheroProcessedElements);
    }

    template<bool VALIDATE>
    void getPreviousRecord( t_record& updateRecord) const
    {
      std::array <bool, sizeof...(T)> hitheroProcessedElements;
      hitheroProcessedElements.fill(false);
      _getPreviousValue<sizeof...(T) -1, VALIDATE>(updateRecord, hitheroProcessedElements);
    }

    int isSubset(const _SnapshotDataSetBase<T...>& other) const // if greater than 0 then yes
    {
      std::array <int, sizeof...(T)> mergeableElements;
      mergeableElements.fill(0);
      _isMergeable<sizeof...(T) -1, _SnapshotDataSetBase<T...> >(other, mergeableElements);  // 0, 1, 2

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
      _ChangesInDataSetBase<T... >::toCSV(oss);
      return oss.str();
    }

  protected:
    _ChangesInDataSetBase( const std::array<bool, sizeof...(T)>& modifiedElements,
                           const t_record& oldValues,
                           const t_record& newValues,
                                 ApplicableChangeDirection applicableChangeDirection)
      : _modifiedElements(),
        _oldValues(oldValues),
        _newValues(newValues),
        _applicableChangeDirection(applicableChangeDirection)
    {
      for(size_t iii = 0; iii < sizeof...(T); ++iii)
      {
        _modifiedElements.at(iii) = (modifiedElements.at(iii) ? DELTA_CHANGE : NO_MODIFICATION);
      }
    }

    _ChangesInDataSetBase(const _SnapshotDataSetBase<T...>& snapOther)
      : _modifiedElements(snapOther.getModifiedIndexes()),
        _oldValues(),
        _newValues(snapOther.getNewRecord()),
        _applicableChangeDirection(snapOther.getApplicableChangeDirection())
    {}

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

      if( _modifiedElements.at(IDX) == DELTA_CHANGE ) // check if element is marked for change
      {
        // NOTE :: will fail for types that donot support "operator<<"
        oss << std::get<IDX>(_oldValues) << "->" << std::get<IDX>(_newValues);
      } else if( _modifiedElements.at(IDX) == SNAPSHOT ) // check if element is marked for change
      {
        // NOTE :: will fail for types that donot support "operator<<"
        oss << "...->" << std::get<IDX>(_newValues);
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

    template<size_t IDX>
    inline bool _isPreviousChgValueEqual(const t_record& matchRecord) const
    {
      if( _modifiedElements.at(IDX) == DELTA_CHANGE && 
          ( std::get<IDX>(matchRecord) != std::get<IDX>(_oldValues) ) )
      {
        return false;
      }

      if constexpr( IDX > 0 )
      {
        // "((IDX>0)?(IDX-1):0)" eliminates infinite compile time looping,
        // and we don't have to define function specialization for _isPreviousChgValueEqual<0>()
        return _isPreviousChgValueEqual<((IDX>0)?(IDX-1):0)>(matchRecord);
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
            eoss << "ERROR(1) : in function _ChangesInDataSetBase<T ...>::_getLatestValue() : ";
            eoss << " at tuple-index[" << IDX << "] : updateRecord-value{" << std::get<IDX>(updateRecord);
            eoss << "} was previously modified." << std::endl;
            throw std::invalid_argument(eoss.str());
          }
          hitheroProcessedElements[IDX] = true;

          if( _modifiedElements.at(IDX) == DELTA_CHANGE )
          {
            if( std::get<IDX>(updateRecord) != std::get<IDX>(_oldValues) )
            {
              std::ostringstream eoss;
              eoss << "ERROR(2) : in function _ChangesInDataSetBase<T ...>::_getLatestValue() : ";
              eoss << " at tuple-index[" << IDX << "] : updateRecord-value{" << std::get<IDX>(updateRecord);
              eoss << "} doesn't match with expected old-Value{" << std::get<IDX>(_oldValues) << "}" << std::endl;
              throw std::invalid_argument(eoss.str());
            }
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
      if( _modifiedElements.at(IDX) != NO_MODIFICATION ) // check if element is marked for change
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
            eoss << "} doesn't match with expected new-Value{" << std::get<IDX>(_newValues) << "}" << std::endl;
            throw std::invalid_argument(eoss.str());
          }
        }

        if( _modifiedElements.at(IDX) == DELTA_CHANGE )
          std::get<IDX>(updateRecord) = std::get<IDX>(_oldValues);
      }

      if constexpr( IDX > 0 )
      {
        // "((IDX>0)?(IDX-1):0)" eliminates infinite compile time looping,
        // and we don't have to define function specialization for _getPreviousValue<0, VALIDATE>()
        _getPreviousValue< ((IDX>0)?(IDX-1):0), VALIDATE >(updateRecord, hitheroProcessedElements);
      }
    }

    template <isChangeType<T...> OT>
    int _merge(const OT& other)
    {
      //static_assert(std::is_same_v<decltype(other), const  _SnapshotDataSetBase<T...>& > == true ||
      //              std::is_same_v<decltype(other), const _ChangesInDataSetBase<T...>& > == true );
  
      std::array <int, sizeof...(T)> mergeableElements;
      mergeableElements.fill(0);
      _isMergeable<sizeof...(T) -1, OT>(other, mergeableElements);

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
        _merge<sizeof...(T) -1, OT>(other, mergeableElements);

      return mergeableCount1 + mergeableCount2;
    }

    template <size_t IDX, isChangeType<T...> OT>
    void _merge(const OT& other, std::array <int, sizeof...(T)>& mergeableElements)
    {
      //static_assert(std::is_same_v<decltype(other), const  _SnapshotDataSetBase<T...>& > == true ||
      //              std::is_same_v<decltype(other), const _ChangesInDataSetBase<T...>& > == true );

      if(mergeableElements.at(IDX) == 2)  // copy where applicable
      {
        std::get<IDX>(_newValues) = std::get<IDX>(other.getNewRecord());
        if constexpr(std::is_same_v<OT, _ChangesInDataSetBase<T...> > == true)
        {
          std::get<IDX>(_oldValues) = std::get<IDX>(other._oldValues);
          _modifiedElements.at(IDX) = DELTA_CHANGE;
        } else
        if constexpr(std::is_same_v<OT, _SnapshotDataSetBase<T...> > == true)
        {
          _modifiedElements.at(IDX) = SNAPSHOT;
        } else {
          static_assert(false);
        }
      }

      if constexpr( IDX > 0 )
      {
        // "((IDX>0)?(IDX-1):0)" eliminates infinite compile time looping,
        // and we don't have to define function specialization for _merge<0>()
        _merge< ((IDX>0)?(IDX-1):0) , OT >(other, mergeableElements);
      }
    }


    template<size_t IDX, isChangeType<T...> OT>
    void _isMergeable(const OT& other, std::array <int, sizeof...(T)>& mergeableElements) const
    {
      //static_assert(std::is_same_v<decltype(other), const  _SnapshotDataSetBase<T...>& > == true ||
      //              std::is_same_v<decltype(other), const _ChangesInDataSetBase<T...>& > == true );

      if( _modifiedElements.at(IDX) != NO_MODIFICATION ) // check if element is marked for change
      {
        if( other.getModifiedIndexes().at(IDX) != NO_MODIFICATION )
        {
          if constexpr((sizeof...(T) -1) == IDX)
          {
            if(other.getApplicableChangeDirection() != _applicableChangeDirection)
            {
              std::ostringstream eoss;
              eoss << "ERROR : in function _ChangesInDataSetBase<T ...>::_isMergeable(const auto&) : ";
              eoss << " other._applicableChangeDirection{" << (other.getApplicableChangeDirection()==FORWARD?"FORWARD":"REVERSE");
              eoss << "} doesn't match with expected this._applicableChangeDirection{" << (_applicableChangeDirection==FORWARD?"FORWARD":"REVERSE") << "}" << std::endl;
              throw std::invalid_argument(eoss.str());
            }
          }

          bool isError = std::get<IDX>(other.getNewRecord()) != std::get<IDX>(_newValues);
          if constexpr(std::is_same_v<decltype(other), const  _ChangesInDataSetBase<T...>& >)
          {
            isError |= (std::get<IDX>(other._oldValues) != std::get<IDX>(_oldValues));
          }
          if(isError)
          {
            std::ostringstream eoss;
            if constexpr(std::is_same_v<decltype(other), const  _ChangesInDataSetBase<T...>& >)
            {
              eoss << "ERROR : in function _ChangesInDataSetBase<T ...>::_isMergeable(const _ChangesInDataSetBase<T...>& other) : ";
              eoss << " at tuple-index[" << IDX << "] : other<newvalue,oldvalue>{" << std::get<IDX>(other._newValues);
              eoss << ',' << std::get<IDX>(other._oldValues);
              eoss << "} doesn't match with expected this<newvalue,oldvalue>{" << std::get<IDX>(_newValues);
              eoss << ',' << std::get<IDX>(_oldValues) << "}" << std::endl;
            } else {
              eoss << "ERROR : in function _ChangesInDataSetBase<T ...>::_isMergeable(const _SnapshotDataSetBase<T...>& other) : ";
              eoss << " at tuple-index[" << IDX << "] : other-value{" << std::get<IDX>(other.getNewRecord());
              eoss << "} doesn't match with expected this-Value{" << std::get<IDX>(_newValues) << "}" << std::endl;
            }
            throw std::invalid_argument(eoss.str());
          }
          mergeableElements.at(IDX) = 1;  // elements of both tuple matches
        // } else { do nothing
        }
      } else {
        if( other.getModifiedIndexes().at(IDX) != NO_MODIFICATION )
        {
          mergeableElements.at(IDX) = 2;  // element of other-tuple exists and is copyable to this-tuple.
        // } else { do nothing
        }
      }

      if constexpr( IDX > 0 )
      {
        // "((IDX>0)?(IDX-1):0)" eliminates infinite compile time looping,
        // and we don't have to define function specialization for _isMergeable<0>()
        _isMergeable< ((IDX>0)?(IDX-1):0), OT >(other, mergeableElements);
      }
    }

    std::array <ModficationType, sizeof...(T)> _modifiedElements;        // elements that has changed is indicated by 'true'
    t_record _oldValues;                                      // value(s) of elements before change
    t_record _newValues;                                      // value(s) of elements after  change
    const ApplicableChangeDirection _applicableChangeDirection;
  };



  template <typename M, typename ... T>
  class ChangesInDataSet;

  template <c_MetaData M, typename ... T>
  class ChangesInDataSet<M, T...> : public _ChangesInDataSetBase<T...>
  {
  public:
    using t_record  = typename std::tuple<T ...>;

    ChangesInDataSet( const M& metaData,
                      const std::array<bool, sizeof...(T)>& modifiedElements,
                      const t_record& oldValues,
                      const t_record& newValues,
                      ApplicableChangeDirection applicableChangeDirection)
      : _ChangesInDataSetBase<T...>(modifiedElements, oldValues, newValues, applicableChangeDirection),
        _metaData(metaData)
    {}

    explicit ChangesInDataSet( const SnapshotDataSet<M, T...>& otherSnapDataset)
      : _ChangesInDataSetBase<T...>(otherSnapDataset),
        _metaData(otherSnapDataset.getMetaData())
    {}

    ChangesInDataSet() = delete;
    ChangesInDataSet(ChangesInDataSet const&) = default;
    ChangesInDataSet& operator=(ChangesInDataSet const&) = delete;
    bool operator==(ChangesInDataSet const&) const = default;

    inline const M&           getMetaData() const { return _metaData; }

    int merge(const auto& other)
    {
      if constexpr(std::is_same_v<decltype(other), const  SnapshotDataSet<M, T...>& > == true)
      {
        this->_metaData.merge(other.getMetaData());

        return this->_merge(dynamic_cast<const _SnapshotDataSetBase<T...>& >(other));
      } else
      if constexpr(std::is_same_v<decltype(other), const  ChangesInDataSet<M, T...>& > == true)
      {
        this->_metaData.merge(other.getMetaData());

        return this->_merge(dynamic_cast<const _ChangesInDataSetBase<T...>& >(other));
      } else
      if constexpr(std::is_same_v<decltype(other), const  SnapshotDataSet<T...>& > == true)
      {
         // special case: SnapshotDataSet<T...> is without metaInfo, as it's part of
         //               previous ChangesInDataSet<M, T...> OR SnapshotDataSet<M, T...>
         //               that's being appended here.
        return this->_merge(dynamic_cast<const _SnapshotDataSetBase<T...>& >(other));
      } else
        static_assert(false);
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
      ChangesInDataSet<M, T...>::toCSV(oss);
      return oss.str();
    }

  private:
    M           _metaData;     // metaData-id of a change instance
  };

  template <c_noMetaData T1, typename ... TR>
  class ChangesInDataSet<T1, TR...> : public _ChangesInDataSetBase<T1, TR...>
  {
  public:
    using t_record  = typename std::tuple<T1, TR...>;

    ChangesInDataSet( const std::array<bool, std::tuple_size_v<t_record>>& modifiedElements,
                      const t_record& oldValues,
                      const t_record& newValues,
                      ApplicableChangeDirection applicableChangeDirection)
      : _ChangesInDataSetBase<T1, TR...>(modifiedElements, oldValues, newValues, applicableChangeDirection)
    {}

    explicit ChangesInDataSet( const SnapshotDataSet<T1, TR...>& otherSnapDataset)
      : _ChangesInDataSetBase<T1, TR...>(otherSnapDataset)
    {}

    int merge(const auto& other)
    {
      if constexpr(std::is_same_v<decltype(other), const  SnapshotDataSet<T1, TR...>& > == true)
      {
        return this->_merge(dynamic_cast<const _SnapshotDataSetBase<T1, TR...>& >(other));
      } else
      if constexpr(std::is_same_v<decltype(other), const  ChangesInDataSet<T1, TR...>& > == true)
      {
        return this->_merge(dynamic_cast<const _ChangesInDataSetBase<T1, TR...>& >(other));
      } else
        static_assert(false);
    }

    ChangesInDataSet() = delete;
    ChangesInDataSet(ChangesInDataSet const&) = default;
    ChangesInDataSet& operator=(ChangesInDataSet const&) = delete;
    bool operator==(ChangesInDataSet const&) const = default;
  };

} }  //  datastructure::versionedObject
