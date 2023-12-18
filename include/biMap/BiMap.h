/*
 * dataStructure.h
 *
 * URL:      https://github.com/panchaBhuta/dataStructure
 * Version:  v2.0.0
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


namespace bimap
{
  template < typename LEFTT, typename RIGHT>
  class BiMap_LR11_RL11
  {
  private:
    std::map<LEFTT, RIGHT>  _lefttMap;
    std::map<RIGHT, LEFTT>  _rightMap;

    const decltype(_lefttMap)& _cLefttMap;
    const decltype(_rightMap)& _cRightMap;

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
        _cRightMap(_rightMap)
    {}

    ~BiMap_LR11_RL11()
    {
      _lefttMap.clear();
      _rightMap.clear();
    }

    //inline std::pair<const_lefttMapIterator, InsertResult> insert_left(const LEFTT& lefttKey, const RIGHT& rightKey)
    inline std::pair<const_lefttMapIterator, bool> insert_left(const LEFTT& lefttKey, const RIGHT& rightKey)
    {
      const_lefttMapIterator lbSearchLefttMap = _cLefttMap.lower_bound(lefttKey);

      if( ( lbSearchLefttMap == _cLefttMap.cbegin() ) ||
          // Returns an iterator pointing to the first element that is not less than (i.e. greater or equal to) key.
          ( lefttKey < (lbSearchLefttMap->first) ))
      {
        // lefttKey NOT present in _lefttMap

        const std::pair<rightMapIterator, bool> insertRightMap = _rightMap.emplace(rightKey, lefttKey);
        if(insertRightMap.second)
        {
          // rightKey NOT present in _rightMap

          lefttMapIterator insertLefttMap = _lefttMap.emplace_hint(lbSearchLefttMap, lefttKey, rightKey);
          //return { const_lefttMapIterator(insertLefttMap), InsertResult::INSERT_TRUE };
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
  };




    /*
  template < typename LEFTT, typename RIGHT>
  class BiMap_LR1M_RL11
  {
  private:
    std::map<LEFTT, std::set<RIGHT> >  _lefttMap1ToM;
    std::map<RIGHT, LEFTT>             _rightMap1To1;

    const decltype(_lefttMap1ToM)& _cLefttMap1ToM;
    const decltype(_rightMap1To1)& _cRightMap1To1;

  public:
    using lefttMapIterator = std::map<LEFTT, std::set<RIGHT>>::iterator;
    using rightMapIterator = std::map<RIGHT, LEFTT>::iterator;
    using rightSetIterator = std::set<RIGHT     >::iterator;

    using const_lefttMapIterator = std::map<LEFTT, std::set<RIGHT>>::const_iterator;
    using const_rightMapIterator = std::map<RIGHT, LEFTT>::const_iterator;
    using const_rightSetIterator = std::set<RIGHT     >::const_iterator;


    BiMap_LR1M_RL11()
      : _lefttMap1ToM(), _rightMap1To1()
    {}

    ~BiMap_LR1M_RL11()
    {
      for(auto& manySet : _lefttMap1ToM)
      {
        manySet.clear();
      }
      _lefttMap1ToM.clear();
      _rightMap1To1.clear();
    }
      std::pair<rightMapIterator, bool> resultSecMap = _rightMap1To1.emplace(rightKey, lefttKey);
      if(!(resultSecMap.second))
        return {resultSecMap.first, false};

      bool deleteSet = false;
      {
        std::set<RIGHT>& rightSetKeys = _lefttMap1ToM[lefttKey];
        std::pair<rightSetIterator, bool> resultSecSet = rightSetKeys.emplace(rightKey);
        if(!(resultSecSet->second))
        {
          _rightMap1To1.erase(resultSecMap);
          deleteSet = (rightSetKeys.size() == 0);
        }
      }
      if(deleteSet)
        _lefttMap1ToM.erase(lefttKey);

      return { resultSecMap.first, resultSecMap.second };

    std::pair<const_lefttMapIterator, bool> insert_left(const LEFTT& right_Key, const RIGHT& leftt_Key)
    {
      const_lefttMapIterator lbSearchRight_Map = _cRight_Map.lower_bound(right_Key);

      if( ( lbSearchRight_Map == _cRight_Map.cbegin() ) ||
          // Returns an iterator pointing to the first element that is not less than (i.e. greater or equal to) key.
          ( right_Key < (lbSearchRight_Map->first) ))
      {
        // right_Key NOT present in _right_Map

        const std::pair<leftt_MapIterator, bool> insertLeftt_Map = _leftt_Map.emplace(leftt_Key, right_Key);
        if(insertLeftt_Map.second)
        {
          // leftt_Key NOT present in _leftt_Map

          right_MapIterator insertRight_Map = _right_Map.emplace_hint(lbSearchRight_Map, right_Key, leftt_Key);
          return { const_lefttMapIterator(insertRight_Map), true };  ?????
        } else {
          ??????
          const_rightMapIterator rightMapSearch = insertLeftt_Map.first;
          const LEFTT& existingLefttKey = rightMapSearch->second;
          const_lefttMapIterator revLefttMapSearch = _cLefttMap.find(existingLefttKey);
          return { revLefttMapSearch, false };
        }
      } else {
        // right_Key == (lbSearchRight_Map->first)
        return { lbSearchRight_Map, false };
      }
    }
      std::pair<rightMapIterator, bool> resultSecMap = _rightMap1To1.emplace(rightKey, lefttKey);
      if(!(resultSecMap.second))
        return {resultSecMap.first, false};

      bool deleteSet = false;
      {
        std::set<RIGHT>& rightSetKeys = _lefttMap1ToM[lefttKey];
        std::pair<rightSetIterator, bool> resultSecSet = rightSetKeys.emplace(rightKey);
        if(!(resultSecSet->second))
        {
          _rightMap1To1.erase(resultSecMap);
          deleteSet = (rightSetKeys.size() == 0);
        }
      }
      if(deleteSet)
        _lefttMap1ToM.erase(lefttKey);

      return { resultSecMap.first, resultSecMap.second };

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
  };
    */

}

