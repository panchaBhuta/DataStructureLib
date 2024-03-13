/*
 * VersionedObjectPriorityMerge.h
 *
 * URL:      https://github.com/panchaBhuta/dataStructure
 * Version:  v2.1.3
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


  using VOPM_Record_Mismatch_exception = VO_exception<2>;

  template <typename VDT, typename ... MT>
  class VersionedObjectPriorityMerge
  {
  public:
    using t_versionDate      = VDT;
    using t_versionedObject  = VersionedObject<VDT, MT...>;
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
        const t_versionDate& highPriorityDate = iterHighPriorityVO->first;
        const t_versionDate& lowrPriorityDate = iterLowrPriorityVO->first;
        const t_dataset& highPriorityDataset = iterHighPriorityVO->second;
        const t_dataset& lowrPriorityDataset = iterLowrPriorityVO->second;
        if(highPriorityDate == lowrPriorityDate) {
          if(highPriorityDataset.getRecord() == lowrPriorityDataset.getRecord()) {
            // metaData need NOT be equal (when data from different sources)
            if constexpr(t_dataset::hasMetaData()) {
              t_metaData hpMetaData {highPriorityDataset.getMetaData()};
              hpMetaData.merge(lowrPriorityDataset.getMetaData());
              t_dataset highPriorityDataset_metaMerge{hpMetaData, highPriorityDataset.getRecord()};
              mergeVersionedObject.insertVersion(highPriorityDate, highPriorityDataset_metaMerge);
            } else {
              mergeVersionedObject.insertVersion(highPriorityDate, highPriorityDataset);
            }
            ++iterHighPriorityVO;
            ++iterLowrPriorityVO;
          } else {
            static std::string errMsg("ERROR : failure in VersionedObjectPriorityMerge<VDT, MT...>::getMergeResult() : different 'record' exits between 2 merge-candidates of VersionedObject");
#if FLAG_VERSIONEDOBJECT_debug_log == 1
            std::ostringstream eoss;
            eoss << errMsg << " : forDate=" << highPriorityDate;
            eoss <<   " : highPriorityDataset={ " << highPriorityDataset.toCSV();
            eoss << " } : lowrPriorityDataset={ " << lowrPriorityDataset.toCSV() << " }";
            VERSIONEDOBJECT_DEBUG_LOG(eoss.str());
#endif
            throw VOPM_Record_Mismatch_exception(errMsg);
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
        const t_versionDate& highPriorityDate = iterHighPriorityVO->first;
        const t_dataset& highPriorityDataset = iterHighPriorityVO->second;
        mergeVersionedObject.insertVersion(highPriorityDate, highPriorityDataset);
        ++iterHighPriorityVO;
      }

      while(iterLowrPriorityVO != _lowrPriorityVersionedObject.getDatasetLedger().cend())
      {
        const t_versionDate& lowrPriorityDate = iterLowrPriorityVO->first;
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
