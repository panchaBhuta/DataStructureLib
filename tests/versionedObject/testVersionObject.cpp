
// test default conversions for inbuilt-types


#include <iostream>

#include <converter/specializedTypes/date.h>
#include <converter/specializedTypes/case_insensitive_string.h>

#include <versionedObject/VersionedObject.h>
#include <versionedObject/VersionedObjectBuilder.h>

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
using t_listingDate = t_fmtdbY; // std::chrono::year_month_day;
using t_paidUpValue = uint16_t;
using t_marketLot   = uint16_t;
using t_isinNumber  = std::string;
using t_faceValue   = uint16_t;

#define COMPANYINFO_TYPE_LIST t_symbol, t_companyName, t_series,     \
                              t_paidUpValue, t_marketLot, t_isinNumber, t_faceValue
using t_companyInfo = std::tuple<COMPANYINFO_TYPE_LIST>;

namespace dsvo = datastructure::versionedObject;

using t_versionDate = t_fmtdbY; // std::chrono::year_month_day;
using t_versionObject = dsvo::VersionedObject<t_versionDate, COMPANYINFO_TYPE_LIST>;


namespace unittest
{
  template<>
  struct SScompatible<dsvo::DataSet<COMPANYINFO_TYPE_LIST>> {
    inline static std::string getVal(const dsvo::DataSet<COMPANYINFO_TYPE_LIST>& val)
    {
      return val.toCSV();
    }
  };
}

void loadVO(dsvo::VersionedObject<t_versionDate, COMPANYINFO_TYPE_LIST>& vo,
            bool insertResultExpected)
{
  bool insertResult;


  std::string voStr =
    "13-May-2004,APPAPER,International Paper APPM Limited,EQ,10,1,INE435A01028,10\n"
    "21-Jan-2014,IPAPPM,International Paper APPM Limited,EQ,10,1,INE435A01028,10\n"
    "22-Jan-2020,ANDPAPER,ANDHRA PAPER LIMITED,EQ,10,1,INE435A01028,10\n"
    "05-Mar-2020,ANDHRAPAP,ANDHRA PAPER LIMITED,EQ,10,1,INE435A01028,10\n"
    "07-Apr-2021,ANDHRAPAP,ANDHRA PAPER LIMITED,EQ,10,2,INE435A01028,10\n";

//  APPAPER,International Paper APPM Limited,EQ,13-MAY-2004,10,1,INE435A01028,10
  t_companyInfo companyInfo1 = converter::ConvertFromString<COMPANYINFO_TYPE_LIST>::ToVal(
        "APPAPER,International Paper APPM Limited,EQ,10,1,INE435A01028,10"    );
  dsvo::DataSet<COMPANYINFO_TYPE_LIST> companyRecord1 {companyInfo1};
  t_versionDate date1{std::chrono::year(int(2004)), std::chrono::May, std::chrono::day(unsigned(13))};
  VERSIONEDOBJECT_DEBUG_MSG( "DEBUG_LOG: vo.insertVersion() -> versionDate: " << date1 << "; DATASET{" << companyRecord1.toCSV() << "}");
  insertResult = vo.insertVersion( date1, companyRecord1);
  VERSIONEDOBJECT_DEBUG_MSG( "DEBUG_LOG: vo.insertVersion() insertResult=" << insertResult);
  unittest::ExpectEqual(bool, insertResultExpected, insertResult);


  t_companyInfo companyInfo2 = converter::ConvertFromString<COMPANYINFO_TYPE_LIST>::ToVal(
        "IPAPPM,International Paper APPM Limited,EQ,10,1,INE435A01028,10"    );
  dsvo::DataSet<COMPANYINFO_TYPE_LIST> companyRecord2 {companyInfo2};
  t_versionDate date2{std::chrono::year(int(2014)), std::chrono::January, std::chrono::day(unsigned(21))};
  VERSIONEDOBJECT_DEBUG_MSG( "DEBUG_LOG: vo.insertVersion() -> versionDate: " << date2 << "; DATASET{" << companyRecord2.toCSV() << "}");
  insertResult = vo.insertVersion( date2, companyRecord2);
  VERSIONEDOBJECT_DEBUG_MSG( "DEBUG_LOG: vo.insertVersion() insertResult=" << insertResult);
  unittest::ExpectEqual(bool, insertResultExpected, insertResult);


  t_companyInfo companyInfo3 = converter::ConvertFromString<COMPANYINFO_TYPE_LIST>::ToVal(
        "ANDPAPER,ANDHRA PAPER LIMITED,EQ,10,1,INE435A01028,10"    );
  dsvo::DataSet<COMPANYINFO_TYPE_LIST> companyRecord3 {companyInfo3};
  t_versionDate date3{std::chrono::year(int(2020)), std::chrono::January, std::chrono::day(unsigned(22))};
  VERSIONEDOBJECT_DEBUG_MSG( "DEBUG_LOG: vo.insertVersion() -> versionDate: " << date3 << "; DATASET{" << companyRecord3.toCSV() << "}");
  insertResult = vo.insertVersion( date3, companyRecord3);
  VERSIONEDOBJECT_DEBUG_MSG( "DEBUG_LOG: vo.insertVersion() insertResult=" << insertResult);
  unittest::ExpectEqual(bool, insertResultExpected, insertResult);


  t_companyInfo companyInfo4 = converter::ConvertFromString<COMPANYINFO_TYPE_LIST>::ToVal(
        "ANDHRAPAP,ANDHRA PAPER LIMITED,EQ,10,1,INE435A01028,10"    );
  dsvo::DataSet<COMPANYINFO_TYPE_LIST> companyRecord4 {companyInfo4};
  t_versionDate date4{std::chrono::year(int(2020)), std::chrono::March, std::chrono::day(unsigned(05))};
  VERSIONEDOBJECT_DEBUG_MSG( "DEBUG_LOG: vo.insertVersion() -> versionDate: " << date4 << "; DATASET{" << companyRecord4.toCSV() << "}");
  insertResult = vo.insertVersion( date4, companyRecord4);
  VERSIONEDOBJECT_DEBUG_MSG( "DEBUG_LOG: vo.insertVersion() insertResult=" << insertResult);
  unittest::ExpectEqual(bool, insertResultExpected, insertResult);

  t_companyInfo companyInfo5 = converter::ConvertFromString<COMPANYINFO_TYPE_LIST>::ToVal(
        "ANDHRAPAP,ANDHRA PAPER LIMITED,EQ,10,2,INE435A01028,10"    );
  dsvo::DataSet<COMPANYINFO_TYPE_LIST> companyRecord5 {companyInfo5};
  t_versionDate date5{std::chrono::year(int(2021)), std::chrono::April, std::chrono::day(unsigned(07))};
  VERSIONEDOBJECT_DEBUG_MSG( "DEBUG_LOG: vo.insertVersion() -> versionDate: " << date5 << "; DATASET{" << companyRecord5.toCSV() << "}");
  insertResult = vo.insertVersion( date5, companyRecord5);
  VERSIONEDOBJECT_DEBUG_MSG( "DEBUG_LOG: vo.insertVersion() insertResult=" << insertResult);
  unittest::ExpectEqual(bool, insertResultExpected, insertResult);


/////////  getVersionAt()  ///////////

  t_versionObject::t_datasetLedger::const_iterator companyRecordSearch =
    vo.getVersionAt(t_versionDate{std::chrono::year(int(2004)), std::chrono::May, std::chrono::day(unsigned(12))});
  unittest::ExpectEqual(bool, true, vo.getDatasetLedger().cend() == companyRecordSearch);

  companyRecordSearch =
    vo.getVersionAt(t_versionDate{std::chrono::year(int(2004)), std::chrono::May, std::chrono::day(unsigned(13))});
  unittest::ExpectEqual(dsvo::DataSet<COMPANYINFO_TYPE_LIST>, companyRecord1,
                                                              companyRecordSearch->second);

  companyRecordSearch =
    vo.getVersionAt(t_versionDate{std::chrono::year(int(2004)), std::chrono::May, std::chrono::day(unsigned(14))});
  unittest::ExpectEqual(dsvo::DataSet<COMPANYINFO_TYPE_LIST>, companyRecord1,
                                                              companyRecordSearch->second);



  companyRecordSearch =
    vo.getVersionAt(t_versionDate{std::chrono::year(int(2020)), std::chrono::January, std::chrono::day(unsigned(21))});
  unittest::ExpectEqual(dsvo::DataSet<COMPANYINFO_TYPE_LIST>, companyRecord2,
                                                              companyRecordSearch->second);

  companyRecordSearch =
    vo.getVersionAt(t_versionDate{std::chrono::year(int(2020)), std::chrono::January, std::chrono::day(unsigned(22))});
  unittest::ExpectEqual(dsvo::DataSet<COMPANYINFO_TYPE_LIST>, companyRecord3,
                                                              companyRecordSearch->second);

  companyRecordSearch =
    vo.getVersionAt(t_versionDate{std::chrono::year(int(2020)), std::chrono::January, std::chrono::day(unsigned(23))});
  unittest::ExpectEqual(dsvo::DataSet<COMPANYINFO_TYPE_LIST>, companyRecord3,
                                                              companyRecordSearch->second);



  companyRecordSearch =
    vo.getVersionAt(t_versionDate{std::chrono::year(int(2021)), std::chrono::April, std::chrono::day(unsigned(6))});
  unittest::ExpectEqual(dsvo::DataSet<COMPANYINFO_TYPE_LIST>, companyRecord4,
                                                              companyRecordSearch->second);

  companyRecordSearch =
    vo.getVersionAt(t_versionDate{std::chrono::year(int(2021)), std::chrono::April, std::chrono::day(unsigned(7))});
  unittest::ExpectEqual(dsvo::DataSet<COMPANYINFO_TYPE_LIST>, companyRecord5,
                                                              companyRecordSearch->second);

  companyRecordSearch =
    vo.getVersionAt(t_versionDate{std::chrono::year(int(2021)), std::chrono::April, std::chrono::day(unsigned(8))});
  unittest::ExpectEqual(dsvo::DataSet<COMPANYINFO_TYPE_LIST>, companyRecord5,
                                                              companyRecordSearch->second);


/////////  getVersionBefore()  ///////////

  companyRecordSearch =
    vo.getVersionBefore(t_versionDate{std::chrono::year(int(2004)), std::chrono::May, std::chrono::day(unsigned(12))});
  unittest::ExpectEqual(bool, true, vo.getDatasetLedger().cend() == companyRecordSearch);

  companyRecordSearch =
    vo.getVersionBefore(t_versionDate{std::chrono::year(int(2004)), std::chrono::May, std::chrono::day(unsigned(13))});
  unittest::ExpectEqual(bool, true, vo.getDatasetLedger().cend() == companyRecordSearch);

  companyRecordSearch =
    vo.getVersionBefore(t_versionDate{std::chrono::year(int(2004)), std::chrono::May, std::chrono::day(unsigned(14))});
  unittest::ExpectEqual(dsvo::DataSet<COMPANYINFO_TYPE_LIST>, companyRecord1,
                                                              companyRecordSearch->second);



  companyRecordSearch =
    vo.getVersionBefore(t_versionDate{std::chrono::year(int(2020)), std::chrono::January, std::chrono::day(unsigned(21))});
  unittest::ExpectEqual(dsvo::DataSet<COMPANYINFO_TYPE_LIST>, companyRecord2,
                                                              companyRecordSearch->second);

  companyRecordSearch =
    vo.getVersionBefore(t_versionDate{std::chrono::year(int(2020)), std::chrono::January, std::chrono::day(unsigned(22))});
  unittest::ExpectEqual(dsvo::DataSet<COMPANYINFO_TYPE_LIST>, companyRecord2,
                                                              companyRecordSearch->second);

  companyRecordSearch =
    vo.getVersionBefore(t_versionDate{std::chrono::year(int(2020)), std::chrono::January, std::chrono::day(unsigned(23))});
  unittest::ExpectEqual(dsvo::DataSet<COMPANYINFO_TYPE_LIST>, companyRecord3,
                                                              companyRecordSearch->second);



  companyRecordSearch =
    vo.getVersionBefore(t_versionDate{std::chrono::year(int(2021)), std::chrono::April, std::chrono::day(unsigned(6))});
  unittest::ExpectEqual(dsvo::DataSet<COMPANYINFO_TYPE_LIST>, companyRecord4,
                                                              companyRecordSearch->second);

  companyRecordSearch =
    vo.getVersionBefore(t_versionDate{std::chrono::year(int(2021)), std::chrono::April, std::chrono::day(unsigned(7))});
  unittest::ExpectEqual(dsvo::DataSet<COMPANYINFO_TYPE_LIST>, companyRecord4,
                                                              companyRecordSearch->second);

  companyRecordSearch =
    vo.getVersionBefore(t_versionDate{std::chrono::year(int(2021)), std::chrono::April, std::chrono::day(unsigned(8))});
  unittest::ExpectEqual(dsvo::DataSet<COMPANYINFO_TYPE_LIST>, companyRecord5,
                                                              companyRecordSearch->second);
}


int main()
{
  int rv = 0;
  dsvo::VersionedObject<t_versionDate, COMPANYINFO_TYPE_LIST> vo;

  try {
    std::cout << "TEST_LOG : FIRST VO load , no initial data. fresh start." << std::endl;
    loadVO(vo, true);
    std::cout << "TEST_LOG : SECOND VO load , with existing data reloaded again. scenario after first load." << std::endl;
    loadVO(vo, false);

  } catch (const std::exception& ex) {
    std::cout << ex.what() << std::endl;
    rv = 1;
  }

  return rv;
}
