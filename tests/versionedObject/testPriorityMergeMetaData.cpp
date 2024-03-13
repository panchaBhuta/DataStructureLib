
// test default conversions for inbuilt-types


#include <iostream>

#include <converter/specializedTypes/date.h>
#include <converter/specializedTypes/case_insensitive_string.h>

#include <versionedObject/VersionedObject.h>
#include <versionedObject/VersionedObjectPriorityMerge.h>

#include "../unittest.h"

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
using t_companyName = std::string;
using t_series      = std::string;
using t_listingDate = t_fmtdbY;  // std::chrono::year_month_day;
using t_paidUpValue = uint16_t;
using t_marketLot   = uint16_t;
using t_isinNumber  = std::string;
using t_faceValue   = uint16_t;

#define COMPANYINFO_TYPE_LIST t_symbol, t_companyName, t_series,     \
                              t_paidUpValue, t_marketLot, t_isinNumber, t_faceValue
using t_companyInfo = std::tuple<COMPANYINFO_TYPE_LIST>;

namespace dsvo = datastructure::versionedObject;

#define COMPANYMETAINFO_TYPE_LIST  dsvo::MetaDataSource, COMPANYINFO_TYPE_LIST



namespace unittest
{
  template<>
  struct SScompatible<dsvo::DataSet<COMPANYMETAINFO_TYPE_LIST>> {
    inline static std::string getVal(const dsvo::DataSet<COMPANYMETAINFO_TYPE_LIST>& val)
    {
      return val.toCSV();
    }
  };

  template<>
  struct SScompatible<dsvo::VersionedObject<t_fmtdbY, COMPANYMETAINFO_TYPE_LIST>> {
    inline static std::string getVal(const dsvo::VersionedObject<t_fmtdbY, COMPANYMETAINFO_TYPE_LIST>& val)
    {
      return val.toCSV();
    }
  };

  template<>
  struct SScompatible<dsvo::MetaDataSource> {
    inline static std::string getVal(const dsvo::MetaDataSource& val)
    {
      return val.toCSV();
    }
  };
}

int main()
{
  int rv = 0;
  try {
    bool insertResult;

/* these rows are from symbolchange.csv
ANDHRA PAPER LIMITED,APPAPER,IPAPPM,21-JAN-2014
ANDHRA PAPER LIMITED,IPAPPM,ANDPAPER,22-JAN-2020
ANDHRA PAPER LIMITED,ANDPAPER,ANDHRAPAP,05-MAR-2020
*/

//  NOTE : the row below is manually deduced
//  APPAPER,International Paper APPM Limited,EQ,13-MAY-2004,10,1,INE435A01028,10
    t_companyInfo companyInfoStart = converter::ConvertFromString<COMPANYINFO_TYPE_LIST>::ToVal(
      "APPAPER,International Paper APPM Limited,EQ,10,1,INE435A01028,10"    );

    dsvo::MetaDataSource crownMeta("crown",'^');
    dsvo::DataSet<COMPANYMETAINFO_TYPE_LIST> companyRecordStartCrown {crownMeta, companyInfoStart};

    const t_listingDate crownDate{std::chrono::year(int(2004)), std::chrono::May, std::chrono::day(unsigned(13))};
    dsvo::VersionedObject<t_fmtdbY, COMPANYMETAINFO_TYPE_LIST> voHighPriority;
    insertResult = voHighPriority.insertVersion(crownDate,
                                                companyRecordStartCrown);
    unittest::ExpectEqual(bool, true, insertResult);

    dsvo::MetaDataSource manualMeta("manualDeduction",'%');
    dsvo::DataSet<COMPANYMETAINFO_TYPE_LIST> companyRecordStartManual {manualMeta, companyInfoStart};

    dsvo::VersionedObject<t_fmtdbY, COMPANYMETAINFO_TYPE_LIST> voLowrPriority;
    insertResult = voLowrPriority.insertVersion(crownDate,
                                                companyRecordStartManual);
    unittest::ExpectEqual(bool, true, insertResult);


    dsvo::MetaDataSource expectedStartMergeMeta("crown%manualDeduction",'^');
    dsvo::DataSet<COMPANYMETAINFO_TYPE_LIST> companyStartRecordExpected { expectedStartMergeMeta, companyInfoStart};
    dsvo::VersionedObject<t_fmtdbY, COMPANYMETAINFO_TYPE_LIST> voExpected;
    insertResult = voExpected.insertVersion(crownDate,
                                            companyStartRecordExpected);
    unittest::ExpectEqual(bool, true, insertResult);
    {
      dsvo::VersionedObjectPriorityMerge<t_fmtdbY, COMPANYMETAINFO_TYPE_LIST> vopm{voHighPriority,voLowrPriority};
      dsvo::VersionedObject<t_fmtdbY, COMPANYMETAINFO_TYPE_LIST> voMerged;
      vopm.getMergeResult(voMerged);
      unittest::ExpectEqual(dsvo::VersionedObject<t_fmtdbY COMMA COMPANYMETAINFO_TYPE_LIST>, voExpected, voMerged);

      std::optional<dsvo::DataSet<COMPANYMETAINFO_TYPE_LIST>> companyRecordMerged = voMerged.getVersionAt(crownDate);
      unittest::ExpectEqual(bool, true, companyRecordMerged.has_value()); // has dsvo::DataSet<COMPANYMETAINFO_TYPE_LIST>

      unittest::ExpectEqual(dsvo::DataSet<COMPANYMETAINFO_TYPE_LIST>, companyStartRecordExpected,
                                                                      companyRecordMerged.value());

      unittest::ExpectEqual(dsvo::MetaDataSource, expectedStartMergeMeta, companyRecordMerged.value().getMetaData());
    }

//  ANDHRA PAPER LIMITED,APPAPER,IPAPPM,21-JAN-2014
    t_companyInfo companyInfoSecond = converter::ConvertFromString<COMPANYINFO_TYPE_LIST>::ToVal(
      "IPAPPM,International Paper APPM Limited,EQ,10,1,INE435A01028,10"    );

    dsvo::MetaDataSource symChgMetaExp("symbolchange",'+');
    dsvo::DataSet<COMPANYMETAINFO_TYPE_LIST> companyRecordSecondExpected {symChgMetaExp, companyInfoSecond};

    const t_listingDate symChgDate{std::chrono::year(int(2014)), std::chrono::January, std::chrono::day(unsigned(21))};
    insertResult = voHighPriority.insertVersion(symChgDate,
                                                companyRecordSecondExpected);

    unittest::ExpectEqual(bool, true, insertResult);

    insertResult = voExpected.insertVersion(symChgDate,
                                            companyRecordSecondExpected);
    unittest::ExpectEqual(bool, true, insertResult);
    {
      dsvo::VersionedObjectPriorityMerge<t_fmtdbY, COMPANYMETAINFO_TYPE_LIST> vopm{voHighPriority,voLowrPriority};
      dsvo::VersionedObject<t_fmtdbY, COMPANYMETAINFO_TYPE_LIST> voMerged;
      vopm.getMergeResult(voMerged);
      unittest::ExpectEqual(dsvo::VersionedObject<t_fmtdbY COMMA COMPANYMETAINFO_TYPE_LIST>, voExpected, voMerged);

      std::optional<dsvo::DataSet<COMPANYMETAINFO_TYPE_LIST>> companyRecordMerged = voMerged.getVersionAt(symChgDate);
      unittest::ExpectEqual(bool, true, companyRecordMerged.has_value()); // has dsvo::DataSet<COMPANYMETAINFO_TYPE_LIST>

      unittest::ExpectEqual(dsvo::DataSet<COMPANYMETAINFO_TYPE_LIST>, companyRecordSecondExpected,
                                                                      companyRecordMerged.value());

      unittest::ExpectEqual(dsvo::MetaDataSource, symChgMetaExp, companyRecordMerged.value().getMetaData());
    }

//  ANDHRA PAPER LIMITED,IPAPPM,ANDPAPER,22-JAN-2020
//  ANDHRAPAP,International Paper APPM Limited,ANDHRA PAPER LIMITED,22-JAN-2020
    t_companyInfo companyInfoThird = converter::ConvertFromString<COMPANYINFO_TYPE_LIST>::ToVal(
      "ANDPAPER,ANDHRA PAPER LIMITED,EQ,10,1,INE435A01028,10"    );

    dsvo::MetaDataSource symChgNamChgMetaExp("symbolchange+namechange",'+');
    dsvo::DataSet<COMPANYMETAINFO_TYPE_LIST> companyRecordThirdExpected {symChgNamChgMetaExp, companyInfoThird};

    const t_listingDate symChgNameChgDate{std::chrono::year(int(2020)), std::chrono::January, std::chrono::day(unsigned(22))};
    insertResult = voLowrPriority.insertVersion(symChgNameChgDate,
                                                companyRecordThirdExpected);
    unittest::ExpectEqual(bool, true, insertResult);

    insertResult = voExpected.insertVersion(symChgNameChgDate,
                                            companyRecordThirdExpected);
    unittest::ExpectEqual(bool, true, insertResult);
    {
      dsvo::VersionedObjectPriorityMerge<t_fmtdbY, COMPANYMETAINFO_TYPE_LIST> vopm{voHighPriority,voLowrPriority};
      dsvo::VersionedObject<t_fmtdbY, COMPANYMETAINFO_TYPE_LIST> voMerged;
      vopm.getMergeResult(voMerged);
      unittest::ExpectEqual(dsvo::VersionedObject<t_fmtdbY COMMA COMPANYMETAINFO_TYPE_LIST>, voExpected, voMerged);

      std::optional<dsvo::DataSet<COMPANYMETAINFO_TYPE_LIST>> companyRecordMerged = voMerged.getVersionAt(symChgNameChgDate);
      unittest::ExpectEqual(bool, true, companyRecordMerged.has_value()); // has dsvo::DataSet<COMPANYMETAINFO_TYPE_LIST>

      unittest::ExpectEqual(dsvo::DataSet<COMPANYMETAINFO_TYPE_LIST>, companyRecordThirdExpected,
                                                                      companyRecordMerged.value());

      unittest::ExpectEqual(dsvo::MetaDataSource, symChgNamChgMetaExp, companyRecordMerged.value().getMetaData());
    }


//  ANDHRA PAPER LIMITED,ANDPAPER,ANDHRAPAP,05-MAR-2020
//  NOTE: the row below is not a versioned information, but CROWN info from EQUITY_L.csv
//       ANDHRAPAP,ANDHRA PAPER LIMITED,EQ,13-MAY-2004,10,1,INE435A01028,10
    t_companyInfo companyInfoLatest1 = converter::ConvertFromString<COMPANYINFO_TYPE_LIST>::ToVal(
      "ANDHRAPAP,ANDHRA PAPER LIMITED:1,EQ,10,1,INE435A01028,10"    );

    const t_listingDate crownDate2{std::chrono::year(int(2020)), std::chrono::March, std::chrono::day(unsigned(5))};
    dsvo::DataSet<COMPANYMETAINFO_TYPE_LIST> companyRecordLatestExpected1 {symChgMetaExp, companyInfoLatest1};

    insertResult = voLowrPriority.insertVersion(crownDate2,
                                                companyRecordLatestExpected1);
    unittest::ExpectEqual(bool, true, insertResult);

    t_companyInfo companyInfoLatest2 = converter::ConvertFromString<COMPANYINFO_TYPE_LIST>::ToVal(
      "ANDHRAPAP,ANDHRA PAPER LIMITED:2,EQ,10,1,INE435A01028,10"    );

    dsvo::DataSet<COMPANYMETAINFO_TYPE_LIST> companyRecordLatestExpected2 {symChgMetaExp, companyInfoLatest2};

    insertResult = voHighPriority.insertVersion(crownDate2,
                                                companyRecordLatestExpected2);
    unittest::ExpectEqual(bool, true, insertResult);
    {
      dsvo::VersionedObjectPriorityMerge<t_fmtdbY, COMPANYMETAINFO_TYPE_LIST> vopm{voHighPriority,voLowrPriority};
      dsvo::VersionedObject<t_fmtdbY, COMPANYMETAINFO_TYPE_LIST> voMerged;
      ExpectExceptionMsg( vopm.getMergeResult(voMerged), dsvo::VOPM_Record_Mismatch_exception, "ERROR : failure in VersionedObjectPriorityMerge<VDT, MT...>::getMergeResult() : different 'record' exits between 2 merge-candidates of VersionedObject");
    }

  } catch (const std::exception& ex) {
    std::cout << ex.what() << std::endl;
    rv = 1;
  }

  return rv;
}
