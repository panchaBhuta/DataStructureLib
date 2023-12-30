/*
 * dataStructure.h
 *
 * URL:      https://github.com/panchaBhuta/dataStructure
 * Version:  v2.1.1
 *
 * Copyright (C) 2023-2023 Gautam Dhar
 * All rights reserved.
 * 
 * dataStructure is distributed under the BSD 3-Clause license, see LICENSE for details. 
 *
 */

#pragma once


#include <set>
#include <map>

#include <dataStructure.h>

#if FLAG_BIMAP_debug_log == 1
  #define BIMAP_DEBUG_LOG(aMessage) { std::cout << aMessage << " :: file:" << DATASTRUCTURE_PREFERRED_PATH << ":" << __LINE__ << std::endl; }
  #define BIMAP_DEBUG_TRY_START try {
  #define BIMAP_DEBUG_TRY_END   }
  #define BIMAP_DEBUG_TRY_CATCH(EXCEPTION_TYPE)                             \
      catch(const EXCEPTION_TYPE& ex) {                                     \
        BIMAP_DEBUG_LOG( "got-ERROR: " << ex.what() );                      \
        throw ex;                                                           \
      }
#else
  #define BIMAP_DEBUG_LOG(aMessage)
  #define BIMAP_DEBUG_TRY_START
  #define BIMAP_DEBUG_TRY_END
  #define BIMAP_DEBUG_TRY_CATCH(EXCEPTION_TYPE)
#endif


namespace datastructure { namespace bimap
{
  template < typename LEFTT, typename RIGHT>
  class BiMap_LR11_RL11
  {
  private:
    std::map<LEFTT, RIGHT>  _lefttMap;
    std::map<RIGHT, LEFTT>  _rightMap;

    const decltype(_lefttMap)& _cLefttMap;
    const decltype(_rightMap)& _cRightMap;

    const decltype(_lefttMap)::key_compare  _lefttKeyLessCompare;
    const decltype(_rightMap)::key_compare  _rightKeyLessCompare;

  public:
    using lefttMapIterator = typename std::map<LEFTT, RIGHT>::iterator;
    using rightMapIterator = typename std::map<RIGHT, LEFTT>::iterator;

    using const_lefttMapIterator = typename std::map<LEFTT, RIGHT>::const_iterator;
    using const_rightMapIterator = typename std::map<RIGHT, LEFTT>::const_iterator;

    /*
    enum InsertResult { INSERT_TRUE, LEFTT_RIGHT_EXISTS,
                        LEFTT_KEY_EXISTS, RIGHT_KEY_EXISTS
                      };
    */

    BiMap_LR11_RL11()
      : _lefttMap(),
        _rightMap(),
        _cLefttMap(_lefttMap),
        _cRightMap(_rightMap),
        _lefttKeyLessCompare(_lefttMap.key_comp()),
        _rightKeyLessCompare(_rightMap.key_comp())
    {}

    ~BiMap_LR11_RL11()
    {
      _lefttMap.clear();
      _rightMap.clear();
    }

    //inline std::pair<const_lefttMapIterator, InsertResult> insert_left(const LEFTT& lefttKey, const RIGHT& rightKey)
    inline std::pair<const_lefttMapIterator, bool> insert_left(const LEFTT& lefttKey, const RIGHT& rightKey)
    {
      // Returns an iterator pointing to the first element that is not less than (i.e. greater or equal to) key.
      const_lefttMapIterator lbSearchLefttMap = _cLefttMap.lower_bound(lefttKey);

      if( ( lbSearchLefttMap == _cLefttMap.cend() ) ||
          _lefttKeyLessCompare( lefttKey , (lbSearchLefttMap->first) ) )
      {
        // lefttKey NOT present in _lefttMap

        const std::pair<rightMapIterator, bool> insertRightMap = _rightMap.emplace(rightKey, lefttKey);
        if(insertRightMap.second)
        {
          // rightKey NOT present in _rightMap

          lefttMapIterator insertLefttMap = _lefttMap.emplace_hint(lbSearchLefttMap, lefttKey, rightKey);
          //return { const_lefttMapIterator(insertLefttMap), InsertResult::INSERT_TRUE };
#if FLAG_BIMAP_debug_log == 1
          if( insertLefttMap == _lefttMap.end() )
          {
            BIMAP_DEBUG_LOG("###################  UNEXPECTED condition experienced :  BiMap_LR11_RL11::insert_left(leftKey='" \
                            << lefttKey <<"') : failed insert for left-key");
          }
          if( _rightKeyLessCompare(rightKey,insertLefttMap->second) ||
              _rightKeyLessCompare(insertLefttMap->second,rightKey)  )  
          {
            // if(rightKey == insertLefttMap->second)
            BIMAP_DEBUG_LOG("###################  UNEXPECTED condition experienced :  BiMap_LR11_RL11::insert_left(rightKey='" \
                            << rightKey <<"') : failed insert, as existing rightKey='" << insertLefttMap->second << "'");
          }
#endif
          return { const_lefttMapIterator(insertLefttMap), true };
        } else {
          const_rightMapIterator rightMapSearch = insertRightMap.first;
          const LEFTT& existingLefttKey = rightMapSearch->second;
          const_lefttMapIterator revLefttMapSearch = _cLefttMap.find(existingLefttKey);
          //return { revLefttMapSearch, InsertResult::RIGHT_KEY_EXISTS };
          return { revLefttMapSearch, false };
        }
      } else {
        // lefttKey == (lbSearchLefttMap->first)

        /*
        const_rightMapIterator rightMapSearch = _cRightMap.find(righttKey);

        if(rightMapSearch == _cRightMap.cend())
        {
          return { lbSearchLeftMap, InsertResult::LEFTT_KEY_EXISTS };
        } else {
          return { lbSearchLeftMap, InsertResult::LEFTT_RIGHT_EXISTS };
        }
        */
        return { lbSearchLefttMap, false };
      }
    }

    inline const_lefttMapIterator find_left(const LEFTT& leftKey) const
    {
      return _cLefttMap.find(leftKey);
    }

    inline const_rightMapIterator find_right(const RIGHT& rightKey) const
    {
      return _cRightMap.find(rightKey);
    }

    inline bool isLeft_end(const_lefttMapIterator& checkEndIter) const
    {
      return checkEndIter == _cLefttMap.end();
    }

    inline bool isRight_end(const_rightMapIterator& checkEndIter) const
    {
      return checkEndIter == _cRightMap.end();
    }

    inline size_t size_left () const { return _lefttMap.size(); }
    inline size_t size_right() const { return _rightMap.size(); }
  };




  template < typename LEFTT, typename RIGHT>
  class BiMap_LR1M_RL11
  {
  private:
    std::map<LEFTT, std::set<RIGHT> >  _lefttMap1ToM;
    std::map<RIGHT, LEFTT>             _rightMap1To1;

    const decltype(_lefttMap1ToM)& _cLefttMap1ToM;
    const decltype(_rightMap1To1)& _cRightMap1To1;

    const decltype(_lefttMap1ToM)::key_compare  _lefttKeyLessCompare;
    const decltype(_rightMap1To1)::key_compare  _rightKeyLessCompare;

  public:
    using lefttMapIterator = std::map<LEFTT, std::set<RIGHT>>::iterator;
    using rightMapIterator = std::map<RIGHT, LEFTT>::iterator;
    using rightSetIterator = std::set<RIGHT     >::iterator;

    using const_lefttMapIterator = std::map<LEFTT, std::set<RIGHT>>::const_iterator;
    using const_rightMapIterator = std::map<RIGHT, LEFTT>::const_iterator;
    using const_rightSetIterator = std::set<RIGHT     >::const_iterator;


    BiMap_LR1M_RL11()
      : _lefttMap1ToM(),
        _rightMap1To1(),
        _cLefttMap1ToM(_lefttMap1ToM),
        _cRightMap1To1(_rightMap1To1),
        _lefttKeyLessCompare(_lefttMap1ToM.key_comp()),
        _rightKeyLessCompare(_rightMap1To1.key_comp())
    {}

    ~BiMap_LR1M_RL11()
    {
      for(auto& manySet : _lefttMap1ToM)
      {
        manySet.second.clear();
      }
      _lefttMap1ToM.clear();
      _rightMap1To1.clear();
    }

    inline std::pair<const_lefttMapIterator, bool> insert_left(const LEFTT& lefttKey, const RIGHT& rightKey)
    {
      const_rightMapIterator rightMapSearch = _cRightMap1To1.find(rightKey);
      if(rightMapSearch != _cRightMap1To1.end())
      {
        const LEFTT& existingLefttKey = rightMapSearch->second;
        //  lefttKey == existingLefttKey  :: same left-right exists, query left-Iterator and return false.
        //  lefttKey != existingLefttKey  :: different LEFT-right exists, query LEFT-Iterator and return false.
        //  NOTE : we just query 'revLefttMapSearch' using 'existingLefttKey', NO need to check if
        //         'existingLefttKey' is same as 'lefttKey'
        const_lefttMapIterator revLefttMapSearch = _cLefttMap1ToM.find(existingLefttKey);
        return { revLefttMapSearch, false };
      }
      // In rightMap : rightKey doesn't exists

      // if existing left-node use that ELSE create a new left-node.
      std::pair<lefttMapIterator, bool> insertLeftMap = _lefttMap1ToM.emplace(lefttKey, std::set<RIGHT>{} );
      // no need to check on insertLeftMap.second
      lefttMapIterator lefttMapSearch = insertLeftMap.first;
      std::set<RIGHT>& rightSetKeys = lefttMapSearch->second;

      const std::pair<rightSetIterator, bool> insertRightSet = rightSetKeys.emplace(rightKey);
      if(!insertRightSet.second)
      {
        if(rightSetKeys.size() == 0)
        {
          BIMAP_DEBUG_LOG("###################  UNEXPECTED condition experienced :  BiMap_LR1M_RL11::insert_left(rightKey='" \
                          << rightKey <<"') : failed insert in empty-SET for right-key");
          _lefttMap1ToM.erase(lefttMapSearch);
          return { _cLefttMap1ToM.end(), false };
        }
        return { const_lefttMapIterator(lefttMapSearch), insertRightSet.second };
      }

      const std::pair<rightMapIterator, bool> insertRightMap = _rightMap1To1.emplace(rightKey, lefttKey);
      if(!insertRightMap.second)
      {
        BIMAP_DEBUG_LOG("###################  UNEXPECTED condition experienced :  BiMap_LR1M_RL11::insert_left(rightKey='" \
                        << rightKey <<"') : failed insert for right-key");
        rightSetKeys.erase(insertRightSet.first);
        if(rightSetKeys.size() == 0)
        {
          _lefttMap1ToM.erase(lefttMapSearch);
          return { _cLefttMap1ToM.end(), false };
        }
      }
      return { const_lefttMapIterator(lefttMapSearch), insertRightSet.second };
    }

    inline const_lefttMapIterator find_left(const LEFTT& lefttKey) const
    {
      return _cLefttMap1ToM.find(lefttKey);
    }

    inline const_rightMapIterator find_right(const RIGHT& rightKey) const
    {
      return _cRightMap1To1.find(rightKey);
    }

    inline bool isLeft_end(const_lefttMapIterator& checkEndIter) const
    {
      return checkEndIter == _cLefttMap1ToM.end();
    }

    inline bool isRight_end(const_rightMapIterator& checkEndIter) const
    {
      return checkEndIter == _cRightMap1To1.end();
    }

    inline bool contains_left(const LEFTT& leftKey, const RIGHT& rightKey) const
    {
      const_lefttMapIterator primIter = _cLefttMap1ToM.find(leftKey);
      if(primIter == _cLefttMap1ToM.end())
        return false;

      return primIter->contains(rightKey);
    }

    inline bool contains_right(const RIGHT& rightKey, const LEFTT& leftKey) const
    {
      const_rightMapIterator secoIter = _cRightMap1To1.find(rightKey);
      if(secoIter == _cRightMap1To1.end())
        return false;

      return secoIter->contains(leftKey);
    }

    inline size_t size_left () const { return _lefttMap1ToM.size(); }
    inline size_t size_right() const { return _rightMap1To1.size(); }
  };

} }   //  namespace datastructure::bimap

