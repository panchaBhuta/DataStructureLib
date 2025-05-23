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
  class _VersionedObjectStream
  {
  public:
    using t_versionDate    = VDT;
    using t_dataset        = DataSet<MT ...>;
    using t_datasetLedger  = std::map< t_versionDate, t_dataset >;
    using t_record         = typename t_dataset::t_record;

    _VersionedObjectStream(const t_datasetLedger& datasetLedger) : _datasetLedger(datasetLedger) {}

  protected:
    const t_datasetLedger&  _datasetLedger;

    inline void _toCSV(const std::string& prefix, std::ostream& oss, const char delimiterMetaData) const
    {
      for(auto iter : _datasetLedger)
      {
        const t_versionDate& versionDate = iter.first;
        const t_dataset& dataset = iter.second;
        oss << prefix << versionDate << ",";
        if constexpr (t_dataset::hasMetaData())
        {
          dataset.toCSV(oss, delimiterMetaData);
        } else {
          dataset.toCSV(oss);
        }
        oss << std::endl;
      }
    }

    inline void _toCSV(std::ostream& oss, const char delimiterMetaData) const
    {
      for(auto iter : _datasetLedger)
      {
        const t_versionDate& versionDate = iter.first;
        oss << versionDate << ",";
        const t_dataset& dataset = iter.second;
        if constexpr (t_dataset::hasMetaData())
        {
          dataset.toCSV(oss, delimiterMetaData);
        } else {
          dataset.toCSV(oss);
        }
        oss << std::endl;
      }
    }

    inline std::string _toCSV(const char delimiterMetaData) const
    {
      std::ostringstream oss;
      _toCSV(oss, delimiterMetaData);
      return oss.str();
    }

    inline void _toStr(const std::string& prefix, std::ostream& oss, const char delimiterMetaData) const
    {
      for(auto iter : _datasetLedger)
      {
        const t_versionDate& versionDate = iter.first;
        const t_dataset& dataset = iter.second;
        oss << prefix << "versionDate=" << versionDate << ", dataSet={";
        if constexpr (t_dataset::hasMetaData())
        {
          dataset.toCSV(oss, delimiterMetaData);
        } else {
          dataset.toCSV(oss);
        }
        oss << "}" << std::endl;
      }
    }

    inline void _toStr(std::ostream& oss, const char delimiterMetaData) const
    {
      for(auto iter : _datasetLedger)
      {
        const t_versionDate& versionDate = iter.first;
        oss << "versionDate=" << versionDate << ", dataSet={";
        const t_dataset& dataset = iter.second;
        if constexpr (t_dataset::hasMetaData())
        {
          dataset.toCSV(oss, delimiterMetaData);
        } else {
          dataset.toCSV(oss);
        }
        oss << "}" << std::endl;
      }
    }

    inline std::string _toStr(const char delimiterMetaData) const
    {
      std::ostringstream oss;
      if constexpr (t_dataset::hasMetaData())
      {
        _toCSV(oss, delimiterMetaData);
      } else {
        _toCSV(oss);
      }
      return oss.str();
    }
  };


  template <bool hasMetaData, typename VDT, typename M, typename ... T>
  class VersionedObjectStream;

  template <typename VDT, c_MetaData M, typename ... T>
  class VersionedObjectStream<true, VDT, M, T...> :
        protected _VersionedObjectStream<VDT, M, T...>
  {
  public:
    using t_versionDate    = VDT;
    using t_dataset        = DataSet<M, T ...>;
    using t_datasetLedger  = std::map< t_versionDate, t_dataset >;
    using t_record         = typename t_dataset::t_record;
    using t_versionedObjectStreamBase = _VersionedObjectStream<VDT, M, T...>;

    VersionedObjectStream(const VersionedObject<VDT, M, T...>& vo) :
          _VersionedObjectStream<VDT, M, T...>(vo.getDatasetLedger()) {}

    static VersionedObjectStream createVOstreamer(const VersionedObject<VDT, M, T...>& vo)
    {
      return VersionedObjectStream(vo);
    }

    inline void toCSV(const std::string& prefix, std::ostream& oss,
                      const char delimiterMetaData = M::delimiter) const
    {
      t_versionedObjectStreamBase::_toCSV(prefix, oss, delimiterMetaData);
    }

    inline void toCSV(std::ostream& oss, const char delimiterMetaData = M::delimiter) const
    {
      t_versionedObjectStreamBase::_toCSV(oss, delimiterMetaData);
    }

    inline std::string toCSV(const char delimiterMetaData = M::delimiter) const
    {
      return t_versionedObjectStreamBase::_toCSV(delimiterMetaData);
    }

    inline void toStr(const std::string& prefix, std::ostream& oss, const char delimiterMetaData = M::delimiter) const
    {
      return t_versionedObjectStreamBase::_toStr(prefix, oss, delimiterMetaData);
    }

    inline void toStr(std::ostream& oss, const char delimiterMetaData = M::delimiter) const
    {
      return t_versionedObjectStreamBase::_toStr(oss, delimiterMetaData);
    }

    inline std::string toStr(const char delimiterMetaData = M::delimiter) const
    {
      return t_versionedObjectStreamBase::_toStr(delimiterMetaData);
    }
  };

  template <typename VDT, c_noMetaData T1, typename ... TR>
  class VersionedObjectStream<false, VDT, T1, TR...> :
        protected _VersionedObjectStream<VDT, T1, TR...>
  {
  public:
    using t_versionDate    = VDT;
    using t_dataset        = DataSet<T1, TR...>;
    using t_datasetLedger  = std::map< t_versionDate, t_dataset >;
    using t_record         = typename t_dataset::t_record;
    using t_versionedObjectStreamBase = _VersionedObjectStream<VDT, T1, TR...>;

    VersionedObjectStream(const VersionedObject<VDT, T1, TR...>& vo) :
          _VersionedObjectStream<VDT, T1, TR...>(vo.getDatasetLedger()) {}

    static VersionedObjectStream createVOstreamer(const VersionedObject<VDT, T1, TR...>& vo)
    {
      return VersionedObjectStream(vo);
    }

    inline void toCSV(const std::string& prefix, std::ostream& oss) const
    {
      t_versionedObjectStreamBase::_toCSV(prefix, oss, '~');
    }

    inline void toCSV(std::ostream& oss) const
    {
      t_versionedObjectStreamBase::_toCSV(oss, '~');
    }

    inline std::string toCSV() const
    {
      return t_versionedObjectStreamBase::_toCSV('~');
    }

    inline void toStr(const std::string& prefix, std::ostream& oss) const
    {
      return t_versionedObjectStreamBase::_toStr(prefix, oss, '~');
    }

    inline void toStr(std::ostream& oss) const
    {
      return t_versionedObjectStreamBase::_toStr(oss, '~');
    }

    inline std::string toStr() const
    {
      return t_versionedObjectStreamBase::_toStr('~');
    }
  };

} }   //  namespace datastructure::versionedObject
