
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
using t_status      = std::string;

#define COMPANYINFO_TYPE_LIST t_symbol, t_companyName, t_series,     \
                              t_paidUpValue, t_marketLot, t_isinNumber, t_faceValue,  t_status
using t_companyInfo = std::tuple<COMPANYINFO_TYPE_LIST>;

namespace dsvo = datastructure::versionedObject;

#define COMPANYMETAINFO_TYPE_LIST  dsvo::MetaDataSource, COMPANYINFO_TYPE_LIST

using t_versionDate = t_fmtdbY; // std::chrono::year_month_day;
using t_versionObject = dsvo::VersionedObject<t_versionDate, COMPANYMETAINFO_TYPE_LIST>;


namespace unittest
{
  template<>
  struct SScompatible<dsvo::DataSet<COMPANYMETAINFO_TYPE_LIST>> {
    inline static std::string getVal(const dsvo::DataSet<COMPANYMETAINFO_TYPE_LIST>& val)
    {
      return val.toCSV();
    }
  };
}


int main()
{
  int rv = 0;
  try {
    dsvo::VersionedObjectBuilder<t_fmtdbY, COMPANYMETAINFO_TYPE_LIST> vob;

    bool insertResult;

/* these rows are from symbolchange.csv
CompanyName, OLD-SYMBOL, NEW-SYMBOL, CHANGE-DATE
ANDHRA PAPER LIMITED,APPAPER,IPAPPM,21-JAN-2014
ANDHRA PAPER LIMITED,IPAPPM,ANDPAPER,22-JAN-2020
ANDHRA PAPER LIMITED,ANDPAPER,ANDHRAPAP,05-MAR-2020
*/
    dsvo::MetaDataSource symChgMeta("symbolchange",'-');
    const std::array <bool, std::tuple_size_v<t_companyInfo> > symbolChangeFlg = {true, false, false, false, false, false, false, false};

    t_companyInfo symChgOldInfo1 = converter::ConvertFromString<COMPANYINFO_TYPE_LIST>::ToVal("APPAPER,,,0,0,,0,");
    t_companyInfo symChgNewInfo1 = converter::ConvertFromString<COMPANYINFO_TYPE_LIST>::ToVal("IPAPPM,,,0,0,,0,");
    dsvo::ChangesInDataSet<COMPANYMETAINFO_TYPE_LIST> symbolChange1 {symChgMeta, symbolChangeFlg, symChgOldInfo1, symChgNewInfo1, dsvo::ApplicableChangeDirection::REVERSE};  // DELTA Change
    insertResult = vob.insertDeltaVersion(t_versionDate{std::chrono::year(int(2014)), std::chrono::January, std::chrono::day(unsigned(21))}, symbolChange1);
    unittest::ExpectEqual(bool, true, insertResult);

    t_companyInfo symChgOldInfo2 = converter::ConvertFromString<COMPANYINFO_TYPE_LIST>::ToVal("IPAPPM,,,0,0,,0,");
    t_companyInfo symChgNewInfo2 = converter::ConvertFromString<COMPANYINFO_TYPE_LIST>::ToVal("ANDPAPER,,,0,0,,0,");
    dsvo::ChangesInDataSet<COMPANYMETAINFO_TYPE_LIST> symbolChange2 {symChgMeta, symbolChangeFlg, symChgOldInfo2, symChgNewInfo2, dsvo::ApplicableChangeDirection::REVERSE};  // DELTA Change
    insertResult = vob.insertDeltaVersion(t_versionDate{std::chrono::year(int(2020)), std::chrono::January, std::chrono::day(unsigned(22))}, symbolChange2);
    unittest::ExpectEqual(bool, true, insertResult);

    t_companyInfo symChgOldInfo3 = converter::ConvertFromString<COMPANYINFO_TYPE_LIST>::ToVal("ANDPAPER,,,0,0,,0,");
    t_companyInfo symChgNewInfo3 = converter::ConvertFromString<COMPANYINFO_TYPE_LIST>::ToVal("ANDHRAPAP,,,0,0,,0,");
    dsvo::ChangesInDataSet<COMPANYMETAINFO_TYPE_LIST> symbolChange3 {symChgMeta, symbolChangeFlg, symChgOldInfo3, symChgNewInfo3, dsvo::ApplicableChangeDirection::REVERSE};  // DELTA Change
    t_versionDate reverseLastVersionDate{std::chrono::year(int(2020)), std::chrono::March, std::chrono::day(unsigned(5))};
    insertResult = vob.insertDeltaVersion(reverseLastVersionDate, symbolChange3);
    unittest::ExpectEqual(bool, true, insertResult);


    dsvo::MetaDataSource namChgMeta("namechange",'-');
    const std::array <bool, std::tuple_size_v<t_companyInfo> > nameChangeFlg = {false, true, false, false, false, false, false, false};

//  NOTE : the row below is from namechange.csv
//  SYMBOL, OLD-CompanyName, NEW-CompanyName, CHANGE-DATE
//  ANDHRAPAP,International Paper APPM Limited,ANDHRA PAPER LIMITED,22-JAN-2020
    t_companyInfo namChgOldInfo1 = converter::ConvertFromString<COMPANYINFO_TYPE_LIST>::ToVal(",International Paper APPM Limited,,0,0,,0,");
    t_companyInfo namChgNewInfo1 = converter::ConvertFromString<COMPANYINFO_TYPE_LIST>::ToVal(",ANDHRA PAPER LIMITED,,0,0,,0,");
    dsvo::ChangesInDataSet<COMPANYMETAINFO_TYPE_LIST> nameChange1 {namChgMeta, nameChangeFlg, namChgOldInfo1, namChgNewInfo1, dsvo::ApplicableChangeDirection::REVERSE};  // DELTA Change
    insertResult = vob.insertDeltaVersion(t_versionDate{std::chrono::year(int(2020)), std::chrono::January, std::chrono::day(unsigned(22))}, nameChange1);
    unittest::ExpectEqual(bool, true, insertResult);



//  NOTE : the row below is not a versioned information, but info from EQUITY_L.csv
//       ANDHRAPAP,ANDHRA PAPER LIMITED,EQ,13-MAY-2004,10,1,INE435A01028,10
    t_companyInfo companyInfoFirst = converter::ConvertFromString<COMPANYINFO_TYPE_LIST>::ToVal(
      "ANDHRAPAP,ANDHRA PAPER LIMITED,EQ,10,1,INE435A01028,10,LISTING"    );

    dsvo::MetaDataSource firstMeta("EQUITY_L",'^');
    dsvo::DataSet<COMPANYMETAINFO_TYPE_LIST> companyRecordFirstExpected {firstMeta, companyInfoFirst};

    // resetMetaBack for cloning in _VersionedObjectBuilderBase<MT...>::_buildReverseTimeline()
    dsvo::MetaDataSource resetMetaReverse("",'-');
    dsvo::VersionedObject<t_fmtdbY, COMPANYMETAINFO_TYPE_LIST> vo;
    insertResult = vo.insertVersion(reverseLastVersionDate, companyRecordFirstExpected);
    unittest::ExpectEqual(bool, true, insertResult);
    std::vector<decltype(vob)::t_versionDate> startDates{};
    startDates.push_back(t_versionDate{std::chrono::year(int(2004)), std::chrono::May, std::chrono::day(unsigned(13))});

    ////////////////////////              SNAPSHOT change test
    dsvo::MetaDataSource lotChgMeta("marketLotchange",'+');
    const std::array <bool, std::tuple_size_v<t_companyInfo> > lotChangeFlg = {false, false, false, false, true, false, false, false};

    t_companyInfo lotChgInfo = converter::ConvertFromString<COMPANYINFO_TYPE_LIST>::ToVal(",,,0,2,,0,");
    dsvo::ChangesInDataSet<COMPANYMETAINFO_TYPE_LIST> lotChange {lotChgMeta, lotChangeFlg, lotChgInfo};   // SNAPSHOT Change
    insertResult = vob.insertDeltaVersion(t_versionDate{std::chrono::year(int(2021)), std::chrono::April, std::chrono::day(unsigned(07))}, lotChange);
    unittest::ExpectEqual(bool, true, insertResult);

    ////////////////////////              DELISTED change test
    dsvo::MetaDataSource delistedChgMeta("suspended",'$');
    const std::array <bool, std::tuple_size_v<t_companyInfo> > delistedChangeFlg = {false, false, false, false, false, false, false, true};

    t_companyInfo delistedChgInfo = converter::ConvertFromString<COMPANYINFO_TYPE_LIST>::ToVal(",,,0,0,,0,DELISTED");
    dsvo::ChangesInDataSet<COMPANYMETAINFO_TYPE_LIST> delistedChange {delistedChgMeta, delistedChangeFlg, delistedChgInfo};   // DELISTED Change
    insertResult = vob.insertDeltaVersion(t_versionDate{std::chrono::year(int(2021)), std::chrono::December, std::chrono::day(unsigned(17))}, delistedChange);
    unittest::ExpectEqual(bool, true, insertResult);

    ////////////////////////              RELISTED change & symbolchange test
    startDates.push_back(t_versionDate{std::chrono::year(int(2022)), std::chrono::January, std::chrono::day(unsigned(12))});

    t_companyInfo symChgOldInfo4 = converter::ConvertFromString<COMPANYINFO_TYPE_LIST>::ToVal("ANDHRAPAPER,,,0,0,,0,");
    t_companyInfo symChgNewInfo4 = converter::ConvertFromString<COMPANYINFO_TYPE_LIST>::ToVal("ANDHRAPAPLTD,,,0,0,,0,");
    dsvo::ChangesInDataSet<COMPANYMETAINFO_TYPE_LIST> symbolChange4 {symChgMeta, symbolChangeFlg, symChgOldInfo4, symChgNewInfo4, dsvo::ApplicableChangeDirection::REVERSE};  // DELTA Change
    insertResult = vob.insertDeltaVersion(t_versionDate{std::chrono::year(int(2022)), std::chrono::September, std::chrono::day(unsigned(19))}, symbolChange4);
    unittest::ExpectEqual(bool, true, insertResult);

    t_companyInfo companyInfoAfterRelisting = converter::ConvertFromString<COMPANYINFO_TYPE_LIST>::ToVal(
      "ANDHRAPAPLTD,ANDHRA PAPER LIMITED,EQ,10,1,INE546B12139,10,LISTING"    );

    dsvo::DataSet<COMPANYMETAINFO_TYPE_LIST> companyRecordAfterRelistedExpected {firstMeta, companyInfoAfterRelisting};
    insertResult = vo.insertVersion(t_versionDate{std::chrono::year(int(2022)), std::chrono::September, std::chrono::day(unsigned(19))},
                                    companyRecordAfterRelistedExpected);
    unittest::ExpectEqual(bool, true, insertResult);

    ////////////////////////              SNAPSHOT change test
    t_companyInfo lotChgInfo2 = converter::ConvertFromString<COMPANYINFO_TYPE_LIST>::ToVal(",,,0,5,,0,");
    dsvo::ChangesInDataSet<COMPANYMETAINFO_TYPE_LIST> lotChange2 {lotChgMeta, lotChangeFlg, lotChgInfo2};   // SNAPSHOT Change
    insertResult = vob.insertDeltaVersion(t_versionDate{std::chrono::year(int(2023)), std::chrono::October, std::chrono::day(unsigned(28))}, lotChange2);
    unittest::ExpectEqual(bool, true, insertResult);


    // resetMetaForward for cloning in _VersionedObjectBuilderBase<MT...>::_buildForwardTimeline()
    dsvo::MetaDataSource resetMetaForward("",'+');

    vob.buildBiDirectionalTimeline( startDates, vo, resetMetaReverse, resetMetaForward);

    //std::cout << "#### vo start ######\n" << vo.toCSV() << "#### vo end ######\n";
    std::string voStrBidirection =
      "13-May-2004,-symbolchange,APPAPER,International Paper APPM Limited,EQ,10,1,INE435A01028,10,LISTING\n"
      "21-Jan-2014,-namechange-symbolchange,IPAPPM,International Paper APPM Limited,EQ,10,1,INE435A01028,10,LISTING\n"
      "22-Jan-2020,-symbolchange,ANDPAPER,ANDHRA PAPER LIMITED,EQ,10,1,INE435A01028,10,LISTING\n"
      "05-Mar-2020,EQUITY_L,ANDHRAPAP,ANDHRA PAPER LIMITED,EQ,10,1,INE435A01028,10,LISTING\n"
      "07-Apr-2021,+marketLotchange,ANDHRAPAP,ANDHRA PAPER LIMITED,EQ,10,2,INE435A01028,10,LISTING\n"
      "17-Dec-2021,$suspended,ANDHRAPAP,ANDHRA PAPER LIMITED,EQ,10,2,INE435A01028,10,DELISTED\n"
      "12-Jan-2022,-symbolchange,ANDHRAPAPER,ANDHRA PAPER LIMITED,EQ,10,1,INE546B12139,10,LISTING\n"
      "19-Sep-2022,EQUITY_L,ANDHRAPAPLTD,ANDHRA PAPER LIMITED,EQ,10,1,INE546B12139,10,LISTING\n"
      "28-Oct-2023,+marketLotchange,ANDHRAPAPLTD,ANDHRA PAPER LIMITED,EQ,10,5,INE546B12139,10,LISTING\n";

    unittest::ExpectEqual(std::string, voStrBidirection, vo.toCSV());


//#################### check all versioned objects

//  NOTE : the row below is manually deduced
//  APPAPER,International Paper APPM Limited,EQ,13-MAY-2004,10,1,INE435A01028,10
    t_versionDate  listingDate{std::chrono::year(int(2004)), std::chrono::May, std::chrono::day(unsigned(13))};
    t_companyInfo companyInfoStart = converter::ConvertFromString<COMPANYINFO_TYPE_LIST>::ToVal(
      "APPAPER,International Paper APPM Limited,EQ,10,1,INE435A01028,10,LISTING"    );

    dsvo::MetaDataSource symChgMetaExp("-symbolchange",'-');
    dsvo::DataSet<COMPANYMETAINFO_TYPE_LIST> companyRecordStart {symChgMetaExp, companyInfoStart};

    t_versionObject::t_datasetLedger::const_iterator companyRecordFirstActual =
      vo.getVersionAt(listingDate);

    unittest::ExpectEqual(bool, true, companyRecordFirstActual != vo.getDatasetLedger().cend()); // has dsvo::DataSet<COMPANYMETAINFO_TYPE_LIST>
    unittest::ExpectEqual(dsvo::DataSet<COMPANYMETAINFO_TYPE_LIST>, companyRecordStart,
                                                                    companyRecordFirstActual->second);
    unittest::ExpectEqual(t_versionDate, listingDate,
                                         companyRecordFirstActual->first);
    ////////
    t_versionDate  prelistingDate{std::chrono::year(int(2004)), std::chrono::May, std::chrono::day(unsigned(12))};
    t_versionObject::t_datasetLedger::const_iterator companyRecordPreFirstActual =
      vo.getVersionAt(prelistingDate);
    unittest::ExpectEqual(bool, true, companyRecordPreFirstActual == vo.getDatasetLedger().cend()); // no record dsvo::DataSet<COMPANYMETAINFO_TYPE_LIST>
    ////////
    t_versionDate  postlistingDate{std::chrono::year(int(2004)), std::chrono::May, std::chrono::day(unsigned(14))};
    t_versionObject::t_datasetLedger::const_iterator companyRecordPostFirstActual =
      vo.getVersionAt(postlistingDate);
    unittest::ExpectEqual(bool, true, companyRecordPostFirstActual != vo.getDatasetLedger().cend()); // has record dsvo::DataSet<COMPANYMETAINFO_TYPE_LIST>
    unittest::ExpectEqual(dsvo::DataSet<COMPANYMETAINFO_TYPE_LIST>, companyRecordStart,
                                                                    companyRecordPostFirstActual->second);
    unittest::ExpectEqual(t_versionDate, listingDate,
                                         companyRecordPostFirstActual->first);


//  ANDHRA PAPER LIMITED,APPAPER,IPAPPM,21-JAN-2014
    t_companyInfo companyInfoSecond = converter::ConvertFromString<COMPANYINFO_TYPE_LIST>::ToVal(
      "IPAPPM,International Paper APPM Limited,EQ,10,1,INE435A01028,10,LISTING"    );

    dsvo::MetaDataSource symChgNamChgMetaExp("-namechange-symbolchange",'-');
    dsvo::DataSet<COMPANYMETAINFO_TYPE_LIST> companyRecordSecondExpected {symChgNamChgMetaExp, companyInfoSecond};

    t_versionDate secondDate{std::chrono::year(int(2014)), std::chrono::January, std::chrono::day(unsigned(21))};
    t_versionObject::t_datasetLedger::const_iterator companyRecordSecondActual =
      vo.getVersionAt(secondDate);

    unittest::ExpectEqual(bool, true, companyRecordSecondActual != vo.getDatasetLedger().cend()); // has dsvo::DataSet<COMPANYMETAINFO_TYPE_LIST>
    unittest::ExpectEqual(dsvo::DataSet<COMPANYMETAINFO_TYPE_LIST>, companyRecordSecondExpected,
                                                                    companyRecordSecondActual->second);
    unittest::ExpectEqual(t_versionDate, secondDate,
                                         companyRecordSecondActual->first);
    ////////
    t_versionDate  preSecondDate{std::chrono::year(int(2014)), std::chrono::January, std::chrono::day(unsigned(20))};
    t_versionObject::t_datasetLedger::const_iterator companyRecordPreSecondActual =
      vo.getVersionAt(preSecondDate);
    unittest::ExpectEqual(bool, true, companyRecordPreSecondActual != vo.getDatasetLedger().cend()); // has record dsvo::DataSet<COMPANYMETAINFO_TYPE_LIST>
    unittest::ExpectEqual(dsvo::DataSet<COMPANYMETAINFO_TYPE_LIST>, companyRecordStart,
                                                                    companyRecordPreSecondActual->second);
    unittest::ExpectEqual(t_versionDate, listingDate,
                                         companyRecordPreSecondActual->first);
    ////////
    t_versionDate  postSecondDate{std::chrono::year(int(2014)), std::chrono::January, std::chrono::day(unsigned(22))};
    t_versionObject::t_datasetLedger::const_iterator companyRecordPostSecondActual =
      vo.getVersionAt(postSecondDate);
    unittest::ExpectEqual(bool, true, companyRecordPostFirstActual != vo.getDatasetLedger().cend()); // has record dsvo::DataSet<COMPANYMETAINFO_TYPE_LIST>
    unittest::ExpectEqual(dsvo::DataSet<COMPANYMETAINFO_TYPE_LIST>, companyRecordSecondExpected,
                                                                    companyRecordPostSecondActual->second);
    unittest::ExpectEqual(t_versionDate, listingDate,
                                         companyRecordPostFirstActual->first);


//  ANDHRA PAPER LIMITED,IPAPPM,ANDPAPER,22-JAN-2020
//  ANDHRAPAP,International Paper APPM Limited,ANDHRA PAPER LIMITED,22-JAN-2020
    t_companyInfo companyInfoThird = converter::ConvertFromString<COMPANYINFO_TYPE_LIST>::ToVal(
      "ANDPAPER,ANDHRA PAPER LIMITED,EQ,10,1,INE435A01028,10,LISTING"    );

    dsvo::DataSet<COMPANYMETAINFO_TYPE_LIST> companyRecordThirdExpected {symChgMetaExp, companyInfoThird};

    t_versionObject::t_datasetLedger::const_iterator companyRecordThirdActual =
      vo.getVersionAt(t_versionDate{std::chrono::year(int(2020)), std::chrono::January, std::chrono::day(unsigned(22))});

    unittest::ExpectEqual(bool, true, companyRecordThirdActual != vo.getDatasetLedger().cend()); // has dsvo::DataSet<COMPANYMETAINFO_TYPE_LIST>

    unittest::ExpectEqual(dsvo::DataSet<COMPANYMETAINFO_TYPE_LIST>, companyRecordThirdExpected,
                                                                    companyRecordThirdActual->second);


//  ANDHRA PAPER LIMITED,ANDPAPER,ANDHRAPAP,05-MAR-2020
//  NOTE: the row below is not a versioned information, but info from EQUITY_L.csv
//       ANDHRAPAP,ANDHRA PAPER LIMITED,EQ,13-MAY-2004,10,1,INE435A01028,10
    t_versionObject::t_datasetLedger::const_iterator companyRecordFourthActual =
      vo.getVersionAt(t_versionDate{std::chrono::year(int(2020)), std::chrono::March, std::chrono::day(unsigned(5))});

    unittest::ExpectEqual(bool, true, companyRecordFourthActual != vo.getDatasetLedger().cend()); // has dsvo::DataSet<COMPANYMETAINFO_TYPE_LIST>

    unittest::ExpectEqual(dsvo::DataSet<COMPANYMETAINFO_TYPE_LIST>, companyRecordFirstExpected,  //TODO change varName companyRecordFirstExpected
                                                                    companyRecordFourthActual->second);


//  ",,,0,2,,0,"
//  ANDHRAPAP,ANDHRA PAPER LIMITED,EQ,10,2,INE435A01028,10
    t_companyInfo companyInfoFifth = converter::ConvertFromString<COMPANYINFO_TYPE_LIST>::ToVal(
      "ANDHRAPAP,ANDHRA PAPER LIMITED,EQ,10,2,INE435A01028,10,LISTING"    );

    dsvo::MetaDataSource lotChgMetaExp("+marketLotchange",'+');
    dsvo::DataSet<COMPANYMETAINFO_TYPE_LIST> companyRecordFifthExpected {lotChgMetaExp, companyInfoFifth};

    t_versionObject::t_datasetLedger::const_iterator companyRecordFifthActual =
      vo.getVersionAt(t_versionDate{std::chrono::year(int(2021)), std::chrono::April, std::chrono::day(unsigned(07))});

    unittest::ExpectEqual(bool, true, companyRecordFifthActual != vo.getDatasetLedger().cend()); // has dsvo::DataSet<COMPANYMETAINFO_TYPE_LIST>

    unittest::ExpectEqual(dsvo::DataSet<COMPANYMETAINFO_TYPE_LIST>, companyRecordFifthExpected,
                                                                    companyRecordFifthActual->second);


//  ",,,0,0,,0,DELISTED"
//  17-Dec-2021,$suspended,ANDHRAPAP,ANDHRA PAPER LIMITED,EQ,10,2,INE435A01028,10,DELISTED
    t_companyInfo companyInfoSixth = converter::ConvertFromString<COMPANYINFO_TYPE_LIST>::ToVal(
      "ANDHRAPAP,ANDHRA PAPER LIMITED,EQ,10,2,INE435A01028,10,DELISTED"    );

    dsvo::MetaDataSource delistedChgMetaExp("$suspended",'$');
    dsvo::DataSet<COMPANYMETAINFO_TYPE_LIST> companyRecordSixthExpected {delistedChgMetaExp, companyInfoSixth};

    t_versionObject::t_datasetLedger::const_iterator companyRecordSixthActual =
      vo.getVersionAt(t_versionDate{std::chrono::year(int(2021)), std::chrono::December, std::chrono::day(unsigned(17))});

    unittest::ExpectEqual(bool, true, companyRecordSixthActual != vo.getDatasetLedger().cend()); // has dsvo::DataSet<COMPANYMETAINFO_TYPE_LIST>

    unittest::ExpectEqual(dsvo::DataSet<COMPANYMETAINFO_TYPE_LIST>, companyRecordSixthExpected,
                                                                    companyRecordSixthActual->second);


//  ANDHRA PAPER LIMITED,ANDHRAPAPER,ANDHRAPAPLTD,12-JAN-2022
//  12-Jan-2022,-symbolchange,ANDHRAPAPER,ANDHRA PAPER LIMITED,EQ,10,1,INE546B12139,10,LISTING
    t_companyInfo companyInfoRelisted = converter::ConvertFromString<COMPANYINFO_TYPE_LIST>::ToVal(
      "ANDHRAPAPER,ANDHRA PAPER LIMITED,EQ,10,1,INE546B12139,10,LISTING"    );

    dsvo::DataSet<COMPANYMETAINFO_TYPE_LIST> companyRecordRelistedExpected {symChgMetaExp, companyInfoRelisted};

    t_versionObject::t_datasetLedger::const_iterator companyRecordRelistedActual =
      vo.getVersionAt(t_versionDate{std::chrono::year(int(2022)), std::chrono::January, std::chrono::day(unsigned(12))});

    unittest::ExpectEqual(bool, true, companyRecordRelistedActual != vo.getDatasetLedger().cend()); // has dsvo::DataSet<COMPANYMETAINFO_TYPE_LIST>

    unittest::ExpectEqual(dsvo::DataSet<COMPANYMETAINFO_TYPE_LIST>, companyRecordRelistedExpected,
                                                                    companyRecordRelistedActual->second);



//  NOTE: the row below is not a versioned information, but info from EQUITY_L.csv
//  19-Sep-2022,EQUITY_L,ANDHRAPAPLTD,ANDHRA PAPER LIMITED,EQ,10,1,INE546B12139,10,LISTING
    t_versionObject::t_datasetLedger::const_iterator companyRecordCrown2Actual =
      vo.getVersionAt(t_versionDate{std::chrono::year(int(2022)), std::chrono::September, std::chrono::day(unsigned(19))});

    unittest::ExpectEqual(bool, true, companyRecordCrown2Actual != vo.getDatasetLedger().cend()); // has dsvo::DataSet<COMPANYMETAINFO_TYPE_LIST>

    unittest::ExpectEqual(dsvo::DataSet<COMPANYMETAINFO_TYPE_LIST>, companyRecordAfterRelistedExpected,
                                                                    companyRecordCrown2Actual->second);


//  ",,,0,5,,0,"
//  28-Oct-2023,+marketLotchange,ANDHRAPAPLTD,ANDHRA PAPER LIMITED,EQ,10,5,INE546B12139,10,LISTING
    t_companyInfo companyInfoNine = converter::ConvertFromString<COMPANYINFO_TYPE_LIST>::ToVal(
      "ANDHRAPAPLTD,ANDHRA PAPER LIMITED,EQ,10,5,INE546B12139,10,LISTING"    );

    dsvo::DataSet<COMPANYMETAINFO_TYPE_LIST> companyRecordNineExpected {lotChgMetaExp, companyInfoNine};

    t_versionObject::t_datasetLedger::const_iterator companyRecordNineActual =
      vo.getVersionAt(t_versionDate{std::chrono::year(int(2023)), std::chrono::October, std::chrono::day(unsigned(28))});

    unittest::ExpectEqual(bool, true, companyRecordNineActual != vo.getDatasetLedger().cend()); // has dsvo::DataSet<COMPANYMETAINFO_TYPE_LIST>

    unittest::ExpectEqual(dsvo::DataSet<COMPANYMETAINFO_TYPE_LIST>, companyRecordNineExpected,
                                                                    companyRecordNineActual->second);

  } catch (const std::exception& ex) {
    std::cout << ex.what() << std::endl;
    rv = 1;
  }

  return rv;
}
