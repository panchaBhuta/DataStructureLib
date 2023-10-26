
// test default conversions for inbuilt-types


#include <iostream>

#include <versionedObject/versionedObject.h>

#include "unittest.h"

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

using t_symbol = std::string;
using t_companyName = std::string;
using t_series = std::string;
using t_listingDate = std::chrono::year_month_day;
using t_paidUpValue = uint16_t;
using t_marketLot   = uint16_t;
using t_isinNumber  = std::string;
using t_faceValue   = uint16_t;

#define COMPANYINFO_TYPE_LIST t_symbol, t_companyName, t_series,     \
                              t_paidUpValue, t_marketLot, t_isinNumber, t_faceValue
using t_companyInfo = std::tuple<COMPANYINFO_TYPE_LIST>;



namespace unittest
{
  template<>
  struct SScompatible<versionedObject::DataSet<COMPANYINFO_TYPE_LIST>> {
    inline static std::string getVal(const versionedObject::DataSet<COMPANYINFO_TYPE_LIST>& val)
    {
      std::ostringstream oss;
      val.toCSV(oss);
      return oss.str();
    }
  };
}

int main()
{
  int rv = 0;
  try {
//  NOTE : the row below is manually deduced
//  APPAPER,International Paper APPM Limited,EQ,13-MAY-2004,10,1,INE435A01028,10
    t_companyInfo companyInfoStart = converter::ConvertFromString<COMPANYINFO_TYPE_LIST>::ToVal(
      "APPAPER,International Paper APPM Limited,EQ,10,1,INE435A01028,10"    );

    versionedObject::DataSet<COMPANYINFO_TYPE_LIST> companyRecordStart {"manualDeduction", companyInfoStart};

    versionedObject::VersionedObject<COMPANYINFO_TYPE_LIST> vo;

    vo.insertVersion(t_listingDate(std::chrono::year(int(2004)), std::chrono::May, std::chrono::day(unsigned(13))), companyRecordStart);

/* these rows are from symbolchange.csv
ANDHRA PAPER LIMITED,APPAPER,IPAPPM,21-JAN-2014
ANDHRA PAPER LIMITED,IPAPPM,ANDPAPER,22-JAN-2020
ANDHRA PAPER LIMITED,ANDPAPER,ANDHRAPAP,05-MAR-2020
*/
    const std::array <bool, std::tuple_size_v<t_companyInfo> > symbolChangeIdx = {true, false, false, false, false, false, false};

    t_companyInfo symChgOldInfo1 = converter::ConvertFromString<COMPANYINFO_TYPE_LIST>::ToVal("APPAPER,,,0,0,,0");
    t_companyInfo symChgNewInfo1 = converter::ConvertFromString<COMPANYINFO_TYPE_LIST>::ToVal("IPAPPM,,,0,0,,0");
    versionedObject::ChangesInDataSet<COMPANYINFO_TYPE_LIST> symbolChange1 {"symbolchange", symbolChangeIdx, symChgOldInfo1, symChgNewInfo1};
    vo.insertVersion(t_listingDate(std::chrono::year(int(2014)), std::chrono::January, std::chrono::day(unsigned(21))), symbolChange1);

    t_companyInfo symChgOldInfo2 = converter::ConvertFromString<COMPANYINFO_TYPE_LIST>::ToVal("IPAPPM,,,0,0,,0");
    t_companyInfo symChgNewInfo2 = converter::ConvertFromString<COMPANYINFO_TYPE_LIST>::ToVal("ANDPAPER,,,0,0,,0");
    versionedObject::ChangesInDataSet<COMPANYINFO_TYPE_LIST> symbolChange2 {"symbolchange", symbolChangeIdx, symChgOldInfo2, symChgNewInfo2};
    vo.insertVersion(t_listingDate(std::chrono::year(int(2020)), std::chrono::January, std::chrono::day(unsigned(22))), symbolChange2);

    t_companyInfo symChgOldInfo3 = converter::ConvertFromString<COMPANYINFO_TYPE_LIST>::ToVal("ANDPAPER,,,0,0,,0");
    t_companyInfo symChgNewInfo3 = converter::ConvertFromString<COMPANYINFO_TYPE_LIST>::ToVal("ANDHRAPAP,,,0,0,,0");
    versionedObject::ChangesInDataSet<COMPANYINFO_TYPE_LIST> symbolChange3 {"symbolchange", symbolChangeIdx, symChgOldInfo3, symChgNewInfo3};
    vo.insertVersion(t_listingDate(std::chrono::year(int(2020)), std::chrono::March, std::chrono::day(unsigned(5))), symbolChange3);


    const std::array <bool, std::tuple_size_v<t_companyInfo> > nameChangeIdx = {false, true, false, false, false, false, false};

//  NOTE : the row below is from namechange.csv
//  ANDHRAPAP,International Paper APPM Limited,ANDHRA PAPER LIMITED,22-JAN-2020
    t_companyInfo namChgOldInfo1 = converter::ConvertFromString<COMPANYINFO_TYPE_LIST>::ToVal(",International Paper APPM Limited,,0,0,,0");
    t_companyInfo namChgNewInfo1 = converter::ConvertFromString<COMPANYINFO_TYPE_LIST>::ToVal(",ANDHRA PAPER LIMITED,,0,0,,0");
    versionedObject::ChangesInDataSet<COMPANYINFO_TYPE_LIST> nameChange1 {"namechange", nameChangeIdx, namChgOldInfo1, namChgNewInfo1};
    vo.insertVersion(t_listingDate(std::chrono::year(int(2020)), std::chrono::January, std::chrono::day(unsigned(22))), nameChange1);


    vo.validateChanges();

//  NOTE: the row below is not a versioned information, but info from EQUITY_L.csv
//  ANDHRAPAP,ANDHRA PAPER LIMITED,EQ,13-MAY-2004,10,1,INE435A01028,10
    t_companyInfo companyInfoLatest = converter::ConvertFromString<COMPANYINFO_TYPE_LIST>::ToVal(
      "ANDHRAPAP,ANDHRA PAPER LIMITED,EQ,10,1,INE435A01028,10"    );

    versionedObject::DataSet<COMPANYINFO_TYPE_LIST> companyRecordLatestExpected {"getVersionAt", companyInfoLatest};

    std::variant<versionedObject::DataSet<COMPANYINFO_TYPE_LIST>, bool> companyRecordLatestActualVar =
      vo.getVersionAt(t_listingDate(std::chrono::year(int(2020)), std::chrono::March, std::chrono::day(unsigned(5))));

    unittest::ExpectEqual(size_t, 0, companyRecordLatestActualVar.index());

    versionedObject::DataSet<COMPANYINFO_TYPE_LIST> companyRecordLatestActual =
      std::get<versionedObject::DataSet<COMPANYINFO_TYPE_LIST>>(companyRecordLatestActualVar);

    unittest::ExpectEqual(versionedObject::DataSet<COMPANYINFO_TYPE_LIST>, companyRecordLatestExpected,
                                                                           companyRecordLatestActual);
  } catch (const std::exception& ex) {
    std::cout << ex.what() << std::endl;
    rv = 1;
  }

  return rv;
}