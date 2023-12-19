
// test default conversions for inbuilt-types


#include <iostream>

#include <versionedObject/VersionedObject.h>
#include <versionedObject/VersionedObjectBuilder.h>

#include "../unittest.h"

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
using t_listingDate = std::chrono::year_month_day;
using t_paidUpValue = uint16_t;
using t_marketLot   = uint16_t;
using t_isinNumber  = std::string;
using t_faceValue   = uint16_t;

#define COMPANYINFO_TYPE_LIST t_symbol, t_companyName, t_series,     \
                              t_paidUpValue, t_marketLot, t_isinNumber, t_faceValue
using t_companyInfo = std::tuple<COMPANYINFO_TYPE_LIST>;

#define COMPANYMETAINFO_TYPE_LIST  versionedObject::MetaDataSource, COMPANYINFO_TYPE_LIST


namespace unittest
{
  template<>
  struct SScompatible<versionedObject::DataSet<COMPANYMETAINFO_TYPE_LIST>> {
    inline static std::string getVal(const versionedObject::DataSet<COMPANYMETAINFO_TYPE_LIST>& val)
    {
      return val.toCSV();
    }
  };
}

int main()
{
  int rv = 0;
  try {
    versionedObject::VersionedObjectBuilder<COMPANYMETAINFO_TYPE_LIST> vob;

    bool insertResult;

/* these rows are from symbolchange.csv
CompanyName, OLD-SYMBOL, NEW-SYMBOL, CHANGE-DATE
ANDHRA PAPER LIMITED,APPAPER,IPAPPM,21-JAN-2014
ANDHRA PAPER LIMITED,IPAPPM,ANDPAPER,22-JAN-2020
ANDHRA PAPER LIMITED,ANDPAPER,ANDHRAPAP,05-MAR-2020
*/
    versionedObject::MetaDataSource symChgMeta("symbolchange",'-');
    const std::array <bool, std::tuple_size_v<t_companyInfo> > symbolChangeFlg = {true, false, false, false, false, false, false};

    t_companyInfo symChgOldInfo1 = converter::ConvertFromString<COMPANYINFO_TYPE_LIST>::ToVal("APPAPER,,,0,0,,0");
    t_companyInfo symChgNewInfo1 = converter::ConvertFromString<COMPANYINFO_TYPE_LIST>::ToVal("IPAPPM,,,0,0,,0");
    versionedObject::ChangesInDataSet<COMPANYMETAINFO_TYPE_LIST> symbolChange1 {symChgMeta, symbolChangeFlg, symChgOldInfo1, symChgNewInfo1};
    insertResult = vob.insertDeltaVersion(t_listingDate(std::chrono::year(int(2014)), std::chrono::January, std::chrono::day(unsigned(21))), symbolChange1);
    unittest::ExpectEqual(bool, true, insertResult);

    t_companyInfo symChgOldInfo2 = converter::ConvertFromString<COMPANYINFO_TYPE_LIST>::ToVal("IPAPPM,,,0,0,,0");
    t_companyInfo symChgNewInfo2 = converter::ConvertFromString<COMPANYINFO_TYPE_LIST>::ToVal("ANDPAPER,,,0,0,,0");
    versionedObject::ChangesInDataSet<COMPANYMETAINFO_TYPE_LIST> symbolChange2 {symChgMeta, symbolChangeFlg, symChgOldInfo2, symChgNewInfo2};
    insertResult = vob.insertDeltaVersion(t_listingDate(std::chrono::year(int(2020)), std::chrono::January, std::chrono::day(unsigned(22))), symbolChange2);
    unittest::ExpectEqual(bool, true, insertResult);

    t_companyInfo symChgOldInfo3 = converter::ConvertFromString<COMPANYINFO_TYPE_LIST>::ToVal("ANDPAPER,,,0,0,,0");
    t_companyInfo symChgNewInfo3 = converter::ConvertFromString<COMPANYINFO_TYPE_LIST>::ToVal("ANDHRAPAP,,,0,0,,0");
    versionedObject::ChangesInDataSet<COMPANYMETAINFO_TYPE_LIST> symbolChange3 {symChgMeta, symbolChangeFlg, symChgOldInfo3, symChgNewInfo3};
    insertResult = vob.insertDeltaVersion(t_listingDate(std::chrono::year(int(2020)), std::chrono::March, std::chrono::day(unsigned(5))), symbolChange3);
    unittest::ExpectEqual(bool, true, insertResult);


    versionedObject::MetaDataSource namChgMeta("namechange",'-');
    const std::array <bool, std::tuple_size_v<t_companyInfo> > nameChangeFlg = {false, true, false, false, false, false, false};

//  NOTE : the row below is from namechange.csv
//  SYMBOL, OLD-CompanyName, NEW-CompanyName, CHANGE-DATE
//  ANDHRAPAP,International Paper APPM Limited,ANDHRA PAPER LIMITED,22-JAN-2020
    t_companyInfo namChgOldInfo1 = converter::ConvertFromString<COMPANYINFO_TYPE_LIST>::ToVal(",International Paper APPM Limited,,0,0,,0");
    t_companyInfo namChgNewInfo1 = converter::ConvertFromString<COMPANYINFO_TYPE_LIST>::ToVal(",ANDHRA PAPER LIMITED,,0,0,,0");
    versionedObject::ChangesInDataSet<COMPANYMETAINFO_TYPE_LIST> nameChange1 {namChgMeta, nameChangeFlg, namChgOldInfo1, namChgNewInfo1};
    insertResult = vob.insertDeltaVersion(t_listingDate(std::chrono::year(int(2020)), std::chrono::January, std::chrono::day(unsigned(22))), nameChange1);
    unittest::ExpectEqual(bool, true, insertResult);



//  NOTE : the row below is not a versioned information, but info from EQUITY_L.csv
//       ANDHRAPAP,ANDHRA PAPER LIMITED,EQ,13-MAY-2004,10,1,INE435A01028,10
    t_companyInfo companyInfoLatest = converter::ConvertFromString<COMPANYINFO_TYPE_LIST>::ToVal(
      "ANDHRAPAP,ANDHRA PAPER LIMITED,EQ,10,1,INE435A01028,10"    );

    versionedObject::MetaDataSource latestMeta("EQUITY_L",'-');
    versionedObject::DataSet<COMPANYMETAINFO_TYPE_LIST> companyRecordLatestExpected {latestMeta, companyInfoLatest};

    // resetMeta for cloning in _VersionedObjectBuilderBase<MT...>::_buildReverseTimeline()
    versionedObject::MetaDataSource resetMeta("",'-');
    versionedObject::VersionedObject<COMPANYMETAINFO_TYPE_LIST> vo
                  = vob.buildReverseTimeline(t_listingDate(std::chrono::year(int(2004)), std::chrono::May, std::chrono::day(unsigned(13))),
                                             companyRecordLatestExpected, resetMeta);


//#################### check all versioned objects

//  NOTE : the row below is manually deduced
//  APPAPER,International Paper APPM Limited,EQ,13-MAY-2004,10,1,INE435A01028,10
    t_companyInfo companyInfoStart = converter::ConvertFromString<COMPANYINFO_TYPE_LIST>::ToVal(
      "APPAPER,International Paper APPM Limited,EQ,10,1,INE435A01028,10"    );

    versionedObject::MetaDataSource symChgMetaExp("-symbolchange",'-');
    versionedObject::DataSet<COMPANYMETAINFO_TYPE_LIST> companyRecordStart {symChgMetaExp, companyInfoStart};

    std::optional<versionedObject::DataSet<COMPANYMETAINFO_TYPE_LIST>> companyRecordFirstActual =
      vo.getVersionAt(t_listingDate(std::chrono::year(int(2004)), std::chrono::May, std::chrono::day(unsigned(13))));

    unittest::ExpectEqual(bool, true, companyRecordFirstActual.has_value()); // has versionedObject::DataSet<COMPANYMETAINFO_TYPE_LIST>

    unittest::ExpectEqual(versionedObject::DataSet<COMPANYMETAINFO_TYPE_LIST>, companyRecordStart,
                                                                               companyRecordFirstActual.value());


//  ANDHRA PAPER LIMITED,APPAPER,IPAPPM,21-JAN-2014
    t_companyInfo companyInfoSecond = converter::ConvertFromString<COMPANYINFO_TYPE_LIST>::ToVal(
      "IPAPPM,International Paper APPM Limited,EQ,10,1,INE435A01028,10"    );

    versionedObject::MetaDataSource symChgNamChgMetaExp("-namechange-symbolchange",'-');
    versionedObject::DataSet<COMPANYMETAINFO_TYPE_LIST> companyRecordSecondExpected {symChgNamChgMetaExp, companyInfoSecond};

    std::optional<versionedObject::DataSet<COMPANYMETAINFO_TYPE_LIST>> companyRecordSecondActual =
      vo.getVersionAt(t_listingDate(std::chrono::year(int(2014)), std::chrono::January, std::chrono::day(unsigned(21))));

    unittest::ExpectEqual(bool, true, companyRecordSecondActual.has_value()); // has versionedObject::DataSet<COMPANYMETAINFO_TYPE_LIST>

    unittest::ExpectEqual(versionedObject::DataSet<COMPANYMETAINFO_TYPE_LIST>, companyRecordSecondExpected,
                                                                               companyRecordSecondActual.value());


//  ANDHRA PAPER LIMITED,IPAPPM,ANDPAPER,22-JAN-2020
//  ANDHRAPAP,International Paper APPM Limited,ANDHRA PAPER LIMITED,22-JAN-2020
    t_companyInfo companyInfoThird = converter::ConvertFromString<COMPANYINFO_TYPE_LIST>::ToVal(
      "ANDPAPER,ANDHRA PAPER LIMITED,EQ,10,1,INE435A01028,10"    );

    versionedObject::DataSet<COMPANYMETAINFO_TYPE_LIST> companyRecordThirdExpected {symChgMetaExp, companyInfoThird};

    std::optional<versionedObject::DataSet<COMPANYMETAINFO_TYPE_LIST>> companyRecordThirdActual =
      vo.getVersionAt(t_listingDate(std::chrono::year(int(2020)), std::chrono::January, std::chrono::day(unsigned(22))));

    unittest::ExpectEqual(bool, true, companyRecordThirdActual.has_value()); // has versionedObject::DataSet<COMPANYMETAINFO_TYPE_LIST>

    unittest::ExpectEqual(versionedObject::DataSet<COMPANYMETAINFO_TYPE_LIST>, companyRecordThirdExpected,
                                                                               companyRecordThirdActual.value());


//  ANDHRA PAPER LIMITED,ANDPAPER,ANDHRAPAP,05-MAR-2020
//  NOTE: the row below is not a versioned information, but info from EQUITY_L.csv
//       ANDHRAPAP,ANDHRA PAPER LIMITED,EQ,13-MAY-2004,10,1,INE435A01028,10
    std::optional<versionedObject::DataSet<COMPANYMETAINFO_TYPE_LIST>> companyRecordLatestActual =
      vo.getVersionAt(t_listingDate(std::chrono::year(int(2020)), std::chrono::March, std::chrono::day(unsigned(5))));

    unittest::ExpectEqual(bool, true, companyRecordLatestActual.has_value()); // has versionedObject::DataSet<COMPANYMETAINFO_TYPE_LIST>

    unittest::ExpectEqual(versionedObject::DataSet<COMPANYMETAINFO_TYPE_LIST>, companyRecordLatestExpected,
                                                                               companyRecordLatestActual.value());
  } catch (const std::exception& ex) {
    std::cout << ex.what() << std::endl;
    rv = 1;
  }

  return rv;
}
