#pragma once

// test default conversions for inbuilt-types


#include <iostream>

#include <converter/specializedTypes/date.h>
#include <converter/specializedTypes/case_insensitive_string.h>

#include <versionedObject/VersionedObject.h>
#include <versionedObject/VersionedObjectBuilder.h>

#include <unittest.h>

//#define COMMA ,  defined in 'unittest.h'

#if TEST_ENABLE_METADATA == 1
  #define TEST_WITH_METADATA(X)  X
#else
  #define TEST_WITH_METADATA(X)
#endif

using t_fmtdbY = converter::format_year_month_day<converter::dbY_fmt, converter::FailureS2Tprocess::THROW_ERROR>;


/*
SYMBOL, NAME OF COMPANY, SERIES, DATE OF LISTING, PAID UP VALUE, MARKET LOT, ISIN NUMBER, FACE VALUE
20MICRONS,20 Microns Limited,BE,06-OCT-2008,5,1,INE144J01027,5
21STCENMGM,21st Century Management Services Limited,EQ,03-MAY-1995,10,1,INE253B01015,10
360ONE,360 ONE WAM LIMITED,EQ,19-SEP-2019,1,1,INE466L01038,1
3IINFOLTD,3i Infotech Limited,EQ,22-OCT-2021,10,1,INE748C01038,10
3MINDIA,3M India Limited,EQ,13-AUG-2004,10,1,INE470A01017,10

ANDHRAPAP,ANDHRA PAPER LIMITED,EQ,13-MAY-2004,10,1,INE435A01028,10


NOTE: columns { SYMBOL, NAME OF COMPANY, ... }
*/

using t_symbol      = std::string;
using t_companyName = converter::ci_string; //std::string;  REFER test :test_caseInsensitive_companyName_BuildReverseTimelineNoMetaData
using t_series      = std::string;
using t_listingDate = t_fmtdbY; // std::chrono::year_month_day;
using t_paidUpValue = uint16_t;
using t_marketLot   = uint16_t;
using t_isinNumber  = std::string;
using t_faceValue   = uint16_t;
using t_status      = std::string;

#define COMPANYINFO_TYPE_LIST t_symbol, t_companyName, t_series,     \
                              t_paidUpValue, t_marketLot, t_isinNumber, t_faceValue,  t_status
using t_companyInfo = std::tuple<COMPANYINFO_TYPE_LIST>;

namespace dsvo = datastructure::versionedObject;

#define COMPANYMETAINFO_TYPE_LIST  TEST_WITH_METADATA(dsvo::MetaDataSource COMMA) COMPANYINFO_TYPE_LIST

using t_versionDate = t_fmtdbY; // std::chrono::year_month_day;
using t_versionObject = dsvo::VersionedObject<t_versionDate, COMPANYMETAINFO_TYPE_LIST>;
using t_versionObjectBuilder = dsvo::VersionedObjectBuilder<t_versionDate, COMPANYMETAINFO_TYPE_LIST>;
using t_changesInDataSet = dsvo::ChangesInDataSet<COMPANYMETAINFO_TYPE_LIST>;
using t_snapshotDataSet  = dsvo::SnapshotDataSet<COMPANYMETAINFO_TYPE_LIST>;
using t_dataSet  = dsvo::DataSet<COMPANYMETAINFO_TYPE_LIST>;
using t_convertFromString = converter::ConvertFromString<COMPANYINFO_TYPE_LIST>;

namespace unittest
{
  template<>
  struct SScompatible<t_dataSet> {
    inline static std::string getVal(const t_dataSet& val)
    {
      return val.toCSV();
    }
  };
}

void loadVO(t_versionObject& vo,
            bool insertResultExpected);

