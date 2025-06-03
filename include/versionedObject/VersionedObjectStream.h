/*
 * versionedObject.h
 *
 * URL:      https://github.com/panchaBhuta/dataStructure
 * Version:  v2.3.13
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

#include <optional>
#include <iterator>
#include <stdexcept>
#include <type_traits>

#include <converter/converter.h>

#include <dataStructure.h>

#include <versionedObject/VersionedObject.h>


namespace datastructure { namespace versionedObject
{

  template <typename VDT, typename ... MT>
  class VersionedObjectStream
  {
  public:
    using t_versionDate    = VDT;
    using t_dataset        = DataSet<MT ...>;
    using t_datasetLedger  = std::map< t_versionDate, t_dataset >;
    using t_record         = typename t_dataset::t_record;

    VersionedObjectStream(const VersionedObject<VDT, MT...>& vo) :
          _datasetLedger(vo.getDatasetLedger()) {}

    static VersionedObjectStream createVOstreamer(const VersionedObject<VDT, MT...>& vo)
    {
      return VersionedObjectStream(vo);
    }

    template<typename SH = StreamerHelper>
    inline void toCSV(const std::string& prefix, std::ostream& oss, const SH& streamerHelper = SH{}) const
    {
      for(auto iter : _datasetLedger)
      {
        const t_versionDate& versionDate = iter.first;
        const t_dataset& dataset = iter.second;
        oss << prefix << versionDate << streamerHelper.getDelimiterCSV();
        dataset.toCSV(oss, streamerHelper);
        oss << std::endl;
      }
    }

    template<typename SH = StreamerHelper>
    inline void toCSV(std::ostream& oss,
      typename std::enable_if_t<  !std::is_same_v<SH, std::ostream&>,
                                  const SH& >
      streamerHelper = SH{}) const
    {
      for(auto iter : _datasetLedger)
      {
        const t_versionDate& versionDate = iter.first;
        oss << versionDate << streamerHelper.getDelimiterCSV();
        const t_dataset& dataset = iter.second;
        dataset.toCSV(oss, streamerHelper);
        oss << std::endl;
      }
    }

    template<typename SH = StreamerHelper>
    inline std::string toCSV(const SH& streamerHelper = SH{}) const
    {
      std::ostringstream oss;
      toCSV(oss, streamerHelper);
      return oss.str();
    }

    template<typename SH = StreamerHelper>
    inline void toStr(const std::string& prefix, std::ostream& oss, const SH& streamerHelper = SH{}) const
    {
      for(auto iter : _datasetLedger)
      {
        const t_versionDate& versionDate = iter.first;
        const t_dataset& dataset = iter.second;
        oss << prefix << "versionDate=" << versionDate << ", dataSet={";
        dataset.toCSV(oss, streamerHelper);
        oss << "}" << std::endl;
      }
    }

    template<typename SH = StreamerHelper>
    inline void toStr(std::ostream& oss,
      typename std::enable_if_t<  !std::is_same_v<SH, std::ostream&>,
                                  const SH& >
      streamerHelper = SH{}) const
    {
      for(auto iter : _datasetLedger)
      {
        const t_versionDate& versionDate = iter.first;
        oss << "versionDate=" << versionDate << ", dataSet={";
        const t_dataset& dataset = iter.second;
        dataset.toCSV(oss, streamerHelper);
        oss << "}" << std::endl;
      }
    }

    template<typename SH = StreamerHelper>
    inline std::string toStr(const SH& streamerHelper = SH{}) const
    {
      std::ostringstream oss;
      toCSV(oss, streamerHelper);
      return oss.str();
    }

  protected:
    const t_datasetLedger&  _datasetLedger;

  };

} }   //  namespace datastructure::versionedObject
