/*
 * VersionedObjectBuilder.h
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
#include <vector>
#include <set>
#include <map>
#include <stdexcept>
#include <iterator>
#include <utility>

#include <versionedObject/VersionedObject.h>
#include <versionedObject/SnapshotDataSet.h>
#include <versionedObject/ChangesInDataSet.h>


namespace datastructure { namespace versionedObject
{

  using FirstDeltaChange_Before_StartDate_Timeline_exception             =  VO_exception<1>;
  using Empty_VersionObject_exception                                    =  VO_exception<3>;
  using FirstVersion_Before_LastDeltaChange_ReverseTimeline_exception    =  VO_exception<4>;
  using GetVersionAt_isNull_exception                                    =  VO_exception<5>;
  using LastVersion_After_FirstDeltaChange_ForwardTimeline_exception     =  VO_exception<6>;
  using Unexpected_ApplicableChangeDirection_exception                   =  VO_exception<7>;

  template <typename VDT, typename ... MT>
  class _VersionedObjectBuilderBase
  {
  public:
    using t_deltaEntriesMap  = typename std::map < VDT, ChangesInDataSet<MT...> >;
    using t_deltaEntriesMap_iter_diff_type  = typename std::iterator_traits<
                                            typename t_deltaEntriesMap::const_iterator
                                        >::difference_type;

    using t_snapShotEntriesMap  = typename std::multimap < VDT, SnapshotDataSet<MT...> >;

  protected:
    using t_versionDate      = VDT;
    using t_versionedObject  = VersionedObject<VDT, MT...>;
    using t_dataset          = DataSet<MT...>;
    using t_record           = typename t_dataset::t_record;
    using t_metaData         = typename t_dataset::t_metaData;
    using t_datasetLedger    = typename t_versionedObject::t_datasetLedger;

    t_deltaEntriesMap     _deltaChgEntries;
    t_snapShotEntriesMap  _snapShotEntries;

    _VersionedObjectBuilderBase() : _deltaChgEntries(), _snapShotEntries() {}

    inline static t_dataset _datasetFactory(
                    [[maybe_unused]]  const t_deltaEntriesMap::mapped_type& changesInDataSet,
                                      const t_record& record)
    {
      if constexpr ( t_dataset::hasMetaData() )
      {
        return t_dataset(changesInDataSet.getMetaData(), record);
      } else {
        return t_dataset(record);
      }
    }

#if FLAG_VERSIONEDOBJECT_debug_log == 1
    void inline _logDeltaEntriesMap(const t_deltaEntriesMap& deltaEntries)
    {
      std::cout << "DEBUG_LOG:    _logDeltaEntriesMap(START)" << std::endl;
      for(auto iterDelta: deltaEntries)
      {
        std::cout << "DEBUG_LOG:    versionDate[" << iterDelta.first << "] : delta{";
        iterDelta.second.toCSV(std::cout);
        std::cout << "}" << std::endl;
      }
      std::cout << "DEBUG_LOG:    _logDeltaEntriesMap(END)" << std::endl;
    }

    void inline _logSnapEntriesMap(const t_snapShotEntriesMap& snapEntries)
    {
      std::cout << "DEBUG_LOG:    _logSnapEntriesMap(START)" << std::endl;
      for(auto iterSnap: snapEntries)
      {
        std::cout << "DEBUG_LOG:    versionDate[" << iterSnap.first << "] : snap{";
        iterSnap.second.toCSV(std::cout);
        std::cout << "}" << std::endl;
      }
      std::cout << "DEBUG_LOG:    _logSnapEntriesMap(END)" << std::endl;
    }
#endif

    void inline _updateComboDataSet(t_deltaEntriesMap& comboChgEntries)
    {
#if FLAG_VERSIONEDOBJECT_debug_log == 1
      std::cout << "DEBUG_LOG:  _VersionedObjectBuilderBase<VDT, MT...>::_updateComboDataSet(START)" << std::endl;
      std::cout << "DEBUG_LOG:  +++++before combo+++++++" << std::endl;
      _logDeltaEntriesMap(comboChgEntries);
      std::cout << "DEBUG_LOG:  ~~~~~~~~~~~~~~~~~~~~~~~~" << std::endl;
      _logSnapEntriesMap(_snapShotEntries);
      std::cout << "DEBUG_LOG:  -----before combo-------" << std::endl;
#endif
      for( auto snapIter : _snapShotEntries)
      {
        if( auto comboIter  = comboChgEntries.find(snapIter.first);
                 comboIter != comboChgEntries.end() )
        {
          comboIter->second.merge(snapIter.second);
        } else {
          comboChgEntries.emplace(snapIter.first, snapIter.second);
        }
      }
#if FLAG_VERSIONEDOBJECT_debug_log == 1
      std::cout << "DEBUG_LOG:  +++++after combo+++++++" << std::endl;
      _logDeltaEntriesMap(comboChgEntries);
      std::cout << "DEBUG_LOG:  -----after combo-------" << std::endl;
      std::cout << "DEBUG_LOG:  _VersionedObjectBuilderBase<VDT, MT...>::_updateComboDataSet(END)" << std::endl;
#endif
    }

    void _buildForwardTimeline( // with filled VersionObject
                  t_versionedObject& vo,
                  const t_deltaEntriesMap& comboChgEntries)
    {
      VERSIONEDOBJECT_DEBUG_LOG("DEBUG_LOG:  _VersionedObjectBuilderBase<VDT, MT...>::_buildForwardTimeline(START)");
#if FLAG_VERSIONEDOBJECT_debug_log == 1
      vo.toStr("DEBUG_LOG:    prebuild-VO: ", std::cout);
#endif

      /*
      _buildForwardTimeline
      vo{
        Version Objects, timeline ...
        last-VersionObject
      }
      Expected ::   first Combo-ChangeDelta version-date > last-VersionObject-date
      comboChgEntries{
        first Combo-ChangeDelta Version,
        remaining Combo-Change Delta Versions, timeline ...
        last-Combo-ChangeDelta version
      }
      */

      if(comboChgEntries.empty())
      {
        return;
      }

      if(vo.getDatasetLedger().empty())
      {
        std::ostringstream eoss;
        eoss << "ERROR(1) : failure in _VersionedObjectBuilderBase<VDT, MT...>::_buildForwardTimeline() : "
                "VersionObject cannot be empty." << std::endl;
        toStr(comboChgEntries, "VersionObjectBuilder :: ", eoss);
        throw Empty_VersionObject_exception(eoss.str());
      }

      for( auto iterDelta = comboChgEntries.begin(); iterDelta != comboChgEntries.end(); ++iterDelta )
      {
        if(iterDelta->second.getApplicableChangeDirection() == ApplicableChangeDirection::REVERSE)
        {
          std::ostringstream eoss;
          eoss << "ERROR(2) : failure in _VersionedObjectBuilderBase<VDT, MT...>::_buildForwardTimeline() : for changeDate[";
          eoss << iterDelta->first << "] should NOT be marked as 'ApplicableChangeDirection::REVERSE', expected FORWARD" << std::endl;
          toStr(comboChgEntries, "VersionObjectBuilder :: ", eoss);
          vo.toStr("VersionObject :: ", eoss);
          throw Unexpected_ApplicableChangeDirection_exception(eoss.str());
        }
      }

      auto iterDelta = comboChgEntries.begin();
      /*
      {
        auto lastVersionIter = vo.getDatasetLedger().rbegin();
        t_versionDate firstDeltaChangeDate = iterDelta->first;
        t_versionDate lastVersionDate = lastVersionIter->first;
        if( firstDeltaChangeDate <= lastVersionDate ) //  no overlap in ranges
        {
          std::ostringstream eoss;
          eoss << "ERROR(3) : failure in _VersionedObjectBuilderBase<VDT, MT...>::_buildForwardTimeline() : first-deltaChangeDate[";
          eoss << firstDeltaChangeDate << "] should be more than last-VersionDate[" << lastVersionDate << "]" << std::endl;
          vo.toStr("VersionObject :: ", eoss);
          toStr(comboChgEntries, "VersionObjectBuilder :: ", eoss);
          throw LastVersion_After_FirstDeltaChange_ForwardTimeline_exception(eoss.str());
        }
      }
      */

      std::array <bool, std::tuple_size_v< t_record > > hitheroProcessedElements;
      t_versionDate presentDeltaChangeDate = iterDelta->first;

      // forward traversal in time: present -> future
      while( iterDelta != comboChgEntries.end() )
      {
        presentDeltaChangeDate = iterDelta->first;
        typename t_versionedObject::t_datasetLedger::const_iterator
            ledgerIter = vo.getVersionBefore(presentDeltaChangeDate);

#if FLAG_VERSIONEDOBJECT_debug_log == 1
        std::cout << "DEBUG_LOG:    ** deltaEntryDate[" << iterDelta->first << "] >> versionObjectDate[" << std::flush;
        if( ledgerIter != vo.getDatasetLedger().end() )
        {
          std::cout << ledgerIter->first << std::flush;
        } else {
          std::cout << "ERROR:NULL-DATE" << std::flush;
        }
        std::cout << "]:data{" << ledgerIter->second.toCSV()
                  << "} << deltaChange{" << iterDelta->second.toCSV() << "}" << std::endl;
#endif

        if(ledgerIter == vo.getDatasetLedger().end())
        {
          std::ostringstream eoss;
          eoss << "ERROR(4) : failure in _VersionedObjectBuilderBase<VDT, MT...>::_buildForwardTimeline() : ";
          eoss << "Call to vo.getVersionAt(presentDeltaChangeDate=";
          eoss << presentDeltaChangeDate << ") has no version available in VersionObject." << std::endl;
          vo.toStr("VersionObject :: ", eoss);
          toStr(comboChgEntries, "VersionObjectBuilder :: ", eoss);
          throw GetVersionAt_isNull_exception(eoss.str());
        }
        t_record record { ledgerIter->second.getRecord() };

        try {
          hitheroProcessedElements.fill(false);
          // 'getLatestRecord()' is called with VALIDATE=true
          iterDelta->second.template getLatestRecord<true>(record, hitheroProcessedElements);

          t_dataset dataset = _datasetFactory(iterDelta->second, record);
          VERSIONEDOBJECT_DEBUG_MSG( "DEBUG_LOG: vo.insertVersion() -> versionDate: " << presentDeltaChangeDate << "; DATASET{" << dataset.toCSV() << "}");
          bool insertResult = vo.insertVersion( presentDeltaChangeDate, dataset );
          VERSIONEDOBJECT_DEBUG_MSG( "DEBUG_LOG: vo.insertVersion() insertResult=" << insertResult);
        } catch (const std::exception& err) {
          std::ostringstream eoss;
          eoss << "ERROR(5) : failure in _VersionedObjectBuilderBase<VDT, MT...>::_buildForwardTimeline() : "
               << err.what() << std::endl;
          toStr(comboChgEntries, "VersionObjectBuilder :: ", eoss);
          eoss << "DataSet :: ";
          ledgerIter->second.toCSV(eoss);
          throw std::invalid_argument(eoss.str());
        }

        ++iterDelta;
      }

#if FLAG_VERSIONEDOBJECT_debug_log == 1
      vo.toStr("DEBUG_LOG:    postbuild-VO: ", std::cout);
#endif
      VERSIONEDOBJECT_DEBUG_LOG("DEBUG_LOG:  _VersionedObjectBuilderBase<VDT, MT...>::_buildForwardTimeline(END)");
    }


    void _buildReverseTimeline( // with filled VersionObject
                  const t_versionDate& startDate,
                  t_versionedObject& vo,
                  const t_deltaEntriesMap& comboChgEntries)
    {
      VERSIONEDOBJECT_DEBUG_LOG("DEBUG_LOG:  _VersionedObjectBuilderBase<VDT, MT...>::_buildReverseTimeline(START) : startDate=" << startDate);
#if FLAG_VERSIONEDOBJECT_debug_log == 1
      vo.toStr("DEBUG_LOG:    prebuild-VO: ", std::cout);
#endif

      /*
      _buildReverseTimeline
      comboChgEntries{
        startDate of first ComboChangeDelta Version,
        remaining Combo-Change Delta Versions, timeline ...
        last-ComboChangeDelta version
      }
      Expected :: last-ComboChangeDelta version-date == first-VersionObject-date
      vo{
        first-VersionObject
        Version Objects, timeline ...
      }
      */

      if(comboChgEntries.empty())
      {
        return;
      }

      if(vo.getDatasetLedger().empty())
      {
        std::ostringstream eoss;
        eoss << "ERROR(1) : failure in _VersionedObjectBuilderBase<VDT, MT...>::_buildReverseTimeline() : "
                "VersionObject cannot be empty." << std::endl;
        toStr(comboChgEntries, "VersionObjectBuilder :: ", eoss);
        throw Empty_VersionObject_exception(eoss.str());
      }

      for( auto iterDelta = comboChgEntries.begin(); iterDelta != comboChgEntries.end(); ++iterDelta )
      {
        if(iterDelta->second.getApplicableChangeDirection() == ApplicableChangeDirection::FORWARD)
        {
          std::ostringstream eoss;
          eoss << "ERROR(2) : failure in _VersionedObjectBuilderBase<VDT, MT...>::_buildReverseTimeline() : for changeDate[";
          eoss << iterDelta->first << "] should NOT be marked as 'ApplicableChangeDirection::FORWARD', expected REVERSE" << std::endl;
          toStr(comboChgEntries, "VersionObjectBuilder :: ", eoss);
          vo.toStr("VersionObject :: ", eoss);
          throw Unexpected_ApplicableChangeDirection_exception(eoss.str());
        }
      }

      { // Similar check not applicable for  _buildForwardTimeline
        auto iterDelta = comboChgEntries.begin();
        t_versionDate firstDeltaChangeDate = iterDelta->first;
        if( startDate >= firstDeltaChangeDate )
        {
          std::ostringstream eoss;
          eoss << "ERROR(3) : failure in _VersionedObjectBuilderBase<VDT, MT...>::_buildReverseTimeline() : startDate[";
          eoss << startDate << "] should be less than first-changeDate[" << firstDeltaChangeDate << "]" << std::endl;
          eoss << "DataSet :: ";
          comboChgEntries.begin()->second.toCSV(eoss);
          toStr(comboChgEntries, "VersionObjectBuilder :: ", eoss);
          throw FirstDeltaChange_Before_StartDate_Timeline_exception(eoss.str());
        }
      }

      auto rIterDelta = comboChgEntries.rbegin();  // comboChgEntries is NOT empty, check done before
      /*
      {
        t_versionDate lastDeltaChangeDate = rIterDelta->first;
        t_versionDate firstVersionDate = vo.getDatasetLedger().begin()->first;
        if( lastDeltaChangeDate != firstVersionDate )
        {
          std::ostringstream eoss;
          eoss << "ERROR(4) : failure in _VersionedObjectBuilderBase<VDT, MT...>::_buildReverseTimeline() : last-changeDate[";
          eoss << lastDeltaChangeDate << "] should be equal to first-VersionDate[" << vo.getDatasetLedger().begin()->first << "]" << std::endl;
          vo.toStr("VersionObject :: ", eoss);
          toStr(comboChgEntries, "VersionObjectBuilder :: ", eoss);
          throw FirstVersion_Before_LastDeltaChange_ReverseTimeline_exception(eoss.str());
        }
      }
      */

      std::array <bool, std::tuple_size_v< t_record > > hitheroProcessedElements;
      auto rPastIterDelta = rIterDelta;
      t_versionDate presentDeltaChangeDate = rIterDelta->first;
      t_versionDate pastDeltaChangeDate = rIterDelta->first;

      // reverse traversal in time: present -> past
      while( rIterDelta != comboChgEntries.rend() )
      {
        presentDeltaChangeDate = rIterDelta->first;
        typename t_versionedObject::t_datasetLedger::const_iterator
                ledgerIter = vo.getVersionAt(presentDeltaChangeDate);

#if FLAG_VERSIONEDOBJECT_debug_log == 1
        std::cout << "DEBUG_LOG:    ** deltaEntryDate[" << presentDeltaChangeDate << "] >> versionObjectDate[" << std::flush;
        if( ledgerIter != vo.getDatasetLedger().cend() )
        {
          std::cout << ledgerIter->first << std::flush;
        } else {
          std::cout << "ERROR:NULL-DATE" << std::flush;
        }
        std::cout << "]:data{" << ledgerIter->second.toCSV()
                  << "} << deltaChange{" << rIterDelta->second.toCSV() << "}" << std::endl << std::flush;
#endif

        if(ledgerIter == vo.getDatasetLedger().cend())
        {
          std::ostringstream eoss;
          eoss << "ERROR(5) : failure in _VersionedObjectBuilderBase<VDT, MT...>::_buildReverseTimeline() : ";
          eoss << "Call to vo.getVersionAt(presentDeltaChangeDate=";
          eoss << presentDeltaChangeDate << ") has no version available in VersionObject." << std::endl;
          vo.toStr("VersionObject :: ", eoss);
          toStr(comboChgEntries, "VersionObjectBuilder :: ", eoss);
          throw GetVersionAt_isNull_exception(eoss.str());
        }
        t_record record { ledgerIter->second.getRecord() };

        try {
          hitheroProcessedElements.fill(false);
          // 'getPreviousRecord()' is called with VALIDATE=true
          rIterDelta->second.template getPreviousRecord<true>(record, hitheroProcessedElements);

          t_dataset dataset = _datasetFactory(rIterDelta->second, record);
          ++rPastIterDelta;
          pastDeltaChangeDate =   ( rPastIterDelta != comboChgEntries.rend() )  ?
                                    rPastIterDelta->first  :
                                    startDate;
          VERSIONEDOBJECT_DEBUG_MSG( "DEBUG_LOG: vo.insertVersion() -> versionDate: " << pastDeltaChangeDate << "; DATASET{" << dataset.toCSV() << "}");
          bool insertResult = vo.insertVersion( pastDeltaChangeDate, dataset );
          VERSIONEDOBJECT_DEBUG_MSG( "DEBUG_LOG: vo.insertVersion() insertResult=" << insertResult);
        } catch (const std::exception& err) {
          std::ostringstream eoss;
          eoss << "ERROR(6) : failure in _VersionedObjectBuilderBase<VDT, MT...>::_buildReverseTimeline() : "
               << err.what() << std::endl;
          toStr(comboChgEntries, "VersionObjectBuilder :: ", eoss);
          eoss << "DataSet :: ";
          ledgerIter->second.toCSV(eoss);
          throw std::invalid_argument(eoss.str());
        }

        ++rIterDelta;
      }

#if FLAG_VERSIONEDOBJECT_debug_log == 1
      vo.toStr("DEBUG_LOG:    postbuild-VO: ", std::cout);
#endif
      VERSIONEDOBJECT_DEBUG_LOG("DEBUG_LOG:  _VersionedObjectBuilderBase<VDT, MT...>::_buildReverseTimeline(END)");
    }

    template<typename T>
    static std::string _checkDateEnd(const T& dateMapIter, const T& dateMapEndIter )
    {
      if(dateMapIter != dateMapEndIter)
      {
        std::ostringstream oss;
        oss << dateMapIter->first;
        return oss.str();
      } else {
        return "END";
      }
    }

    //   pair< Number of entries processed, Number of entries pending>
    std::pair< t_deltaEntriesMap_iter_diff_type, t_deltaEntriesMap_iter_diff_type >
    _buildBiDirectionalTimeline( // with filled VersionObject
                  const std::vector<t_versionDate>& startDates,
                  t_versionedObject& vo,
                  const t_deltaEntriesMap& comboChgEntries)
    {
      VERSIONEDOBJECT_DEBUG_LOG("DEBUG_LOG:  _VersionedObjectBuilderBase<VDT, MT...>::_buildBiDirectionalTimeline(START)");
#if FLAG_VERSIONEDOBJECT_debug_log == 1
      std::cout << "DEBUG_LOG:    list of startDates[";
      for(auto sd : startDates) std::cout << sd << ",";
      std::cout << "]" << std::endl;
      vo.toStr("DEBUG_LOG:    prebuild-VO: ", std::cout);
#endif

      t_versionedObject initialStateVOcopy{vo};

      typename t_deltaEntriesMap::const_iterator   iterComboChgEntries     = comboChgEntries.cbegin();
      typename t_deltaEntriesMap::const_iterator   iterComboChgEntriesLast = comboChgEntries.cbegin();
      typename t_deltaEntriesMap::const_iterator   iterComboChgEntriesEnd  = comboChgEntries.cend();
      typename t_datasetLedger::const_iterator     iterISVOcopyBegin       = initialStateVOcopy.getDatasetLedger().cend();
      typename t_datasetLedger::const_iterator     iterISVOcopyEnd         = initialStateVOcopy.getDatasetLedger().cend();

      if(startDates.size() > 1) // altleast 2
      {
        for(unsigned int i = 1, j = 0; i < startDates.size(); ++i, ++j )
        {
          if(startDates[j] >= startDates[i])
          {
            std::ostringstream eoss;
            eoss << "ERROR(1) : failure in _VersionedObjectBuilderBase<VDT, MT...>::_buildBiDirectionalTimeline() : list of startDates[";
            for(auto sd : startDates) eoss << sd << ",";
            eoss << "]. For startDate[" << j << "]=" << startDates[j] << " should be less than startDate[";
            eoss << i << "]=" << startDates[i] << std::endl;
            vo.toStr("VersionObject :: ", eoss);
            toStr(comboChgEntries, "VersionObjectBuilder :: ", eoss);
            throw std::out_of_range(eoss.str());
          }
        }
      }

      size_t startDateIter = 0;
      bool changeProcessed = true;

      while( changeProcessed && iterComboChgEntries != comboChgEntries.cend() )
      {
        changeProcessed = false;

        if(    iterComboChgEntries != comboChgEntries.cend()
           &&  iterComboChgEntries->second.getApplicableChangeDirection() == ApplicableChangeDirection::REVERSE
          )   // Reverse Timeline
        {
          VERSIONEDOBJECT_DEBUG_MSG("DEBUG_LOG:   ReverseTimeline1 initialization  START <<<<<<<<<<<<<<<<<");
          _VersionedObjectBuilderBase<VDT, MT...> tempVOB{};

          iterISVOcopyBegin = initialStateVOcopy.getDatasetLedger().lower_bound(startDates[startDateIter]);

          t_versionDate reverseBuildEndDate{};

          iterComboChgEntriesEnd = // exclusive
                    ( (startDateIter+1) < startDates.size() ?
                      comboChgEntries.upper_bound(startDates[startDateIter+1]) :
                      comboChgEntries.cend() );

          iterISVOcopyEnd =  // exclusive
          //NOTE : there can be 2 consecutive startDates, but following REVERSE-FORWARD-REVERSE configuration
                    ( (startDateIter+1) < startDates.size() ?
                      initialStateVOcopy.getDatasetLedger().upper_bound(startDates[startDateIter+1]) :
                      initialStateVOcopy.getDatasetLedger().cend() );

#if FLAG_VERSIONEDOBJECT_debug_log == 1
VERSIONEDOBJECT_DEBUG_MSG("DEBUG_LOG:   iterComboChgEntries->first = " << _checkDateEnd(iterComboChgEntries, comboChgEntries.cend()) \
    << "  : iterComboChgEntriesEnd->first(tentative) = " << _checkDateEnd(iterComboChgEntriesEnd, comboChgEntries.cend()) );
VERSIONEDOBJECT_DEBUG_MSG("DEBUG_LOG:   iterISVOcopyBegin->first = " << _checkDateEnd(iterISVOcopyBegin, initialStateVOcopy.getDatasetLedger().cend()) \
    << "  : iterISVOcopyEnd->first(tentative) = " << _checkDateEnd(iterISVOcopyEnd, initialStateVOcopy.getDatasetLedger().cend()) );
          auto iterComboChgEntriesFirst = iterComboChgEntries;
#endif
          iterComboChgEntriesLast = iterComboChgEntries;
          t_versionedObject reverseBuildVO{};
          while(    iterComboChgEntries != iterComboChgEntriesEnd
                &&  iterComboChgEntries->second.getApplicableChangeDirection() == ApplicableChangeDirection::REVERSE
               )
          {
            tempVOB.insertDeltaVersion(iterComboChgEntries->first, iterComboChgEntries->second);
            iterComboChgEntriesLast = iterComboChgEntries;
            changeProcessed = true;
            ++iterComboChgEntries;
          }

          iterComboChgEntriesEnd = // exclusive
                  ( iterComboChgEntriesLast != comboChgEntries.cend() ?
                    std::next(iterComboChgEntriesLast) :
                    comboChgEntries.cend() );

          iterISVOcopyEnd =  // exclusive
                  ( iterComboChgEntriesLast != comboChgEntries.cend() ?
                    initialStateVOcopy.getDatasetLedger().upper_bound(iterComboChgEntriesLast->first) :
                    initialStateVOcopy.getDatasetLedger().cend() );
/*
          if( iterISVOcopyEnd != initialStateVOcopy.getDatasetLedger().cend() &&
              iterISVOcopyEnd == iterISVOcopyBegin )
          {
            VERSIONEDOBJECT_DEBUG_MSG("DEBUG_LOG:   WARNING : iterISVOcopyEnd->first = " << iterISVOcopyEnd->first \
                  << " is equal to iterISVOcopyBegin->first. Incrementing iterISVOcopyEnd." );
            ++iterISVOcopyEnd;
          }
*/

#if FLAG_VERSIONEDOBJECT_debug_log == 1
          auto iterISVOcopyLast = iterISVOcopyEnd;
          if( iterISVOcopyEnd != initialStateVOcopy.getDatasetLedger().cbegin()) --iterISVOcopyLast;
          VERSIONEDOBJECT_DEBUG_MSG("DEBUG_LOG:   " \
               "{iterComboChgEntriesStart->first[" << _checkDateEnd(iterComboChgEntriesFirst, comboChgEntries.cend()) \
            << "]..iterComboChgEntriesLast->first[" << _checkDateEnd(iterComboChgEntriesLast, comboChgEntries.cend()) << "]}" );
          VERSIONEDOBJECT_DEBUG_MSG("DEBUG_LOG:   " \
            << "{iterISVOcopyStart->first[" << _checkDateEnd(iterISVOcopyBegin, initialStateVOcopy.getDatasetLedger().cend()) \
            << "]..iterISVOcopyLast->first[" << _checkDateEnd(iterISVOcopyLast, initialStateVOcopy.getDatasetLedger().cend()) << "]}" );
#endif

          for(auto iterISVOcopy = iterISVOcopyBegin; iterISVOcopy != iterISVOcopyEnd; ++iterISVOcopy )
          {
            VERSIONEDOBJECT_DEBUG_MSG( "DEBUG_LOG: reverseBuildVO.insertVersion() -> versionDate: " << iterISVOcopy->first << "; DATASET{" << iterISVOcopy->second.toCSV() << "}");
            //bool insertResult =
            reverseBuildVO.insertVersion(iterISVOcopy->first, iterISVOcopy->second);
            //VERSIONEDOBJECT_DEBUG_MSG( "DEBUG_LOG: reverseBuildVO.insertVersion() insertResult=" << insertResult); is always true
          }

          t_deltaEntriesMap tmpComboChgEntries{tempVOB._deltaChgEntries};
          tempVOB._updateComboDataSet(tmpComboChgEntries);

          if(startDateIter >= startDates.size())
          {
            std::ostringstream eoss;
            eoss << "ERROR(3) : failure in _VersionedObjectBuilderBase<VDT, MT...>::_buildBiDirectionalTimeline() : list of startDates[";
            for(auto sd : startDates) eoss << sd << ",";
            eoss << "] has insufficient dates." << std::endl;
            vo.toStr("VersionObject :: ", eoss);
            toStr(tmpComboChgEntries, "VersionObjectBuilder :: ", eoss);
            tmpComboChgEntries.clear();
            throw std::out_of_range(eoss.str());
          }

          tempVOB._buildReverseTimeline(startDates[startDateIter++], // use 0, then increment to 1
                                        reverseBuildVO, tmpComboChgEntries);
          tmpComboChgEntries.clear();

          size_t count = 1; // count is 1 NOT zero, as the last valid element in 'reverseBuildVO' is seed,
                            // and already present in 'vo'
          for(auto  partialResultIter  = reverseBuildVO.getDatasetLedger().cbegin();
                    count < reverseBuildVO.getDatasetLedger().size();
                  ++partialResultIter, ++count)
          {
            vo.insertVersion(partialResultIter->first, partialResultIter->second);
          }
          VERSIONEDOBJECT_DEBUG_MSG("DEBUG_LOG:   ReverseTimeline1 initialization  END <<<<<<<<<<<<<<<<<");
        } // else ::: DONOT club with if(iterComboChgEntries->first > iterISVOcopy->first) ::: keep these 2 blocks disjoint



        if(    iterComboChgEntries != comboChgEntries.cend()
           &&  iterComboChgEntries->second.getApplicableChangeDirection() == ApplicableChangeDirection::FORWARD
          )   // Forward Timeline
        {
          VERSIONEDOBJECT_DEBUG_MSG("DEBUG_LOG:   ForwardTimeline2 initialization   START >>>>>>>>>>>>>>>>");
          _VersionedObjectBuilderBase<VDT, MT...> tempVOB{};

          iterISVOcopyBegin =
                    ( iterComboChgEntries != comboChgEntries.cend() ?
                      initialStateVOcopy.getDatasetLedger().lower_bound(iterComboChgEntries->first) :
                      initialStateVOcopy.getDatasetLedger().cend() );
          if(iterISVOcopyBegin != initialStateVOcopy.getDatasetLedger().cbegin())
          {
            --iterISVOcopyBegin;  //  Get the start seed for FORWARD. This Version-Date is before first Change-date.
          }

#if FLAG_VERSIONEDOBJECT_debug_log == 1
          auto iterComboChgEntriesFirst = iterComboChgEntries;
#endif
          iterComboChgEntriesLast = iterComboChgEntries;
          while(    iterComboChgEntries != comboChgEntries.cend()
                &&  iterComboChgEntries->second.getApplicableChangeDirection() == ApplicableChangeDirection::FORWARD
               )
          {
            tempVOB.insertDeltaVersion(iterComboChgEntries->first, iterComboChgEntries->second);
            iterComboChgEntriesLast = iterComboChgEntries;
            ++iterComboChgEntries;
            changeProcessed = true;
          }
          iterISVOcopyEnd =   // This Version-Date is after(i.e exclusive as it's .cend()) the last-valid Change-Date.
                    ( iterComboChgEntriesLast != comboChgEntries.cend() ?
                      initialStateVOcopy.getDatasetLedger().upper_bound(iterComboChgEntriesLast->first) :
                      initialStateVOcopy.getDatasetLedger().cend() );

#if FLAG_VERSIONEDOBJECT_debug_log == 1
          auto iterISVOcopyLast = iterISVOcopyEnd;
          if(iterISVOcopyEnd != initialStateVOcopy.getDatasetLedger().cbegin()) --iterISVOcopyLast;
          VERSIONEDOBJECT_DEBUG_MSG("DEBUG_LOG:   forward-applicable ::  " \
                "{iterComboChgEntriesStart->first[" << _checkDateEnd(iterComboChgEntriesFirst, comboChgEntries.cend()) \
            << "]..iterComboChgEntriesLast->first[" << _checkDateEnd(iterComboChgEntriesLast, comboChgEntries.cend()) \
            << "]} ==>> {iterISVOcopyStart->first[" << _checkDateEnd(iterISVOcopyBegin, initialStateVOcopy.getDatasetLedger().cend()) \
            << "]..iterISVOcopyLast->first[" << _checkDateEnd(iterISVOcopyLast, initialStateVOcopy.getDatasetLedger().cend()) << "]}" );
#endif

          t_versionedObject forwardBuildVO{};
          while( iterISVOcopyBegin != iterISVOcopyEnd )
          {
            VERSIONEDOBJECT_DEBUG_MSG( "DEBUG_LOG: forwardBuildVO.insertVersion() -> versionDate: " << iterISVOcopyBegin->first << "; DATASET{" << iterISVOcopyBegin->second.toCSV() << "}");
            //bool insertResult =
            forwardBuildVO.insertVersion(iterISVOcopyBegin->first, iterISVOcopyBegin->second);
            // VERSIONEDOBJECT_DEBUG_MSG( "DEBUG_LOG: forwardBuildVO.insertVersion() insertResult=" << insertResult); is always true
            ++iterISVOcopyBegin;
          }

          t_deltaEntriesMap tmpComboChgEntries{tempVOB._deltaChgEntries};
          tempVOB._updateComboDataSet(tmpComboChgEntries);
          tempVOB._buildForwardTimeline(forwardBuildVO, tmpComboChgEntries);
          tmpComboChgEntries.clear();

          auto  partialResultIter  = forwardBuildVO.getDatasetLedger().cbegin();
          ++partialResultIter; // skip the seed
          for(  ;
                partialResultIter != forwardBuildVO.getDatasetLedger().cend();
              ++partialResultIter)
          {
            vo.insertVersion(partialResultIter->first, partialResultIter->second);
          }
          VERSIONEDOBJECT_DEBUG_MSG("DEBUG_LOG:   ForwardTimeline2 initialization   END >>>>>>>>>>>>>>>>");
        }
      }


#if FLAG_VERSIONEDOBJECT_debug_log == 1
      vo.toStr("DEBUG_LOG:    postbuild-VO: ", std::cout);
#endif

      if(iterComboChgEntries != comboChgEntries.cend())
      {
        std::cout << "ERROR(2) : failure in _VersionedObjectBuilderBase<VDT, MT...>::_buildBiDirectionalTimeline() : unprocessed Changes for dates[";
        for(;iterComboChgEntries != comboChgEntries.cend(); ++iterComboChgEntries) std::cout << iterComboChgEntries->first << ",";
        std::cout << "]" << std::endl;
      }
      VERSIONEDOBJECT_DEBUG_LOG("DEBUG_LOG:  _VersionedObjectBuilderBase<VDT, MT...>::_buildBiDirectionalTimeline(END)");

      return std::pair< t_deltaEntriesMap_iter_diff_type, t_deltaEntriesMap_iter_diff_type > {
                std::distance(comboChgEntries.cbegin() , iterComboChgEntries),
                std::distance(iterComboChgEntries , comboChgEntries.cend())  };
    }

  public:
  inline bool insertDeltaVersion(const t_versionDate& forDate,
                                   const ChangesInDataSet<MT...>& chgEntry)
    {
      //VERSIONEDOBJECT_DEBUG_LOG("DEBUG_LOG:  _VersionedObjectBuilderBase<VDT, MT...>::insertDeltaVersion(START)");
#if FLAG_VERSIONEDOBJECT_debug_log == 1
      std::cout << "DEBUG_LOG:  insert ChangesInDataSet<MT...> : versionDate=" << forDate << " : dataset={";
      chgEntry.toCSV(std::cout);
#endif

      std::pair<typename t_snapShotEntriesMap::iterator, typename t_snapShotEntriesMap::iterator>
          snapshotSearchRange = _snapShotEntries.equal_range(forDate);

      std::vector<typename t_snapShotEntriesMap::iterator> snapshotIsSubset;
      if(snapshotSearchRange.first != _snapShotEntries.end() &&
         snapshotSearchRange.first != snapshotSearchRange.second)
      {
        for(typename t_snapShotEntriesMap::iterator iterSnapShot = snapshotSearchRange.first;
              iterSnapShot != snapshotSearchRange.second; ++iterSnapShot)
        {
          const SnapshotDataSet<MT...>& snpEntry = iterSnapShot->second;
          /* this is not applicable
          if(chgEntry.getApplicableChangeDirection() != snpEntry.getApplicableChangeDirection())
          {
            std::ostringstream eoss;
            eoss << "ERROR : failure in _VersionedObjectBuilderBase<VDT, MT...>::insertDeltaVersion() : versionDate=" << forDate;
            eoss << " has ApplicableChangeDirection mismatch." << std::endl;
            eoss << "ChangesInDataSet<MT...>={";
            chgEntry.toCSV(eoss);
            eoss << "} ApplicableChangeDirection="
                 << (chgEntry.getApplicableChangeDirection() == ApplicableChangeDirection::FORWARD ?
                     "FORWARD" : "REVERSE") << std::endl;
            eoss << "SnapshotDataSet<MT...>={";
            snpEntry.toCSV(eoss);
            eoss << "} ApplicableChangeDirection="
                 << (snpEntry.getApplicableChangeDirection() == ApplicableChangeDirection::FORWARD ?
                     "FORWARD" : "REVERSE") << std::endl;
            throw std::invalid_argument(eoss.str());
          }
          */
          if( chgEntry.isSubset(snpEntry) > 0 ) // also checks for equality in values where applicable
          {
            snapshotIsSubset.push_back(iterSnapShot);
          }
        }
      }

      std::pair<typename t_deltaEntriesMap::iterator, bool> insertResult =
                  _deltaChgEntries.insert({forDate, chgEntry});
      const bool isInsertSuccess = insertResult.second;
      if (!isInsertSuccess)
      {
#if FLAG_VERSIONEDOBJECT_debug_log == 1
        std::cout << "} : dataset exists, Insert failed" << std::endl;
#endif
        typename t_deltaEntriesMap::iterator& existingIter = insertResult.first;
        ChangesInDataSet<MT...>& existingChgDataSet = existingIter->second;
        try {
#if FLAG_VERSIONEDOBJECT_debug_log == 1
        std::cout << "DEBUG_LOG:    existingChgDataSet={";
        existingChgDataSet.toCSV(std::cout);
        std::cout << "}" << std::endl;
#endif
          existingChgDataSet.merge(chgEntry);
#if FLAG_VERSIONEDOBJECT_debug_log == 1
        std::cout << "DEBUG_LOG:    merged-existingChgDataSet={";
        existingChgDataSet.toCSV(std::cout);
        std::cout << "}" << std::endl;
#endif
        } catch (const std::invalid_argument& err) {
          std::ostringstream eoss;
          eoss << "catch-ERROR : _VersionedObjectBuilderBase<VDT, MT...>::insertDeltaVersion() : forDate="
               << forDate << "\nChangesInDataSet<MT...>={";
          chgEntry.toCSV(eoss);
          eoss << "}" << std::endl << err.what() << std::endl;
          throw err;
        }
      } else {
#if FLAG_VERSIONEDOBJECT_debug_log == 1
        std::cout << "} : Insert success" << std::endl;
#endif
      }

      for(typename t_snapShotEntriesMap::iterator iterSnapShot : snapshotIsSubset)
      {
#if FLAG_VERSIONEDOBJECT_debug_log == 1
        std::cout << "DEBUG_LOG:    removing matching data SnapShotDataSet<MT...>";
        std::cout << " : versionDate=" << iterSnapShot->first << " : dataset={";
        iterSnapShot->second.toCSV(std::cout);
        std::cout << "}" << std::endl;
#endif

        _snapShotEntries.erase(iterSnapShot);
      }
      snapshotIsSubset.clear();

      //VERSIONEDOBJECT_DEBUG_LOG("DEBUG_LOG:  _VersionedObjectBuilderBase<VDT, MT...>::insertDeltaVersion(END)");
      return isInsertSuccess;
    }

    inline bool insertSnapshotVersion(const t_versionDate& forDate,
                                      const SnapshotDataSet<MT...>& snpEntry)
    {
      //VERSIONEDOBJECT_DEBUG_LOG("DEBUG_LOG:  _VersionedObjectBuilderBase<VDT, MT...>::insertSnapshotVersion(START)");
#if FLAG_VERSIONEDOBJECT_debug_log == 1
      std::cout << "DEBUG_LOG:  insert SnapshotDataSet<MT...> : versionDate=" << forDate << " : dataset={";
      snpEntry.toCSV(std::cout);
#endif

      typename t_deltaEntriesMap::iterator deltaChgSearch = _deltaChgEntries.find(forDate);
      if(deltaChgSearch != _deltaChgEntries.end())
      {
        const ChangesInDataSet<MT...>& deltaChg = deltaChgSearch->second;
        /* this is not applicable
        if(deltaChg.getApplicableChangeDirection() != snpEntry.getApplicableChangeDirection())
        {
          std::ostringstream eoss;
          eoss << "ERROR : failure in _VersionedObjectBuilderBase<VDT, MT...>::insertSnapshotVersion() : versionDate=" << forDate;
          eoss << " has ApplicableChangeDirection mismatch." << std::endl;
          eoss << "ChangesInDataSet<MT...>={";
          deltaChg.toCSV(eoss);
          eoss << "} ApplicableChangeDirection="
                << (deltaChg.getApplicableChangeDirection() == ApplicableChangeDirection::FORWARD ?
                    "FORWARD" : "REVERSE") << std::endl;
          eoss << "SnapshotDataSet<MT...>={";
          snpEntry.toCSV(eoss);
          eoss << "} ApplicableChangeDirection="
                << (snpEntry.getApplicableChangeDirection() == ApplicableChangeDirection::FORWARD ?
                    "FORWARD" : "REVERSE") << std::endl;
          throw std::invalid_argument(eoss.str());
        }
        */
        try {
          if(deltaChg.isSubset(snpEntry) > 0)
          {
#if FLAG_VERSIONEDOBJECT_debug_log == 1
            std::cout << "} skipped as it's a subset of ..." << std::endl;
            std::cout << "DEBUG_LOG:    ChangesInDataSet<MT...>={";
            deltaChg.toCSV(std::cout);
            std::cout << "}" << std::endl;
#endif
            // since snpEntry is mergable, it's data already exists in _deltaChgEntries.
            // no need to add this seperately to _snapShotEntries.
            return false;
          }
        } catch (const std::invalid_argument& err) {
          std::ostringstream eoss;
          eoss << "catch-ERROR(1) : _VersionedObjectBuilderBase<VDT, MT...>::insertSnapshotVersion() : forDate="
              << forDate << "\nSnapshotDataSet<MT...>={";
          snpEntry.toCSV(eoss);
          eoss << "}" << std::endl << err.what() << std::endl;
          throw err;
        }
      }

      std::pair<typename t_snapShotEntriesMap::iterator, typename t_snapShotEntriesMap::iterator>
          snapshotSearchRange = _snapShotEntries.equal_range(forDate);

      if(snapshotSearchRange.first != _snapShotEntries.end() &&
         snapshotSearchRange.first != snapshotSearchRange.second)
      {
        for(typename t_snapShotEntriesMap::iterator iter = snapshotSearchRange.first;
              iter != snapshotSearchRange.second; ++iter)
        {
          const SnapshotDataSet<MT...>& chkSnpEntry = iter->second;
          try {
            if( chkSnpEntry.isSubset(snpEntry) > 0 ||
                snpEntry.isSubset(chkSnpEntry) > 0 )
            {
#if FLAG_VERSIONEDOBJECT_debug_log == 1
            std::cout << "} skipped as it's a subset of ..." << std::endl;
            std::cout << "DEBUG_LOG:    SnapshotDataSet<MT...>={";
            chkSnpEntry.toCSV(std::cout);
            std::cout << "}" << std::endl;
#endif
              return false;
            }
          } catch (const std::invalid_argument& err) {
            std::ostringstream eoss;
            eoss << "catch-ERROR(2) : _VersionedObjectBuilderBase<VDT, MT...>::insertSnapshotVersion() : forDate="
                << forDate << "\nSnapshotDataSet<MT...>={";
            snpEntry.toCSV(eoss);
            eoss << "}" << std::endl << err.what() << std::endl;
            throw err;
          }
        }
      }

      //VERSIONEDOBJECT_DEBUG_LOG("DEBUG_LOG:  _VersionedObjectBuilderBase<VDT, MT...>::insertSnapshotVersion(END)");
      bool retn = (_snapShotEntries.insert({forDate, snpEntry}) != _snapShotEntries.end());
#if FLAG_VERSIONEDOBJECT_debug_log == 1
      if(retn)
      {
            std::cout << "} : Insert success" << std::endl;
      } else {
            std::cout << "} : Insert fails as it already exists" << std::endl;
      }
#endif
      return retn;
    }

    /*
    inline void removeHistoricalDeltaVersionArtifacts(
                    const t_versionedObject& historicalVO,
                    std::set<t_versionDate>& datesDeltaChangesExistOrg,
                    std::set<t_versionDate>& datesDeltaChangesMismatch)
    {
      std::set<t_versionDate> datesDeltaChangesExist;
      const t_datasetLedger& datasetLedger = historicalVO.getDatasetLedger();

      typename t_deltaEntriesMap::const_iterator iterDelta = _deltaChgEntries.cbegin(); // map
      while( iterDelta != _deltaChgEntries.cend() )
      {
        t_versionDate deltaChangeDate = iterDelta->first;
        typename t_datasetLedger::const_iterator iterDatasetLedger = datasetLedger.find(deltaChangeDate);
        if( iterDatasetLedger != datasetLedger.cend() )
        {
          if(iterDelta->second.isSnapShot())
          {
            if(iterDelta->second.isSnapshotChgValueEqual(iterDatasetLedger->second.getRecord()))
            {
                  //std::cout << "#### PATHTEST : SNAPSHOT : insert : " << deltaChangeDate << std::endl;
              datesDeltaChangesExist.insert(deltaChangeDate);
            } else {
                  //std::cout << "#### PATHTEST : SNAPSHOT : erase : " << deltaChangeDate << std::endl;
              // since _deltaChgEntries is multi-map, possible that 'datesDeltaChangesExist'
              // has same 'deltaChangeDate' inserted on previous iteration loop
              datesDeltaChangesExist.erase(deltaChangeDate);
              datesDeltaChangesMismatch.insert(deltaChangeDate);
              return;
            }
          } else {  // NOT a snapshot
            if(iterDelta->second.getApplicableChangeDirection() == ApplicableChangeDirection::FORWARD)
            {
              typename t_datasetLedger::const_iterator iterNextDatasetLedger = iterDatasetLedger;
              ++iterNextDatasetLedger;
              if( iterNextDatasetLedger != datasetLedger.cend() )
              {
                t_versionDate deltaChangeNextDate = iterNextDatasetLedger->first;
                if( iterDelta->second.isNextChgValueEqual_deltaChange(iterNextDatasetLedger->second.getRecord()) &&
                    iterDelta->second.isPreviousChgValueEqual_deltaChange(iterDatasetLedger->second.getRecord())
                  )
                {
                  //std::cout << "#### PATHTEST : FORWARD : insert : " << deltaChangeNextDate << std::endl;
                  datesDeltaChangesExist.insert(deltaChangeNextDate);
                } else {
                  //std::cout << "#### PATHTEST : FORWARD : erase : " << deltaChangeNextDate << std::endl;
                  // since _deltaChgEntries is multi-map, possible that 'datesDeltaChangesExist'
                  // has same 'deltaChangeNextDate' inserted on previous iteration loop
                  datesDeltaChangesExist.erase(deltaChangeNextDate);
                  datesDeltaChangesMismatch.insert(deltaChangeNextDate);
                  return;
                }
              } else { // last iterator
                // do nothing as it's probably new delta that needs to be applied later
                  //std::cout << "#### PATHTEST : FORWARD : donothing : " << deltaChangeDate << std::endl;
              }
            } else { // REVERSE direction
              if( iterDatasetLedger != datasetLedger.cbegin() )
              {
                typename t_datasetLedger::const_iterator iterPrevDatasetLedger = iterDatasetLedger;
                --iterPrevDatasetLedger;
                t_versionDate deltaChangePrevDate = iterPrevDatasetLedger->first;
                if( iterDelta->second.isNextChgValueEqual_deltaChange(iterDatasetLedger->second.getRecord()) &&
                    iterDelta->second.isPreviousChgValueEqual_deltaChange(iterPrevDatasetLedger->second.getRecord())
                  )
                {
                  //std::cout << "#### PATHTEST : REVERSE : insert : " << deltaChangePrevDate << std::endl;
                  datesDeltaChangesExist.insert(deltaChangePrevDate);
                } else {
                  //std::cout << "#### PATHTEST : REVERSE : erase : " << deltaChangePrevDate << std::endl;
                  // since _deltaChgEntries is multi-map, possible that 'datesDeltaChangesExist'
                  // has same 'deltaChangePrevDate' inserted on previous iteration loop
                  datesDeltaChangesExist.erase(deltaChangePrevDate);
                  datesDeltaChangesMismatch.insert(deltaChangePrevDate);
                  return;
                }
              } else {  // first iterator
                // do nothing as it's probably new delta that needs to be applied later
                  //std::cout << "#### PATHTEST : REVERSE : donothing : " << deltaChangeDate << std::endl;
              }
            }
          }
        } else {
          // do nothing as it's probably new delta that needs to be applied later
                  //std::cout << "#### PATHTEST : No DataSet Found : donothing : " << deltaChangeDate << std::endl;
        }
        ++iterDelta;
      }

      //remove existing&matching deltaChanges from multimap
      for(auto matchedDeltaChangeDate : datesDeltaChangesExist)
      {
        _deltaChgEntries.erase(matchedDeltaChangeDate);
      }
      datesDeltaChangesExistOrg.merge(datesDeltaChangesExist);
    }
    */

    inline void toCSV(const t_deltaEntriesMap& comboChgEntries, const std::string& prefix, std::ostream& oss) const
    {
      for(auto iter : comboChgEntries)
      {
        oss << prefix << iter.first << ",";
        iter.second.toCSV(oss);
        oss << std::endl;
      }
    }

    inline void toCSV(const t_deltaEntriesMap& comboChgEntries, std::ostream& oss) const
    {
      for(auto iter : comboChgEntries)
      {
        oss << iter.first << ",";
        iter.second.toCSV(oss);
        oss << std::endl;
      }
    }

    inline void toStr(const t_deltaEntriesMap& comboChgEntries, const std::string& prefix, std::ostream& oss) const
    {
      for(auto iter : comboChgEntries)
      {
        oss << prefix << "versionDate=" << iter.first << ", deltaEntry = {";
        iter.second.toCSV(oss);
        oss << "}" << std::endl;
      }
    }

    inline void toStr(const t_deltaEntriesMap& comboChgEntries, std::ostream& oss) const
    {
      for(auto iter : comboChgEntries)
      {
        oss << "versionDate=" << iter.first << ", deltaEntry = {";
        iter.second.toCSV(oss);
        oss << "}" << std::endl;
      }
    }

    //inline const t_deltaEntriesMap& getDeltaChangeMap() const { return _deltaChgEntries; }

    virtual ~_VersionedObjectBuilderBase()
    {
      clear();
    }

    inline void clear()
    {
      _deltaChgEntries.clear();
      _snapShotEntries.clear();
    }
  };

  template <typename VDT, typename M, typename ... T>
  class VersionedObjectBuilder;

  template <typename VDT, c_MetaData M, typename ... T>
  class VersionedObjectBuilder<VDT, M, T...> : public _VersionedObjectBuilderBase<VDT, M, T...>
  {
  public:
    using t_versionDate      = VDT;
    using t_versionedObject  = VersionedObject<VDT, M, T...>;
    using t_dataset          = DataSet<M, T...>;
    using t_record           = typename t_dataset::t_record;
    using t_metaData         = typename t_dataset::t_metaData;
    using t_deltaEntriesMap  =
            typename _VersionedObjectBuilderBase<VDT, M, T...>::t_deltaEntriesMap;
    using t_deltaEntriesMap_iter_diff_type =
            typename _VersionedObjectBuilderBase<VDT, M, T...>::t_deltaEntriesMap_iter_diff_type;

    VersionedObjectBuilder() : _VersionedObjectBuilderBase<VDT, M, T...>() {}
    virtual ~VersionedObjectBuilder() {}

    inline void buildForwardTimeline( // with filled VersionObject
            VersionedObject<VDT, M, T...>&  vo)
    {
      VERSIONEDOBJECT_DEBUG_LOG("DEBUG_LOG:##VersionedObjectBuilder<VDT, M, T...>::buildForwardTimeline(START)");
      t_deltaEntriesMap comboChgEntries{this->_deltaChgEntries};
      this->_updateComboDataSet(comboChgEntries);
      this->_buildForwardTimeline(vo, comboChgEntries);
      comboChgEntries.clear();
      VERSIONEDOBJECT_DEBUG_LOG("DEBUG_LOG:##VersionedObjectBuilder<VDT, M, T...>::buildForwardTimeline(END)");
    }

    inline void buildReverseTimeline( // with filled VersionObject
            const t_versionDate& startDate,
            VersionedObject<VDT, M, T...>& vo)
    {
      VERSIONEDOBJECT_DEBUG_LOG("DEBUG_LOG:##VersionedObjectBuilder<VDT, M, T...>::buildReverseTimeline(START) : startDate=" << startDate);
      t_deltaEntriesMap comboChgEntries{this->_deltaChgEntries};
      this->_updateComboDataSet(comboChgEntries);
      this->_buildReverseTimeline(startDate, vo, comboChgEntries);
      comboChgEntries.clear();
      VERSIONEDOBJECT_DEBUG_LOG("DEBUG_LOG:##VersionedObjectBuilder<VDT, M, T...>::buildReverseTimeline(END)");
    }

    //          pair< Number of entries processed, Number of entries pending>
    inline std::pair< t_deltaEntriesMap_iter_diff_type, t_deltaEntriesMap_iter_diff_type >
    buildBiDirectionalTimeline( // with filled VersionObject
            const std::vector<t_versionDate>& startDates,
            VersionedObject<VDT, M, T...>& vo)
    {
      VERSIONEDOBJECT_DEBUG_LOG("DEBUG_LOG:##VersionedObjectBuilder<VDT, M, T...>::buildBiDirectionalTimeline(START)");
      t_deltaEntriesMap comboChgEntries{this->_deltaChgEntries};
      this->_updateComboDataSet(comboChgEntries);
      std::pair< t_deltaEntriesMap_iter_diff_type, t_deltaEntriesMap_iter_diff_type >
            ret = this->_buildBiDirectionalTimeline(startDates, vo, comboChgEntries);
      comboChgEntries.clear();
      VERSIONEDOBJECT_DEBUG_LOG("DEBUG_LOG:##VersionedObjectBuilder<VDT, M, T...>::buildBiDirectionalTimeline(END)");
      return ret;
    }
  };


  template <typename VDT, c_noMetaData T1, typename ... TR>
  class VersionedObjectBuilder<VDT, T1, TR...> : public _VersionedObjectBuilderBase<VDT, T1, TR...>
  {
  public:
    using t_versionDate      = VDT;
    using t_versionedObject  = VersionedObject<T1, TR...>;
    using t_dataset          = DataSet<T1, TR...>;
    using t_record           = typename t_dataset::t_record;
    using t_metaData         = typename t_dataset::t_metaData;
    using t_deltaEntriesMap  =
            typename _VersionedObjectBuilderBase<VDT, T1, TR...>::t_deltaEntriesMap;
    using t_deltaEntriesMap_iter_diff_type =
            typename _VersionedObjectBuilderBase<VDT, T1, TR...>::t_deltaEntriesMap_iter_diff_type;

    VersionedObjectBuilder() : _VersionedObjectBuilderBase<VDT, T1, TR...>() {}
    virtual ~VersionedObjectBuilder() {}

    inline void buildForwardTimeline( // with filled VersionObject
            VersionedObject<VDT, T1, TR...>& vo) // when MetaData is NOT used
    {
      VERSIONEDOBJECT_DEBUG_LOG("DEBUG_LOG:##VersionedObjectBuilder<VDT, T1, TR...>::buildForwardTimeline(START)");
      t_deltaEntriesMap comboChgEntries{this->_deltaChgEntries};
      this->_updateComboDataSet(comboChgEntries);
      this->_buildForwardTimeline(vo, comboChgEntries);
      comboChgEntries.clear();
      VERSIONEDOBJECT_DEBUG_LOG("DEBUG_LOG:##VersionedObjectBuilder<VDT, T1, TR...>::buildForwardTimeline(END)");
    }

    inline void buildReverseTimeline(
            const t_versionDate& startDate,
            VersionedObject<VDT, T1, TR...>& vo) // when MetaData is NOT used
    {
      VERSIONEDOBJECT_DEBUG_LOG("DEBUG_LOG:##VersionedObjectBuilder<VDT, T1, TR...>::buildReverseTimeline(START) : startDate=" << startDate);
      t_deltaEntriesMap comboChgEntries{this->_deltaChgEntries};
      this->_updateComboDataSet(comboChgEntries);
      this->_buildReverseTimeline(startDate, vo, comboChgEntries);
      comboChgEntries.clear();
      VERSIONEDOBJECT_DEBUG_LOG("DEBUG_LOG:##VersionedObjectBuilder<VDT, T1, TR...>::buildReverseTimeline(END)");
    }

    //          pair< Number of entries processed, Number of entries pending>
    inline std::pair< t_deltaEntriesMap_iter_diff_type, t_deltaEntriesMap_iter_diff_type >
    buildBiDirectionalTimeline( // with filled VersionObject
            const std::vector<t_versionDate>& startDates,
            VersionedObject<VDT, T1, TR...>& vo) // when MetaData is NOT used
    {
      VERSIONEDOBJECT_DEBUG_LOG("DEBUG_LOG:##VersionedObjectBuilder<VDT, T1, TR...>::buildBiDirectionalTimeline(START)");
      t_deltaEntriesMap comboChgEntries{this->_deltaChgEntries};
      this->_updateComboDataSet(comboChgEntries);
      std::pair< t_deltaEntriesMap_iter_diff_type, t_deltaEntriesMap_iter_diff_type >
                ret = this->_buildBiDirectionalTimeline(startDates, vo, comboChgEntries);
      comboChgEntries.clear();
      VERSIONEDOBJECT_DEBUG_LOG("DEBUG_LOG:##VersionedObjectBuilder<VDT, T1, TR...>::buildBiDirectionalTimeline(END)");
      return ret;
    }
  };
} }  //  datastructure::versionedObject
