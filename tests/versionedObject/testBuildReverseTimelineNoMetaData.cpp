
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
using t_versionObject = dsvo::VersionedObject<t_fmtdbY, COMPANYINFO_TYPE_LIST>;


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


int main()
{
  int rv = 0;
  try {
    dsvo::VersionedObjectBuilder<t_versionDate, COMPANYINFO_TYPE_LIST> vob;

    bool insertResult;

/* these rows are from symbolchange.csv
CompanyName, OLD-SYMBOL, NEW-SYMBOL, CHANGE-DATE
ANDHRA PAPER LIMITED,APPAPER,IPAPPM,21-JAN-2014
ANDHRA PAPER LIMITED,IPAPPM,ANDPAPER,22-JAN-2020
ANDHRA PAPER LIMITED,ANDPAPER,ANDHRAPAP,05-MAR-2020
*/
    const std::array <bool, std::tuple_size_v<t_companyInfo> > symbolChangeFlg = {true, false, false, false, false, false, false};

    t_companyInfo symChgOldInfo1 = converter::ConvertFromString<COMPANYINFO_TYPE_LIST>::ToVal("APPAPER,,,0,0,,0");
    t_companyInfo symChgNewInfo1 = converter::ConvertFromString<COMPANYINFO_TYPE_LIST>::ToVal("IPAPPM,,,0,0,,0");
    dsvo::ChangesInDataSet<COMPANYINFO_TYPE_LIST> symbolChange1 {symbolChangeFlg, symChgOldInfo1, symChgNewInfo1, dsvo::ApplicableChangeDirection::REVERSE};  // DELTA Change
    insertResult = vob.insertDeltaVersion(t_versionDate{std::chrono::year(int(2014)), std::chrono::January, std::chrono::day(unsigned(21))}, symbolChange1);
    unittest::ExpectEqual(bool, true, insertResult);

    t_companyInfo symChgOldInfo2 = converter::ConvertFromString<COMPANYINFO_TYPE_LIST>::ToVal("IPAPPM,,,0,0,,0");
    t_companyInfo symChgNewInfo2 = converter::ConvertFromString<COMPANYINFO_TYPE_LIST>::ToVal("ANDPAPER,,,0,0,,0");
    dsvo::ChangesInDataSet<COMPANYINFO_TYPE_LIST> symbolChange2 { symbolChangeFlg, symChgOldInfo2, symChgNewInfo2, dsvo::ApplicableChangeDirection::REVERSE};  // DELTA Change
    insertResult = vob.insertDeltaVersion(t_versionDate{std::chrono::year(int(2020)), std::chrono::January, std::chrono::day(unsigned(22))}, symbolChange2);
    unittest::ExpectEqual(bool, true, insertResult);

    t_companyInfo symChgOldInfo3 = converter::ConvertFromString<COMPANYINFO_TYPE_LIST>::ToVal("ANDPAPER,,,0,0,,0");
    t_companyInfo symChgNewInfo3 = converter::ConvertFromString<COMPANYINFO_TYPE_LIST>::ToVal("ANDHRAPAP,,,0,0,,0");
    dsvo::ChangesInDataSet<COMPANYINFO_TYPE_LIST> symbolChange3 {symbolChangeFlg, symChgOldInfo3, symChgNewInfo3, dsvo::ApplicableChangeDirection::REVERSE};  // DELTA Change

    t_versionDate reverseLastVersionDate{std::chrono::year(int(2020)), std::chrono::March, std::chrono::day(unsigned(5))};
    insertResult = vob.insertDeltaVersion(reverseLastVersionDate, symbolChange3);
    unittest::ExpectEqual(bool, true, insertResult);


    const std::array <bool, std::tuple_size_v<t_companyInfo> > nameChangeFlg = {false, true, false, false, false, false, false};

//  NOTE : the row below is from namechange.csv
//  SYMBOL, OLD-CompanyName, NEW-CompanyName, CHANGE-DATE
//  ANDHRAPAP,International Paper APPM Limited,ANDHRA PAPER LIMITED,22-JAN-2020
    t_companyInfo namChgOldInfo1 = converter::ConvertFromString<COMPANYINFO_TYPE_LIST>::ToVal(",International Paper APPM Limited,,0,0,,0");
    t_companyInfo namChgNewInfo1 = converter::ConvertFromString<COMPANYINFO_TYPE_LIST>::ToVal(",ANDHRA PAPER LIMITED,,0,0,,0");
    dsvo::ChangesInDataSet<COMPANYINFO_TYPE_LIST> nameChange1 {nameChangeFlg, namChgOldInfo1, namChgNewInfo1, dsvo::ApplicableChangeDirection::REVERSE};  // DELTA Change
    insertResult = vob.insertDeltaVersion(t_versionDate{std::chrono::year(int(2020)), std::chrono::January, std::chrono::day(unsigned(22))}, nameChange1);
    unittest::ExpectEqual(bool, true, insertResult);



//  NOTE : the row below is not a versioned information, but info from EQUITY_L.csv
//       ANDHRAPAP,ANDHRA PAPER LIMITED,EQ,13-MAY-2004,10,1,INE435A01028,10
    t_companyInfo companyInfoLatest = converter::ConvertFromString<COMPANYINFO_TYPE_LIST>::ToVal(
      "ANDHRAPAP,ANDHRA PAPER LIMITED,EQ,10,1,INE435A01028,10"    );

    dsvo::DataSet<COMPANYINFO_TYPE_LIST> companyRecordLatestExpected {companyInfoLatest};

    dsvo::VersionedObject<t_versionDate, COMPANYINFO_TYPE_LIST> vo;
    insertResult = vo.insertVersion(reverseLastVersionDate, companyRecordLatestExpected);
    unittest::ExpectEqual(bool, true, insertResult);
    vob.buildReverseTimeline( t_versionDate{std::chrono::year(int(2004)), std::chrono::May, std::chrono::day(unsigned(13))},
                              vo);

    //std::cout << "#### vo start ######\n" << vo.toCSV() << "#### vo end ######\n";
    std::string voStrBack =
      "13-May-2004,APPAPER,International Paper APPM Limited,EQ,10,1,INE435A01028,10\n"
      "21-Jan-2014,IPAPPM,International Paper APPM Limited,EQ,10,1,INE435A01028,10\n"
      "22-Jan-2020,ANDPAPER,ANDHRA PAPER LIMITED,EQ,10,1,INE435A01028,10\n"
      "05-Mar-2020,ANDHRAPAP,ANDHRA PAPER LIMITED,EQ,10,1,INE435A01028,10\n";

    unittest::ExpectEqual(std::string, voStrBack, vo.toCSV());

    ////////////////////////              SNAPSHOT change test
    const std::array <bool, std::tuple_size_v<t_companyInfo> > lotChangeFlg = {false, false, false, false, true, false, false};

    t_companyInfo lotChgInfo = converter::ConvertFromString<COMPANYINFO_TYPE_LIST>::ToVal(",,,0,2,,0");
    dsvo::ChangesInDataSet<COMPANYINFO_TYPE_LIST> lotChange {lotChangeFlg, lotChgInfo};   // SNAPSHOT Change
    insertResult = vob.insertDeltaVersion(t_versionDate{std::chrono::year(int(2021)), std::chrono::April, std::chrono::day(unsigned(07))}, lotChange);
    unittest::ExpectEqual(bool, true, insertResult);


    dsvo::VersionedObjectBuilder<t_fmtdbY, COMPANYINFO_TYPE_LIST> vobCopy{vob};


    // resetMetaForward for cloning in _VersionedObjectBuilderBase<MT...>::_buildForwardTimeline()
    vob.buildForwardTimeline(vo);


//#################### check all versioned objects

    //std::cout << "#### vo start ######\n" << vo.toCSV() << "#### vo end ######\n";
    std::string voStrForward = voStrBack +
      "07-Apr-2021,ANDHRAPAP,ANDHRA PAPER LIMITED,EQ,10,2,INE435A01028,10\n";

    unittest::ExpectEqual(std::string, voStrForward, vo.toCSV());

//  NOTE : the row below is manually deduced
//  APPAPER,International Paper APPM Limited,EQ,13-MAY-2004,10,1,INE435A01028,10
    t_versionDate  listingDate{std::chrono::year(int(2004)), std::chrono::May, std::chrono::day(unsigned(13))};
    t_companyInfo companyInfoStart = converter::ConvertFromString<COMPANYINFO_TYPE_LIST>::ToVal(
      "APPAPER,International Paper APPM Limited,EQ,10,1,INE435A01028,10"    );

    dsvo::DataSet<COMPANYINFO_TYPE_LIST> companyRecordStart {companyInfoStart};

    t_versionObject::t_datasetLedger::const_iterator companyRecordFirstActual =
      vo.getVersionAt(listingDate);

    unittest::ExpectEqual(bool, true, companyRecordFirstActual != vo.getDatasetLedger().cend()); // has dsvo::DataSet<COMPANYINFO_TYPE_LIST>
    unittest::ExpectEqual(dsvo::DataSet<COMPANYINFO_TYPE_LIST>, companyRecordStart,
                                                                companyRecordFirstActual->second);
    unittest::ExpectEqual(t_versionDate, listingDate,
                                         companyRecordFirstActual->first);
    ////////
    t_versionDate  prelistingDate{std::chrono::year(int(2004)), std::chrono::May, std::chrono::day(unsigned(12))};
    t_versionObject::t_datasetLedger::const_iterator companyRecordPreFirstActual =
      vo.getVersionAt(prelistingDate);
    unittest::ExpectEqual(bool, true, companyRecordPreFirstActual == vo.getDatasetLedger().cend()); // no record dsvo::DataSet<COMPANYINFO_TYPE_LIST>
    ////////
    t_versionDate  postlistingDate{std::chrono::year(int(2004)), std::chrono::May, std::chrono::day(unsigned(14))};
    t_versionObject::t_datasetLedger::const_iterator companyRecordPostFirstActual =
      vo.getVersionAt(postlistingDate);
    unittest::ExpectEqual(bool, true, companyRecordPostFirstActual != vo.getDatasetLedger().cend()); // has record dsvo::DataSet<COMPANYINFO_TYPE_LIST>
    unittest::ExpectEqual(dsvo::DataSet<COMPANYINFO_TYPE_LIST>, companyRecordStart,
                                                                companyRecordPostFirstActual->second);
    unittest::ExpectEqual(t_versionDate, listingDate,
                                         companyRecordPostFirstActual->first);


//  ANDHRA PAPER LIMITED,APPAPER,IPAPPM,21-JAN-2014
    t_companyInfo companyInfoSecond = converter::ConvertFromString<COMPANYINFO_TYPE_LIST>::ToVal(
      "IPAPPM,International Paper APPM Limited,EQ,10,1,INE435A01028,10"    );

    dsvo::DataSet<COMPANYINFO_TYPE_LIST> companyRecordSecondExpected {companyInfoSecond};

    t_versionDate secondDate{std::chrono::year(int(2014)), std::chrono::January, std::chrono::day(unsigned(21))};
    t_versionObject::t_datasetLedger::const_iterator companyRecordSecondActual =
      vo.getVersionAt(secondDate);

    unittest::ExpectEqual(bool, true, companyRecordSecondActual != vo.getDatasetLedger().cend()); // has dsvo::DataSet<COMPANYINFO_TYPE_LIST>
    unittest::ExpectEqual(dsvo::DataSet<COMPANYINFO_TYPE_LIST>, companyRecordSecondExpected,
                                                                companyRecordSecondActual->second);
    unittest::ExpectEqual(t_versionDate, secondDate,
                                         companyRecordSecondActual->first);
    ////////
    t_versionDate  preSecondDate{std::chrono::year(int(2014)), std::chrono::January, std::chrono::day(unsigned(20))};
    t_versionObject::t_datasetLedger::const_iterator companyRecordPreSecondActual =
      vo.getVersionAt(preSecondDate);
    unittest::ExpectEqual(bool, true, companyRecordPreSecondActual != vo.getDatasetLedger().cend()); // has record dsvo::DataSet<COMPANYINFO_TYPE_LIST>
    unittest::ExpectEqual(dsvo::DataSet<COMPANYINFO_TYPE_LIST>, companyRecordStart,
                                                                companyRecordPreSecondActual->second);
    unittest::ExpectEqual(t_versionDate, listingDate,
                                         companyRecordPreSecondActual->first);
    ////////
    t_versionDate  postSecondDate{std::chrono::year(int(2014)), std::chrono::January, std::chrono::day(unsigned(22))};
    t_versionObject::t_datasetLedger::const_iterator companyRecordPostSecondActual =
      vo.getVersionAt(postSecondDate);
    unittest::ExpectEqual(bool, true, companyRecordPostFirstActual != vo.getDatasetLedger().cend()); // has record dsvo::DataSet<COMPANYINFO_TYPE_LIST>
    unittest::ExpectEqual(dsvo::DataSet<COMPANYINFO_TYPE_LIST>, companyRecordSecondExpected,
                                                                companyRecordPostSecondActual->second);
    unittest::ExpectEqual(t_versionDate, listingDate,
                                         companyRecordPostFirstActual->first);


//  ANDHRA PAPER LIMITED,IPAPPM,ANDPAPER,22-JAN-2020
//  ANDHRAPAP,International Paper APPM Limited,ANDHRA PAPER LIMITED,22-JAN-2020
    t_companyInfo companyInfoThird = converter::ConvertFromString<COMPANYINFO_TYPE_LIST>::ToVal(
      "ANDPAPER,ANDHRA PAPER LIMITED,EQ,10,1,INE435A01028,10"    );

    dsvo::DataSet<COMPANYINFO_TYPE_LIST> companyRecordThirdExpected {companyInfoThird};

    t_versionObject::t_datasetLedger::const_iterator companyRecordThirdActual =
      vo.getVersionAt(t_versionDate{std::chrono::year(int(2020)), std::chrono::January, std::chrono::day(unsigned(22))});

    unittest::ExpectEqual(bool, true, companyRecordThirdActual != vo.getDatasetLedger().cend()); // has dsvo::DataSet<COMPANYINFO_TYPE_LIST>

    unittest::ExpectEqual(dsvo::DataSet<COMPANYINFO_TYPE_LIST>, companyRecordThirdExpected,
                                                                companyRecordThirdActual->second);


//  ANDHRA PAPER LIMITED,ANDPAPER,ANDHRAPAP,05-MAR-2020
//  NOTE: the row below is not a versioned information, but info from EQUITY_L.csv
//       ANDHRAPAP,ANDHRA PAPER LIMITED,EQ,13-MAY-2004,10,1,INE435A01028,10
    t_versionObject::t_datasetLedger::const_iterator companyRecordLatestActual =
      vo.getVersionAt(t_versionDate{std::chrono::year(int(2020)), std::chrono::March, std::chrono::day(unsigned(5))});

    unittest::ExpectEqual(bool, true, companyRecordLatestActual != vo.getDatasetLedger().cend()); // has dsvo::DataSet<COMPANYINFO_TYPE_LIST>

    unittest::ExpectEqual(dsvo::DataSet<COMPANYINFO_TYPE_LIST>, companyRecordLatestExpected,
                                                                companyRecordLatestActual->second);


//  ",,,0,2,,0"
//  ANDHRAPAP,ANDHRA PAPER LIMITED,EQ,10,2,INE435A01028,10
    t_companyInfo companyInfoFifth = converter::ConvertFromString<COMPANYINFO_TYPE_LIST>::ToVal(
      "ANDHRAPAP,ANDHRA PAPER LIMITED,EQ,10,2,INE435A01028,10"    );

    dsvo::DataSet<COMPANYINFO_TYPE_LIST> companyRecordFifthExpected {companyInfoFifth};

    t_versionObject::t_datasetLedger::const_iterator companyRecordFifthActual =
      vo.getVersionAt(t_versionDate{std::chrono::year(int(2021)), std::chrono::April, std::chrono::day(unsigned(07))});

    unittest::ExpectEqual(bool, true, companyRecordFifthActual != vo.getDatasetLedger().cend()); // has dsvo::DataSet<COMPANYINFO_TYPE_LIST>

    unittest::ExpectEqual(dsvo::DataSet<COMPANYINFO_TYPE_LIST>, companyRecordFifthExpected,
                                                                companyRecordFifthActual->second);

/*
//std::cout << "#### insertFailure start ######\n" << insertFailure.toCSV() << "#### insertFailure end ######\n";
    std::string insertFailureStrBack =
      "13-May-2004,APPAPER,International Paper APPM Limited,EQ,10,1,INE435A01028,10\n"
      "21-Jan-2014,IPAPPM,International Paper APPM Limited,EQ,10,1,INE435A01028,10\n"
      "22-Jan-2020,ANDPAPER,ANDHRA PAPER LIMITED,EQ,10,1,INE435A01028,10\n"
      "05-Mar-2020,ANDHRAPAP,ANDHRA PAPER LIMITED,EQ,10,1,INE435A01028,10\n";

    vobCopy.buildForwardTimeline(vo, insertFailure);
    unittest::ExpectEqual(std::string, insertFailureStrBack, insertFailure.toCSV());
*/
  } catch (const std::exception& ex) {
    std::cout << ex.what() << std::endl;
    rv = 1;
  }

  return rv;
}
