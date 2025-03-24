
#include <testHelper.h>

void interimForwardTest(t_versionObject& vo,
                        bool insertResultExpected,
                        dsvo::VersionedObjectBuilder<t_versionDate, COMPANYINFO_TYPE_LIST>& vob);

void endForwardTest(t_versionObject& vo,
                    bool insertResultExpected,
                    dsvo::VersionedObjectBuilder<t_versionDate, COMPANYINFO_TYPE_LIST>& vob);

void loadVO(t_versionObject& vo,
            bool insertResultExpected)
{
  dsvo::VersionedObjectBuilder<t_versionDate, COMPANYINFO_TYPE_LIST> vob;

  bool insertResult;


/* these rows are from symbolchange.csv
LATEST-CompanyName, OLD-SYMBOL, NEW-SYMBOL, CHANGE-DATE     :: Note: NEW need not correspond to LATEST, it's NEW on that change-date
ANDHRA PAPER LIMITED,APPAPER,IPAPPM,21-JAN-2014
ANDHRA PAPER LIMITED,IPAPPM,ANDPAPER,22-JAN-2020
ANDHRA PAPER LIMITED,ANDPAPER,ANDHRAPAP,05-MAR-2020

this is what VOB model expects
NEW-CompanyName, OLD-SYMBOL, NEW-SYMBOL, CHANGE-DATE     :: Note: NEW need not correspond to LATEST, it's NEW on that change-date
International Paper APPM Limited,APPAPER,IPAPPM,21-JAN-2014
ANDHRA PAPER LIMITED,IPAPPM,ANDPAPER,22-JAN-2020
ANDHRA PAPER LIMITED,ANDPAPER,ANDHRAPAP,05-MAR-2020
*/

  const std::array <bool, std::tuple_size_v<t_companyInfo> > symbolChangeFlg = {true, false, false, false, false, false, false, false};
  const std::array <bool, std::tuple_size_v<t_companyInfo> >   nameChangeFlg = {false, true, false, false, false, false, false, false};


  t_companyInfo symChgOldInfo1 = converter::ConvertFromString<COMPANYINFO_TYPE_LIST>::ToVal("APPAPER,,,0,0,,0,");
  t_companyInfo symChgNewInfo1 = converter::ConvertFromString<COMPANYINFO_TYPE_LIST>::ToVal("IPAPPM,,,0,0,,0,");
  dsvo::ChangesInDataSet<COMPANYINFO_TYPE_LIST> symbolChange1 {symbolChangeFlg, symChgOldInfo1, symChgNewInfo1, dsvo::ApplicableChangeDirection::FORWARD};  // DELTA Change
  insertResult = vob.insertDeltaVersion(t_versionDate{std::chrono::year(int(2014)), std::chrono::January, std::chrono::day(unsigned(21))}, symbolChange1);
  unittest::ExpectEqual(bool, true, insertResult);
  // VOB model error on this t_companyInfo companyNameKeyInfo1 = converter::ConvertFromString<COMPANYINFO_TYPE_LIST>::ToVal(",ANDHRA PAPER LIMITED,,0,0,,0,");
  t_companyInfo companyNameKeyInfo1 = converter::ConvertFromString<COMPANYINFO_TYPE_LIST>::ToVal(",International Paper APPM Limited,,0,0,,0,");
  dsvo::SnapshotDataSet<COMPANYINFO_TYPE_LIST> companyNameKeyDataSet1 {nameChangeFlg, companyNameKeyInfo1, dsvo::ApplicableChangeDirection::FORWARD};   // CompanyName as identifier
  insertResult = vob.insertSnapshotVersion(t_versionDate{std::chrono::year(int(2014)), std::chrono::January, std::chrono::day(unsigned(21))}, companyNameKeyDataSet1);
  unittest::ExpectEqual(bool, true, insertResult);

  t_companyInfo symChgOldInfo2 = converter::ConvertFromString<COMPANYINFO_TYPE_LIST>::ToVal("IPAPPM,,,0,0,,0,");
  t_companyInfo symChgNewInfo2 = converter::ConvertFromString<COMPANYINFO_TYPE_LIST>::ToVal("ANDPAPER,,,0,0,,0,");
  dsvo::ChangesInDataSet<COMPANYINFO_TYPE_LIST> symbolChange2 {symbolChangeFlg, symChgOldInfo2, symChgNewInfo2, dsvo::ApplicableChangeDirection::FORWARD};  // DELTA Change
  insertResult = vob.insertDeltaVersion(t_versionDate{std::chrono::year(int(2020)), std::chrono::January, std::chrono::day(unsigned(22))}, symbolChange2);
  unittest::ExpectEqual(bool, true, insertResult);
  t_companyInfo companyNameKeyInfo2 = converter::ConvertFromString<COMPANYINFO_TYPE_LIST>::ToVal(",ANDHRA PAPER LIMITED,,0,0,,0,");
  dsvo::SnapshotDataSet<COMPANYINFO_TYPE_LIST> companyNameKeyDataSet2 {nameChangeFlg, companyNameKeyInfo2, dsvo::ApplicableChangeDirection::FORWARD};   // CompanyName as identifier
  insertResult = vob.insertSnapshotVersion(t_versionDate{std::chrono::year(int(2020)), std::chrono::January, std::chrono::day(unsigned(22))}, companyNameKeyDataSet2);
  unittest::ExpectEqual(bool, true, insertResult);

  t_companyInfo symChgOldInfo3 = converter::ConvertFromString<COMPANYINFO_TYPE_LIST>::ToVal("ANDPAPER,,,0,0,,0,");
  t_companyInfo symChgNewInfo3 = converter::ConvertFromString<COMPANYINFO_TYPE_LIST>::ToVal("ANDHRAPAP,,,0,0,,0,");
  dsvo::ChangesInDataSet<COMPANYINFO_TYPE_LIST> symbolChange3 {symbolChangeFlg, symChgOldInfo3, symChgNewInfo3, dsvo::ApplicableChangeDirection::FORWARD};  // DELTA Change
  t_versionDate reverseLastVersionDate{std::chrono::year(int(2020)), std::chrono::March, std::chrono::day(unsigned(5))};
  insertResult = vob.insertDeltaVersion(reverseLastVersionDate, symbolChange3);
  unittest::ExpectEqual(bool, true, insertResult);
  t_companyInfo companyNameKeyInfo3 = converter::ConvertFromString<COMPANYINFO_TYPE_LIST>::ToVal(",ANDHRA PAPER LIMITED,,0,0,,0,");
  dsvo::SnapshotDataSet<COMPANYINFO_TYPE_LIST> companyNameKeyDataSet3 {nameChangeFlg, companyNameKeyInfo3, dsvo::ApplicableChangeDirection::FORWARD};   // CompanyName as identifier
  insertResult = vob.insertSnapshotVersion(reverseLastVersionDate, companyNameKeyDataSet3);
  unittest::ExpectEqual(bool, true, insertResult);


//  NOTE : the row below is from namechange.csv
//  LATEST-SYMBOL, OLD-CompanyName, NEW-CompanyName, CHANGE-DATE     :: Note: NEW need not correspond to LATEST
//  ANDPAPER,International Paper APPM Limited,ANDHRA PAPER LIMITED,22-JAN-2020
//  NOTE: actual data has ANDHRAPAP instead of ANDPAPER .
//        IMPORTANT: hence it makes sense not to add SnapshotDataSet<...> in 'dbLoader' for symbol
  t_companyInfo namChgOldInfo1 = converter::ConvertFromString<COMPANYINFO_TYPE_LIST>::ToVal(",International Paper APPM Limited,,0,0,,0,");
  t_companyInfo namChgNewInfo1 = converter::ConvertFromString<COMPANYINFO_TYPE_LIST>::ToVal(",ANDHRA PAPER LIMITED,,0,0,,0,");
  dsvo::ChangesInDataSet<COMPANYINFO_TYPE_LIST> nameChange1 {nameChangeFlg, namChgOldInfo1, namChgNewInfo1, dsvo::ApplicableChangeDirection::FORWARD};  // DELTA Change
  insertResult = vob.insertDeltaVersion(t_versionDate{std::chrono::year(int(2020)), std::chrono::January, std::chrono::day(unsigned(22))}, nameChange1);
  unittest::ExpectEqual(bool, false, insertResult);
  t_companyInfo symbolKeyInfo1 = converter::ConvertFromString<COMPANYINFO_TYPE_LIST>::ToVal("ANDPAPER,,,0,0,,0,");
  dsvo::SnapshotDataSet<COMPANYINFO_TYPE_LIST> symbolKeyDataSet1 {symbolChangeFlg, symbolKeyInfo1, dsvo::ApplicableChangeDirection::FORWARD};   // Symbol as identifier
  insertResult = vob.insertSnapshotVersion(t_versionDate{std::chrono::year(int(2020)), std::chrono::January, std::chrono::day(unsigned(22))}, symbolKeyDataSet1);
  unittest::ExpectEqual(bool, false, insertResult);

  t_versionDate  listingDate{std::chrono::year(int(2004)), std::chrono::May, std::chrono::day(unsigned(13))};


//  NOTE : the row below is manually deduced
//       SYMBOL, NAME OF COMPANY, SERIES, DATE OF LISTING, PAID UP VALUE, MARKET LOT, ISIN NUMBER, FACE VALUE
//       APPAPER,International Paper APPM Limited,EQ,13-MAY-2004,10,1,INE435A01028,10
  t_companyInfo companyInfoStart = converter::ConvertFromString<COMPANYINFO_TYPE_LIST>::ToVal(
    "APPAPER,International Paper APPM Limited,EQ,10,1,INE435A01028,10,LISTED"    );

  dsvo::DataSet<COMPANYINFO_TYPE_LIST> companyRecordStart {companyInfoStart};
  VERSIONEDOBJECT_DEBUG_MSG( "debug_LOG: vo.insertVersion() -> versionDate: " << listingDate << "; DATASET{" << companyRecordStart.toCSV() << "}");
  insertResult = vo.insertVersion(listingDate, companyRecordStart);
  VERSIONEDOBJECT_DEBUG_MSG( "debug_LOG: vo.insertVersion() insertResult=" << insertResult);
  unittest::ExpectEqual(bool, insertResultExpected, insertResult);


//  NOTE : the row below is not a versioned information, but info from EQUITY_L.csv
//       SYMBOL, NAME OF COMPANY, SERIES, DATE OF LISTING, PAID UP VALUE, MARKET LOT, ISIN NUMBER, FACE VALUE
//       ANDHRAPAP,ANDHRA PAPER LIMITED,EQ,13-MAY-2004,10,1,INE435A01028,10
  t_companyInfo companyInfoLatest = converter::ConvertFromString<COMPANYINFO_TYPE_LIST>::ToVal(
    "ANDHRAPAP,ANDHRA PAPER LIMITED,EQ,10,1,INE435A01028,10,LISTED"    );

  dsvo::DataSet<COMPANYINFO_TYPE_LIST> companyRecordLatestExpected {companyInfoLatest};
////////////    NOT used here :         for Forward-build, start-info is deduced using reverseLastVersionDate
//  VERSIONEDOBJECT_DEBUG_MSG( "debug_LOG: vo.insertVersion() -> versionDate: " << reverseLastVersionDate << "; DATASET{" << companyRecordLatestExpected.toCSV() << "}");
//  insertResult = vo.insertVersion(reverseLastVersionDate, companyRecordLatestExpected);
//  VERSIONEDOBJECT_DEBUG_MSG( "debug_LOG: vo.insertVersion() insertResult=" << insertResult);
//  unittest::ExpectEqual(bool, insertResultExpected, insertResult);


  interimForwardTest(vo, insertResultExpected, vob);


  //#################### check all versioned objects

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
    "IPAPPM,International Paper APPM Limited,EQ,10,1,INE435A01028,10,LISTED"    );

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
    "ANDPAPER,ANDHRA PAPER LIMITED,EQ,10,1,INE435A01028,10,LISTED"    );

  dsvo::DataSet<COMPANYINFO_TYPE_LIST> companyRecordThirdExpected {companyInfoThird};

  t_versionObject::t_datasetLedger::const_iterator companyRecordThirdActual =
    vo.getVersionAt(t_versionDate{std::chrono::year(int(2020)), std::chrono::January, std::chrono::day(unsigned(22))});

  unittest::ExpectEqual(bool, true, companyRecordThirdActual != vo.getDatasetLedger().cend()); // has dsvo::DataSet<COMPANYINFO_TYPE_LIST>

  unittest::ExpectEqual(dsvo::DataSet<COMPANYINFO_TYPE_LIST>, companyRecordThirdExpected,
                                                              companyRecordThirdActual->second);


  t_versionObject::t_datasetLedger::const_iterator companyRecordLatestActual =
    vo.getVersionAt(t_versionDate{std::chrono::year(int(2020)), std::chrono::March, std::chrono::day(unsigned(5))});

  unittest::ExpectEqual(bool, true, companyRecordLatestActual != vo.getDatasetLedger().cend()); // has dsvo::DataSet<COMPANYINFO_TYPE_LIST>

  unittest::ExpectEqual(dsvo::DataSet<COMPANYINFO_TYPE_LIST>, companyRecordLatestExpected,
                                                              companyRecordLatestActual->second);


//  ",,,0,2,,0,"
//  ANDHRAPAP,ANDHRA PAPER LIMITED,EQ,10,2,INE435A01028,10
  t_companyInfo companyInfoFifth = converter::ConvertFromString<COMPANYINFO_TYPE_LIST>::ToVal(
    "ANDHRAPAP,ANDHRA PAPER LIMITED,EQ,10,2,INE435A01028,10,LISTED"    );

  dsvo::DataSet<COMPANYINFO_TYPE_LIST> companyRecordFifthExpected {companyInfoFifth};

  t_versionObject::t_datasetLedger::const_iterator companyRecordFifthActual =
    vo.getVersionAt(t_versionDate{std::chrono::year(int(2021)), std::chrono::April, std::chrono::day(unsigned(07))});

  unittest::ExpectEqual(bool, true, companyRecordFifthActual != vo.getDatasetLedger().cend()); // has dsvo::DataSet<COMPANYINFO_TYPE_LIST>

  unittest::ExpectEqual(dsvo::DataSet<COMPANYINFO_TYPE_LIST>, companyRecordFifthExpected,
                                                              companyRecordFifthActual->second);


  endForwardTest(vo, insertResultExpected, vob);
}

