
#include <testHelper.h>

void interimForwardTest(t_versionObject& vo,
                        bool insertResultExpected,
                        t_versionObjectBuilder& vob);

void endForwardTest(t_versionObject& vo,
                    bool insertResultExpected,
                    t_versionObjectBuilder& vob);

void loadVO(t_versionObject& vo,
            bool insertResultExpected)
{
  t_versionObjectBuilder vob;

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

  TEST_WITH_METADATA(dsvo::MetaDataSource symChgMeta("symbolChange" COMMA t_eDataBuild::FORWARD COMMA t_eDataPatch::DELTACHANGE));
  TEST_WITH_METADATA(dsvo::MetaDataSource namChgMeta("nameChange"   COMMA t_eDataBuild::FORWARD COMMA t_eDataPatch::DELTACHANGE));


  TEST_WITH_METADATA(dsvo::MetaDataSource symSpotMeta("symbolSpot" COMMA t_eDataBuild::FORWARD COMMA t_eDataPatch::SNAPSHOT));
  TEST_WITH_METADATA(dsvo::MetaDataSource namSpotMeta("nameSpot"   COMMA t_eDataBuild::FORWARD COMMA t_eDataPatch::SNAPSHOT));

  const std::array <bool, std::tuple_size_v<t_companyInfo> > symbolChangeFlg = {true, false, false, false, false, false, false, false};
  const std::array <bool, std::tuple_size_v<t_companyInfo> >   nameChangeFlg = {false, true, false, false, false, false, false, false};


  t_companyInfo symChgOldInfo1 = t_convertFromString::ToVal("APPAPER,,,0,0,,0,");
  t_companyInfo symChgNewInfo1 = t_convertFromString::ToVal("IPAPPM,,,0,0,,0,");
  t_changesInDataSet symbolChange1 {TEST_WITH_METADATA(symChgMeta COMMA) symbolChangeFlg, symChgOldInfo1, symChgNewInfo1, dsvo::ApplicableChangeDirection::FORWARD};  // DELTA Change
  insertResult = vob.insertDeltaVersion(t_versionDate{std::chrono::year(int(2014)), std::chrono::January, std::chrono::day(unsigned(21))}, symbolChange1);
  unittest::ExpectEqual(bool, true, insertResult);
  // VOB model error on this t_companyInfo companyNameKeyInfo1 = t_convertFromString::ToVal(",ANDHRA PAPER LIMITED,,0,0,,0,");
  t_companyInfo companyNameKeyInfo1 = t_convertFromString::ToVal(",International Paper APPM Limited,,0,0,,0,");
  t_snapshotDataSet companyNameKeyDataSet1 {TEST_WITH_METADATA(namSpotMeta COMMA) nameChangeFlg, companyNameKeyInfo1, dsvo::ApplicableChangeDirection::FORWARD};   // CompanyName as identifier
  insertResult = vob.insertSnapshotVersion(t_versionDate{std::chrono::year(int(2014)), std::chrono::January, std::chrono::day(unsigned(21))}, companyNameKeyDataSet1);
  unittest::ExpectEqual(bool, true, insertResult);

  t_companyInfo symChgOldInfo2 = t_convertFromString::ToVal("IPAPPM,,,0,0,,0,");
  t_companyInfo symChgNewInfo2 = t_convertFromString::ToVal("ANDPAPER,,,0,0,,0,");
  t_changesInDataSet symbolChange2 {TEST_WITH_METADATA(symChgMeta COMMA) symbolChangeFlg, symChgOldInfo2, symChgNewInfo2, dsvo::ApplicableChangeDirection::FORWARD};  // DELTA Change
  insertResult = vob.insertDeltaVersion(t_versionDate{std::chrono::year(int(2020)), std::chrono::January, std::chrono::day(unsigned(22))}, symbolChange2);
  unittest::ExpectEqual(bool, true, insertResult);
  t_companyInfo companyNameKeyInfo2 = t_convertFromString::ToVal(",ANDHRA PAPER LIMITED,,0,0,,0,");
  t_snapshotDataSet companyNameKeyDataSet2 {TEST_WITH_METADATA(namSpotMeta COMMA) nameChangeFlg, companyNameKeyInfo2, dsvo::ApplicableChangeDirection::FORWARD};   // CompanyName as identifier
  insertResult = vob.insertSnapshotVersion(t_versionDate{std::chrono::year(int(2020)), std::chrono::January, std::chrono::day(unsigned(22))}, companyNameKeyDataSet2);
  unittest::ExpectEqual(bool, true, insertResult);

  t_companyInfo symChgOldInfo3 = t_convertFromString::ToVal("ANDPAPER,,,0,0,,0,");
  t_companyInfo symChgNewInfo3 = t_convertFromString::ToVal("ANDHRAPAP,,,0,0,,0,");
  t_changesInDataSet symbolChange3 {TEST_WITH_METADATA(symChgMeta COMMA) symbolChangeFlg, symChgOldInfo3, symChgNewInfo3, dsvo::ApplicableChangeDirection::FORWARD};  // DELTA Change
  t_versionDate reverseLastVersionDate{std::chrono::year(int(2020)), std::chrono::March, std::chrono::day(unsigned(5))};
  insertResult = vob.insertDeltaVersion(reverseLastVersionDate, symbolChange3);
  unittest::ExpectEqual(bool, true, insertResult);
  t_companyInfo companyNameKeyInfo3 = t_convertFromString::ToVal(",ANDHRA PAPER LIMITED,,0,0,,0,");
  t_snapshotDataSet companyNameKeyDataSet3 {TEST_WITH_METADATA(namSpotMeta COMMA) nameChangeFlg, companyNameKeyInfo3, dsvo::ApplicableChangeDirection::FORWARD};   // CompanyName as identifier
  insertResult = vob.insertSnapshotVersion(reverseLastVersionDate, companyNameKeyDataSet3);
  unittest::ExpectEqual(bool, true, insertResult);


//  NOTE : the row below is from namechange.csv
//  LATEST-SYMBOL, OLD-CompanyName, NEW-CompanyName, CHANGE-DATE     :: Note: NEW need not correspond to LATEST
//  ANDPAPER,International Paper APPM Limited,ANDHRA PAPER LIMITED,22-JAN-2020
//  NOTE: actual data has ANDHRAPAP instead of ANDPAPER .
//        IMPORTANT: hence it makes sense not to add SnapshotDataSet<...> in 'dbLoader' for symbol
  t_companyInfo namChgOldInfo1 = t_convertFromString::ToVal(",International Paper APPM Limited,,0,0,,0,");
  t_companyInfo namChgNewInfo1 = t_convertFromString::ToVal(",ANDHRA PAPER LIMITED,,0,0,,0,");
  t_changesInDataSet nameChange1 {TEST_WITH_METADATA(namChgMeta COMMA) nameChangeFlg, namChgOldInfo1, namChgNewInfo1, dsvo::ApplicableChangeDirection::FORWARD};  // DELTA Change
  insertResult = vob.insertDeltaVersion(t_versionDate{std::chrono::year(int(2020)), std::chrono::January, std::chrono::day(unsigned(22))}, nameChange1);
  unittest::ExpectEqual(bool, false, insertResult);
  t_companyInfo symbolKeyInfo1 = t_convertFromString::ToVal("ANDPAPER,,,0,0,,0,");
  t_snapshotDataSet symbolKeyDataSet1 {TEST_WITH_METADATA(symSpotMeta COMMA) symbolChangeFlg, symbolKeyInfo1, dsvo::ApplicableChangeDirection::FORWARD};   // Symbol as identifier
  insertResult = vob.insertSnapshotVersion(t_versionDate{std::chrono::year(int(2020)), std::chrono::January, std::chrono::day(unsigned(22))}, symbolKeyDataSet1);
  unittest::ExpectEqual(bool, false, insertResult);

  t_versionDate  listingDate{std::chrono::year(int(2004)), std::chrono::May, std::chrono::day(unsigned(13))};


//  NOTE : the row below is manually deduced
//       SYMBOL, NAME OF COMPANY, SERIES, DATE OF LISTING, PAID UP VALUE, MARKET LOT, ISIN NUMBER, FACE VALUE
//       APPAPER,International Paper APPM Limited,EQ,13-MAY-2004,10,1,INE435A01028,10
  t_companyInfo companyInfoStart = t_convertFromString::ToVal(
    "APPAPER,International Paper APPM Limited,EQ,10,1,INE435A01028,10,LISTED"    );

  TEST_WITH_METADATA(dsvo::MetaDataSource manualMeta("manualDeduction" COMMA t_eDataBuild::RECORDb COMMA t_eDataPatch::RECORDp));
  t_dataSet companyRecordStart {TEST_WITH_METADATA(manualMeta COMMA) companyInfoStart};
  VERSIONEDOBJECT_DEBUG_MSG( "debug_LOG: vo.insertVersion() -> listingDate-versionDate: " << listingDate << "; DATASET{" << companyRecordStart.toCSV() << "}");
  insertResult = vo.insertVersion(listingDate, companyRecordStart);
  VERSIONEDOBJECT_DEBUG_MSG( "debug_LOG: vo.insertVersion() insertResult=" << insertResult);
  unittest::ExpectEqual(bool, insertResultExpected, insertResult);


//  NOTE : the row below is not a versioned information, but info from EQUITY_L.csv
//       SYMBOL, NAME OF COMPANY, SERIES, DATE OF LISTING, PAID UP VALUE, MARKET LOT, ISIN NUMBER, FACE VALUE
//       ANDHRAPAP,ANDHRA PAPER LIMITED,EQ,13-MAY-2004,10,1,INE435A01028,10
  t_companyInfo companyInfoLatest = t_convertFromString::ToVal(
    "ANDHRAPAP,ANDHRA PAPER LIMITED,EQ,10,1,INE435A01028,10,LISTED"    );

  TEST_WITH_METADATA(dsvo::MetaDataSource symChgMetaExp("symbolChange" COMMA t_eDataBuild::FORWARD COMMA t_eDataPatch::DELTACHANGE));
  t_dataSet companyRecordFourthExpected {TEST_WITH_METADATA(symChgMetaExp COMMA) companyInfoLatest};
////////////    NOT used here :         for Forward-build, start-info is deduced using reverseLastVersionDate
//  VERSIONEDOBJECT_DEBUG_MSG( "debug_LOG: vo.insertVersion() -> versionDate: " << reverseLastVersionDate << "; DATASET{" << companyRecordFourthExpected.toCSV() << "}");
//  insertResult = vo.insertVersion(reverseLastVersionDate, companyRecordFourthExpected);
//  VERSIONEDOBJECT_DEBUG_MSG( "debug_LOG: vo.insertVersion() insertResult=" << insertResult);
//  unittest::ExpectEqual(bool, insertResultExpected, insertResult);


  interimForwardTest(vo, insertResultExpected, vob);


  //#################### check all versioned objects
  t_versionObject::t_datasetLedger::const_iterator companyRecordFirstActual =
              vo.getVersionAt(listingDate);

  unittest::ExpectEqual(bool, true, companyRecordFirstActual != vo.getDatasetLedger().cend()); // has t_dataSet
  unittest::ExpectEqual(t_dataSet, companyRecordStart,
                                   companyRecordFirstActual->second);
  unittest::ExpectEqual(t_versionDate, listingDate,
                                       companyRecordFirstActual->first);
  ////////
  t_versionDate  prelistingDate{std::chrono::year(int(2004)), std::chrono::May, std::chrono::day(unsigned(12))};
  t_versionObject::t_datasetLedger::const_iterator companyRecordPreFirstActual =
              vo.getVersionAt(prelistingDate);
  unittest::ExpectEqual(bool, true, companyRecordPreFirstActual == vo.getDatasetLedger().cend()); // no record t_dataSet
  ////////
  t_versionDate  postlistingDate{std::chrono::year(int(2004)), std::chrono::May, std::chrono::day(unsigned(14))};
  t_versionObject::t_datasetLedger::const_iterator companyRecordPostFirstActual =
              vo.getVersionAt(postlistingDate);
  unittest::ExpectEqual(bool, true, companyRecordPostFirstActual != vo.getDatasetLedger().cend()); // has record t_dataSet
  unittest::ExpectEqual(t_dataSet, companyRecordStart,
                                   companyRecordPostFirstActual->second);
  unittest::ExpectEqual(t_versionDate, listingDate,
                                       companyRecordPostFirstActual->first);


  //  ANDHRA PAPER LIMITED,APPAPER,IPAPPM,21-JAN-2014
  t_companyInfo companyInfoSecond = t_convertFromString::ToVal(
             "IPAPPM,International Paper APPM Limited,EQ,10,1,INE435A01028,10,LISTED"    );

  t_dataSet companyRecordSecondExpected {TEST_WITH_METADATA(symChgMetaExp COMMA) companyInfoSecond};

  t_versionDate secondDate{std::chrono::year(int(2014)), std::chrono::January, std::chrono::day(unsigned(21))};
  t_versionObject::t_datasetLedger::const_iterator companyRecordSecondActual =
              vo.getVersionAt(secondDate);

  unittest::ExpectEqual(bool, true, companyRecordSecondActual != vo.getDatasetLedger().cend()); // has t_dataSet
  unittest::ExpectEqual(t_dataSet, companyRecordSecondExpected,
                                   companyRecordSecondActual->second);
  unittest::ExpectEqual(t_versionDate, secondDate,
                                       companyRecordSecondActual->first);
  ////////
  t_versionDate  preSecondDate{std::chrono::year(int(2014)), std::chrono::January, std::chrono::day(unsigned(20))};
  t_versionObject::t_datasetLedger::const_iterator companyRecordPreSecondActual =
              vo.getVersionAt(preSecondDate);
  unittest::ExpectEqual(bool, true, companyRecordPreSecondActual != vo.getDatasetLedger().cend()); // has record t_dataSet
  unittest::ExpectEqual(t_dataSet, companyRecordStart,
                                   companyRecordPreSecondActual->second);
  unittest::ExpectEqual(t_versionDate, listingDate,
                                       companyRecordPreSecondActual->first);
  ////////
  t_versionDate  postSecondDate{std::chrono::year(int(2014)), std::chrono::January, std::chrono::day(unsigned(22))};
  t_versionObject::t_datasetLedger::const_iterator companyRecordPostSecondActual =
              vo.getVersionAt(postSecondDate);
  unittest::ExpectEqual(bool, true, companyRecordPostFirstActual != vo.getDatasetLedger().cend()); // has record t_dataSet
  unittest::ExpectEqual(t_dataSet, companyRecordSecondExpected,
                                   companyRecordPostSecondActual->second);
  unittest::ExpectEqual(t_versionDate, listingDate,
                                       companyRecordPostFirstActual->first);


  //  ANDHRA PAPER LIMITED,IPAPPM,ANDPAPER,22-JAN-2020
  //  ANDHRAPAP,International Paper APPM Limited,ANDHRA PAPER LIMITED,22-JAN-2020
  t_companyInfo companyInfoThird = t_convertFromString::ToVal(
              "ANDPAPER,ANDHRA PAPER LIMITED,EQ,10,1,INE435A01028,10,LISTED"    );

  TEST_WITH_METADATA(dsvo::MetaDataSource symChgNamChgMetaExp("symbolChange" COMMA t_eDataBuild::FORWARD COMMA t_eDataPatch::DELTACHANGE));
  TEST_WITH_METADATA(symChgNamChgMetaExp.merge(dsvo::MetaDataSource{"nameChange" COMMA t_eDataBuild::FORWARD COMMA t_eDataPatch::DELTACHANGE}));
  t_dataSet companyRecordThirdExpected {TEST_WITH_METADATA(symChgNamChgMetaExp COMMA) companyInfoThird};

  t_versionObject::t_datasetLedger::const_iterator companyRecordThirdActual =
              vo.getVersionAt(t_versionDate{std::chrono::year(int(2020)), std::chrono::January, std::chrono::day(unsigned(22))});

  unittest::ExpectEqual(bool, true, companyRecordThirdActual != vo.getDatasetLedger().cend()); // has t_dataSet

  unittest::ExpectEqual(t_dataSet, companyRecordThirdExpected,
                                   companyRecordThirdActual->second);


  t_versionObject::t_datasetLedger::const_iterator companyRecordFourthActual =
              vo.getVersionAt(t_versionDate{std::chrono::year(int(2020)), std::chrono::March, std::chrono::day(unsigned(5))});

  unittest::ExpectEqual(bool, true, companyRecordFourthActual != vo.getDatasetLedger().cend()); // has t_dataSet

  unittest::ExpectEqual(t_dataSet, companyRecordFourthExpected,
                                   companyRecordFourthActual->second);


//  ",,,0,2,,0,"
//  ANDHRAPAP,ANDHRA PAPER LIMITED,EQ,10,2,INE435A01028,10
  t_companyInfo companyInfoFifth = t_convertFromString::ToVal(
    "ANDHRAPAP,ANDHRA PAPER LIMITED,EQ,10,2,INE435A01028,10,LISTED"    );

  TEST_WITH_METADATA(dsvo::MetaDataSource lotChgMetaExp("marketLotChange" COMMA t_eDataBuild::FORWARD COMMA t_eDataPatch::SNAPSHOT));
  t_dataSet companyRecordFifthExpected {TEST_WITH_METADATA(lotChgMetaExp COMMA) companyInfoFifth};

  t_versionObject::t_datasetLedger::const_iterator companyRecordFifthActual =
    vo.getVersionAt(t_versionDate{std::chrono::year(int(2021)), std::chrono::April, std::chrono::day(unsigned(07))});

  unittest::ExpectEqual(bool, true, companyRecordFifthActual != vo.getDatasetLedger().cend()); // has t_dataSet

  unittest::ExpectEqual(t_dataSet, companyRecordFifthExpected,
                                   companyRecordFifthActual->second);


  endForwardTest(vo, insertResultExpected, vob);
}

