/*
 * VersionedObjectPriorityMerge.h
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

#include <array>
#include <chrono>

#include <versionedObject/VersionedObject.h>


namespace datastructure { namespace versionedObject
{
  template <typename ... MT>
  class VersionedObjectPriorityMerge
  {
  public:
    using t_versionedObject  = VersionedObject<MT...>;
    using t_dataset          = DataSet<MT...>;
    using t_record           = typename t_dataset::t_record;
    using t_metaData         = typename t_dataset::t_metaData;
    using t_datasetLedger    = typename t_versionedObject::t_datasetLedger;

    VersionedObjectPriorityMerge(
      const t_versionedObject& highPriorityVersionedObject,
      const t_versionedObject& lowrPriorityVersionedObject
    ) : _highPriorityVersionedObject(highPriorityVersionedObject),
        _lowrPriorityVersionedObject(lowrPriorityVersionedObject)
    {}

    void getMergeResult(t_versionedObject& mergeVersionedObject) const
    {
      typename t_datasetLedger::const_iterator iterHighPriorityVO = _highPriorityVersionedObject.getDatasetLedger().cbegin();
      typename t_datasetLedger::const_iterator iterLowrPriorityVO = _lowrPriorityVersionedObject.getDatasetLedger().cbegin();

      while(iterHighPriorityVO != _highPriorityVersionedObject.getDatasetLedger().cend() &&
            iterLowrPriorityVO != _lowrPriorityVersionedObject.getDatasetLedger().cend())
      {
        const std::chrono::year_month_day& highPriorityDate = iterHighPriorityVO->first;
        const std::chrono::year_month_day& lowrPriorityDate = iterLowrPriorityVO->first;
        const t_dataset& highPriorityDataset = iterHighPriorityVO->second;
        const t_dataset& lowrPriorityDataset = iterLowrPriorityVO->second;
        if(highPriorityDate == lowrPriorityDate) {
          if(highPriorityDataset.getRecord() == lowrPriorityDataset.getRecord()) {
            // metaData need NOT be equal (when data from different sources)
            mergeVersionedObject.insertVersion(highPriorityDate, highPriorityDataset);
            ++iterHighPriorityVO;
            ++iterLowrPriorityVO;
          } else {
            static std::string errMsg("ERROR : failure in VersionedObjectPriorityMerge<MT...>::getMergeResult() : different 'record' exits between 2 merge-candidates of VersionedObject");
#if FLAG_VERSIONEDOBJECT_debug_log == 1
            std::ostringstream eoss;
            eoss << errMsg << " : forDate=" << converter::toStr_dbY(highPriorityDate);
            eoss << " : highPriorityDataset={ " << highPriorityDataset.toCSV();
            eoss << " } : lowrPriorityDataset={ " << lowrPriorityDataset.toCSV() << " }";
            VERSIONEDOBJECT_DEBUG_LOG(eoss.str());
#endif
            throw std::invalid_argument(errMsg);
          }
        } else if(highPriorityDate < lowrPriorityDate) {
          mergeVersionedObject.insertVersion(highPriorityDate, highPriorityDataset);
          ++iterHighPriorityVO;
        } else { // (highPriorityDate > lowrPriorityDate)
          mergeVersionedObject.insertVersion(lowrPriorityDate, lowrPriorityDataset);
          ++iterLowrPriorityVO;
        }
      }

      while(iterHighPriorityVO != _highPriorityVersionedObject.getDatasetLedger().cend())
      {
        const std::chrono::year_month_day& highPriorityDate = iterHighPriorityVO->first;
        const t_dataset& highPriorityDataset = iterHighPriorityVO->second;
        mergeVersionedObject.insertVersion(highPriorityDate, highPriorityDataset);
        ++iterHighPriorityVO;
      }

      while(iterLowrPriorityVO != _lowrPriorityVersionedObject.getDatasetLedger().cend())
      {
        const std::chrono::year_month_day& lowrPriorityDate = iterLowrPriorityVO->first;
        const t_dataset& lowrPriorityDataset = iterLowrPriorityVO->second;
        mergeVersionedObject.insertVersion(lowrPriorityDate, lowrPriorityDataset);
        ++iterLowrPriorityVO;
      }
    }

  private:
    const t_versionedObject& _highPriorityVersionedObject;
    const t_versionedObject& _lowrPriorityVersionedObject;
  };
} }  //  datastructure::versionedObject
