/*
 * ChangesInDataSet.h
 *
 * URL:      https://github.com/panchaBhuta/dataStructure
 * Version:  v3.5
 *
 * Copyright (C) 2023-2025 Gautam Dhar
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
        (std::is_same_v< _SnapshotDataSetBase<T...>, OT> == true ||
         std::is_same_v<_ChangesInDataSetBase<T...>, OT> == true );
        //(std::is_base_of_v< _SnapshotDataSetBase<T...>, OT> == true ||
        // std::is_base_of_v<_ChangesInDataSetBase<T...>, OT> == true );

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
    inline const std::array<eModificationPatch, sizeof...(T)>& getModifiedIndexes() const { return _modifiedElements; }
    inline eBuildDirection getBuildDirection() const { return _buildDirection; }
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
      _isMergeableChanges<sizeof...(T) -1, _SnapshotDataSetBase<T...> >(other, mergeableElements);  // 0, 1, 2

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

  protected:
    _ChangesInDataSetBase( const std::array<bool, sizeof...(T)>& modifiedElements,
                           const t_record& oldValues,
                           const t_record& newValues,
                           eBuildDirection buildDirection)
      : _modifiedElements{},
        _oldValues{oldValues},
        _newValues{newValues},
        _buildDirection{buildDirection}
    {
      for(size_t iii = 0; iii < sizeof...(T); ++iii)
      {
        _modifiedElements.at(iii) = (modifiedElements.at(iii) ? eModificationPatch::DELTACHANGE
                                                              : eModificationPatch::FullRECORD);
      }
    }

    _ChangesInDataSetBase(const _SnapshotDataSetBase<T...>& snapOther)
      : _modifiedElements{snapOther.getModifiedIndexes()},
        _oldValues{},
        _newValues{snapOther.getNewRecord()},
        _buildDirection{snapOther.getBuildDirection()}
    {}

    template<size_t IDX , typename SH> // = StreamerHelper>
    inline void _toCSV(std::ostream& oss, const SH& streamerHelper = SH{}) const
    {
      if constexpr( IDX == 0 )
      {
        if(_buildDirection == eBuildDirection::FORWARD)
        {
          oss << "[FORWARD]:";
        } else if(_buildDirection == eBuildDirection::REVERSE) {
          oss << "[REVERSE]:";
        } else {
          oss << "[IsRECORD]:";
        }
      } else {
        oss << streamerHelper.getDelimiterCSV();
      }

      using t_idx_type = std::tuple_element_t<IDX, t_record>;
      if( _modifiedElements.at(IDX) == eModificationPatch::DELTACHANGE ) // check if element is marked for change
      {
        const t_idx_type& oldVal = std::get<IDX>(_oldValues);
        const t_idx_type& newVal = std::get<IDX>(_newValues);
        // NOTE :: will fail for types that donot support "operator<<"
        oss << oldVal << "->" << newVal;
      } else if( _modifiedElements.at(IDX) == eModificationPatch::SNAPSHOT ) // check if element is marked for change
      {
        const t_idx_type& newVal = std::get<IDX>(_newValues);
        // NOTE :: will fail for types that donot support "operator<<"
        oss << "...->" << newVal;
      }

      if constexpr( IDX < (sizeof...(T)-1) )
      {
        _toCSV< ( (IDX < (sizeof...(T)-1)) ? (IDX+1) : (sizeof...(T)-1) ) , SH >(oss, streamerHelper);
      }
    }

    template<size_t IDX>
    inline bool _isNextChgValueEqual(const t_record& matchRecord) const
    {
      if( _modifiedElements.at(IDX) != eModificationPatch::FullRECORD &&
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
      if( _modifiedElements.at(IDX) == eModificationPatch::DELTACHANGE &&
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
      if( _modifiedElements.at(IDX) != eModificationPatch::FullRECORD ) // check if element is marked for change
      {
        if constexpr(VALIDATE)
        {
          using t_idx_type = std::tuple_element_t<IDX, t_record>;
          if( hitheroProcessedElements[IDX] ) // check if element was previously modified
          {
            const t_idx_type& updateVal = std::get<IDX>(updateRecord);

            std::ostringstream eoss;
            eoss << "ERROR(1) : in function _ChangesInDataSetBase<T ...>::_getLatestValue() : ";
            eoss << " at tuple-index[" << IDX << "] : updateRecord-value{" << updateVal;
            eoss << "} was previously modified." << std::endl;
            throw std::invalid_argument(eoss.str());
          }
          hitheroProcessedElements[IDX] = true;

          if( _modifiedElements.at(IDX) == eModificationPatch::DELTACHANGE )
          {
            if( std::get<IDX>(updateRecord) != std::get<IDX>(_oldValues) )
            {
              const t_idx_type& updateVal = std::get<IDX>(updateRecord);
              const t_idx_type& oldVal = std::get<IDX>(_oldValues);

              std::ostringstream eoss;
              eoss << "ERROR(2) : in function _ChangesInDataSetBase<T ...>::_getLatestValue() : ";
              eoss << " at tuple-index[" << IDX << "] : updateRecord-value{" << updateVal;
              eoss << "} doesn't match with expected old-Value{" << oldVal << "}" << std::endl;
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
      if( _modifiedElements.at(IDX) != eModificationPatch::FullRECORD ) // check if element is marked for change
      {
        if constexpr(VALIDATE)
        {
          using t_idx_type = std::tuple_element_t<IDX, t_record>;
          if( hitheroProcessedElements[IDX] ) // check if element was previously modified
          {
            const t_idx_type& updateVal = std::get<IDX>(updateRecord);

            std::ostringstream eoss;
            eoss << "ERROR(1) : in function _ChangesInDataSetBase<T ...>::_getPreviousValue() : ";
            eoss << " at tuple-index[" << IDX << "] : updateRecord-value{" << updateVal;
            eoss << "} was previously modified." << std::endl;
            throw std::invalid_argument(eoss.str());
          }
          hitheroProcessedElements[IDX] = true;

          if( std::get<IDX>(updateRecord) != std::get<IDX>(_newValues) )
          {
            const t_idx_type& updateVal = std::get<IDX>(updateRecord);
            const t_idx_type& newVal = std::get<IDX>(_newValues);

            std::ostringstream eoss;
            eoss << "ERROR(2) : in function _ChangesInDataSetBase<T ...>::_getPreviousValue() : ";
            eoss << " at tuple-index[" << IDX << "] : updateRecord-value{" << updateVal;
            eoss << "} doesn't match with expected new-Value{" << newVal << "}" << std::endl;
            throw std::invalid_argument(eoss.str());
          }
        }

        if( _modifiedElements.at(IDX) == eModificationPatch::DELTACHANGE )
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
    int _mergeChanges(const OT& other)
    {
      //static_assert(std::is_same_v<decltype(other), const  _SnapshotDataSetBase<T...>& > == true ||
      //              std::is_same_v<decltype(other), const _ChangesInDataSetBase<T...>& > == true );

      std::array <int, sizeof...(T)> mergeableElements;
      mergeableElements.fill(0);
      _isMergeableChanges<sizeof...(T) -1, OT>(other, mergeableElements);

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
        _mergeChanges<sizeof...(T) -1, OT>(other, mergeableElements);

      return mergeableCount1 + mergeableCount2;
    }

    template <size_t IDX, isChangeType<T...> OT>
    void _mergeChanges(const OT& other, std::array <int, sizeof...(T)>& mergeableElements)
    {
      if(mergeableElements.at(IDX) == 2)  // copy where applicable
      {
        std::get<IDX>(_newValues) = std::get<IDX>(other.getNewRecord());
        _mergeValueChanges<IDX>(other);
      }

      if constexpr( IDX > 0 )
      {
        // "((IDX>0)?(IDX-1):0)" eliminates infinite compile time looping,
        // and we don't have to define function specialization for _merge<0>()
        _mergeChanges< ((IDX>0)?(IDX-1):0) , OT >(other, mergeableElements);
      }
    }

    template <size_t IDX>
    inline void _mergeValueChanges(const _ChangesInDataSetBase<T...>& other)
    {
      std::get<IDX>(_oldValues) = std::get<IDX>(other._oldValues);
      _modifiedElements.at(IDX) = eModificationPatch::DELTACHANGE;
    }

    template <size_t IDX>
    inline void _mergeValueChanges([[maybe_unused]] const _SnapshotDataSetBase<T...>& other)
    {
      _modifiedElements.at(IDX) = eModificationPatch::SNAPSHOT;
    }


    template<size_t IDX, isChangeType<T...> OT>
    void _isMergeableChanges(const OT& other, std::array <int, sizeof...(T)>& mergeableElements) const
    {
      //static_assert(std::is_same_v<decltype(other), const  _SnapshotDataSetBase<T...>& > == true ||
      //              std::is_same_v<decltype(other), const _ChangesInDataSetBase<T...>& > == true );

      if( _modifiedElements.at(IDX) != eModificationPatch::FullRECORD ) // check if element is marked for change
      {
        if( other.getModifiedIndexes().at(IDX) != eModificationPatch::FullRECORD )
        {
          if constexpr((sizeof...(T) -1) == IDX)
          {
            if(other.getBuildDirection() != _buildDirection)
            {
              std::ostringstream eoss;
              eoss << "ERROR(1) : in function _ChangesInDataSetBase<T ...>::_isMergeableChanges(const auto&) : ";
              eoss << " other._buildDirection{" << (other.getBuildDirection()==eBuildDirection::FORWARD?"FORWARD":"REVERSE");
              eoss << "} doesn't match with expected this._buildDirection{" << (_buildDirection==eBuildDirection::FORWARD?"FORWARD":"REVERSE") << "}" << std::endl;
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
            using t_idx_type = std::tuple_element_t<IDX, t_record>;
            std::ostringstream eoss;
            if constexpr(std::is_same_v<decltype(other), const  _ChangesInDataSetBase<T...>& >)
            {
              const t_idx_type& newVal  = std::get<IDX>(_newValues);
              const t_idx_type& oldVal  = std::get<IDX>(_oldValues);
              const t_idx_type& oNewVal = std::get<IDX>(other._newValues);
              const t_idx_type& oOldVal = std::get<IDX>(other._oldValues);
              eoss << "ERROR(2) : in function _ChangesInDataSetBase<T ...>::_isMergeableChanges(const _ChangesInDataSetBase<T...>& other) : ";
              eoss << " at tuple-index[" << IDX << "] : other<newvalue,oldvalue>{" << oNewVal;
              eoss << ',' << oOldVal;
              eoss << "} doesn't match with expected this<newvalue,oldvalue>{" << newVal;
              eoss << ',' << oldVal << "}" << std::endl;
            } else {
              const t_idx_type& newVal  = std::get<IDX>(_newValues);
              const t_idx_type& oNewVal = std::get<IDX>(other.getNewRecord());
              eoss << "ERROR(3) : in function _ChangesInDataSetBase<T ...>::_isMergeableChanges(const _SnapshotDataSetBase<T...>& other) : ";
              eoss << " at tuple-index[" << IDX << "] : other-value{" << oNewVal;
              eoss << "} doesn't match with expected this-Value{" << newVal << "}" << std::endl;
            }
            throw std::invalid_argument(eoss.str());
          }
          mergeableElements.at(IDX) = 1;  // elements of both tuple matches
        // } else { do nothing
        }
      } else {
        if( other.getModifiedIndexes().at(IDX) != eModificationPatch::FullRECORD )
        {
          mergeableElements.at(IDX) = 2;  // element of other-tuple exists and is copyable to this-tuple.
        // } else { do nothing
        }
      }

      if constexpr( IDX > 0 )
      {
        // "((IDX>0)?(IDX-1):0)" eliminates infinite compile time looping,
        // and we don't have to define function specialization for _isMergeable<0>()
        _isMergeableChanges< ((IDX>0)?(IDX-1):0), OT >(other, mergeableElements);
      }
    }

    std::array <eModificationPatch, sizeof...(T)> _modifiedElements;        // elements that has changed is indicated by 'DELTACHANGE'
    t_record _oldValues;                                                    // value(s) of elements before change
    t_record _newValues;                                                    // value(s) of elements after  change
    const eBuildDirection _buildDirection;
  };



  template <typename M, typename ... T>
  class ChangesInDataSet;

  template <c_MetaData M, typename ... T>
  class ChangesInDataSet<M, T...> : public _ChangesInDataSetBase<T...>
  {
  public:
    using t_record  = typename std::tuple<T ...>;

    ChangesInDataSet( const std::array<bool, sizeof...(T)>& modifiedElements,
                      const t_record& oldValues,
                      const t_record& newValues,
                      const M& metaData)
      : _ChangesInDataSetBase<T...>(modifiedElements, oldValues, newValues, metaData.getBuildDirection()),
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

    int mergeChanges(const SnapshotDataSet<M, T...>& other)
    {
      this->_metaData.mergeChanges(other.getMetaData());

      return this->_mergeChanges(dynamic_cast<const _SnapshotDataSetBase<T...>& >(other));
    }

    int mergeChanges(const ChangesInDataSet<M, T...>& other)
    {
      this->_metaData.mergeChanges(other.getMetaData());

      return this->_mergeChanges(dynamic_cast<const _ChangesInDataSetBase<T...>& >(other));
    }

    int mergeChanges(const SnapshotDataSet<T...>& other)
    {
        // special case: SnapshotDataSet<T...> is without metaInfo, as it's part of
        //               previous ChangesInDataSet<M, T...> OR SnapshotDataSet<M, T...>
        //               that's being appended here.
      return this->_mergeChanges(dynamic_cast<const _SnapshotDataSetBase<T...>& >(other));
    }

    template<typename SH = typename M::t_StreamerHelper>
    inline void toCSV(std::ostream& oss, const SH& streamerHelper = SH{}) const
    {
      const SH& sh = streamerHelper;
      oss << _metaData.toCSV(streamerHelper) << sh.getDelimiterCSV();

      // _toCSV<0>(oss);   #########  DOESNOT COMPILE : refer urls below
      //  https://stackoverflow.com/questions/9289859/calling-template-function-of-template-base-class
      //  https://stackoverflow.com/questions/610245/where-and-why-do-i-have-to-put-the-template-and-typename-keywords
      this->template _toCSV<0>(oss, streamerHelper);
    }

    template<typename SH = typename M::t_StreamerHelper>
    inline std::string toCSV(
      typename std::enable_if_t<  !std::is_same_v<SH, std::ostream&>,
                                  const SH& >
      streamerHelper = SH{}) const
    {
      std::ostringstream oss;
      ChangesInDataSet<M, T...>::toCSV(oss, streamerHelper);
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
                      eBuildDirection buildDirection)
      : _ChangesInDataSetBase<T1, TR...>(modifiedElements, oldValues, newValues, buildDirection)
    {}

    explicit ChangesInDataSet( const SnapshotDataSet<T1, TR...>& otherSnapDataset)
      : _ChangesInDataSetBase<T1, TR...>(otherSnapDataset)
    {}

    int mergeChanges(const SnapshotDataSet<T1, TR...>& other)
    {
      return this->_mergeChanges(dynamic_cast<const _SnapshotDataSetBase<T1, TR...>& >(other));
    }

    int mergeChanges(const ChangesInDataSet<T1, TR...>& other)
    {
        return this->_mergeChanges(dynamic_cast<const _ChangesInDataSetBase<T1, TR...>& >(other));
    }

    ChangesInDataSet() = delete;
    ChangesInDataSet(ChangesInDataSet const&) = default;
    ChangesInDataSet& operator=(ChangesInDataSet const&) = delete;
    bool operator==(ChangesInDataSet const&) const = default;

    template<typename SH = StreamerHelper>
    inline void toCSV(std::ostream& oss, const SH& streamerHelper = SH{}) const
    {
      //oss << _source;
      this->template _toCSV<0>(oss, streamerHelper);
    }

    template<typename SH = StreamerHelper>
    inline std::string toCSV(
      typename std::enable_if_t<  !std::is_same_v<SH, std::ostream&>,
                                  const SH& >
      streamerHelper = SH{}) const
    {
      std::ostringstream oss;
      ChangesInDataSet<T1, TR...>::toCSV(oss, streamerHelper);
      return oss.str();
    }
  };

} }  //  datastructure::versionedObject
