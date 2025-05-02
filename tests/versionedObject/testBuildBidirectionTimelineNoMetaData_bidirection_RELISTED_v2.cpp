// test Snapshot changes : marketlot + Delisted + Relisted
// compare with testBuildBidirectionTimelineNoMetaData_bidirection_RELISTED_v1.cpp

#include <testHelper.h>

void interimReverseTest(t_versionObject& vo,
                        [[maybe_unused]] bool insertResultExpected,
                        t_versionObjectBuilder& vob,
                        const t_versionDate&  listingDate)
{
  {
    ////   SNAPSHOT change test  : applicable for 'buildForwardTimeline'  ( NOT for buildReverseTimeline )
    const std::array <bool, std::tuple_size_v<t_companyInfo> > lotChangeFlg = {false, false, false, false, true, false, false, false};

    t_companyInfo lotChgInfo = t_convertFromString::ToVal(",,,0,2,,0,");
    TEST_WITH_METADATA(dsvo::MetaDataSource lotChgMeta("marketLotSpot" COMMA t_eDataBuild::FORWARD COMMA t_eDataPatch::SNAPSHOT));
    t_snapshotDataSet lotChange {lotChangeFlg, lotChgInfo,
                                 TEST_ALTERNATE_METADATA(lotChgMeta, t_eDataBuild::FORWARD)};   // SNAPSHOT Change
    bool insertResult2 = vob.insertSnapshotVersion(t_versionDate{std::chrono::year(int(2021)), std::chrono::April, std::chrono::day(unsigned(07))}, lotChange);
    unittest::ExpectEqual(bool, true, insertResult2);
  }

  {
    ////////////////////////              DELISTED change test
    const std::array <bool, std::tuple_size_v<t_companyInfo> > delistedChangeFlg = {false, false, false, false, false, false, false, true};

    t_companyInfo delistedChgInfo = t_convertFromString::ToVal(",,,0,0,,0,DELISTED");
    TEST_WITH_METADATA(dsvo::MetaDataSource delistedChgMeta("delistedSpot" COMMA t_eDataBuild::FORWARD COMMA t_eDataPatch::SNAPSHOT));
    t_snapshotDataSet delistedChange {delistedChangeFlg, delistedChgInfo,
                                      TEST_ALTERNATE_METADATA(delistedChgMeta, t_eDataBuild::FORWARD)};   // DELISTED Change
    bool insertResult = vob.insertSnapshotVersion(t_versionDate{std::chrono::year(int(2021)), std::chrono::December, std::chrono::day(unsigned(17))}, delistedChange);
    unittest::ExpectEqual(bool, true, insertResult);
  }

  {
    ////////////////////////              RELISTED change & symbolchange test
    //startDates.push_back(t_versionDate{std::chrono::year(int(2022)), std::chrono::January, std::chrono::day(unsigned(12))});
    const std::array <bool, std::tuple_size_v<t_companyInfo> > symbolChangeFlg = {true, false, false, false, false, false, false, false};

    t_companyInfo symChgOldInfo4 = t_convertFromString::ToVal("ANDHRAPAPER,,,0,0,,0,");
    t_companyInfo symChgNewInfo4 = t_convertFromString::ToVal("ANDHRAPAPLTD,,,0,0,,0,");
    TEST_WITH_METADATA(dsvo::MetaDataSource relistedChgMeta("relistedChange" COMMA t_eDataBuild::REVERSE COMMA t_eDataPatch::DELTACHANGE));
    t_changesInDataSet symbolChange4 {symbolChangeFlg, symChgOldInfo4, symChgNewInfo4,
                                      TEST_ALTERNATE_METADATA(relistedChgMeta, t_eDataBuild::REVERSE)};  // DELTA Change
    bool insertResult = vob.insertDeltaVersion(t_versionDate{std::chrono::year(int(2022)), std::chrono::September, std::chrono::day(unsigned(19))}, symbolChange4);
    unittest::ExpectEqual(bool, true, insertResult);
  }

  {
    t_companyInfo companyInfoAfterRelisting = t_convertFromString::ToVal(
      "ANDHRAPAPLTD,ANDHRA PAPER LIMITED,EQ,10,1,INE546B12139,10,LISTED"    );

    TEST_WITH_METADATA(dsvo::MetaDataSource latestMeta2("EQUITY_L" COMMA t_eDataBuild::IsRECORD COMMA t_eDataPatch::UseRECORD));
    t_dataSet companyRecordAfterRelistedExpected {TEST_WITH_METADATA(latestMeta2 COMMA) companyInfoAfterRelisting};
    t_versionDate afterRelistedVersionDate{std::chrono::year(int(2022)), std::chrono::September, std::chrono::day(unsigned(19))};
    VERSIONEDOBJECT_DEBUG_MSG( "debug_LOG: vo.insertVersion() -> afterRelisted-next-versionDate: " << afterRelistedVersionDate << "; DATASET{" << companyRecordAfterRelistedExpected.toCSV() << "}");
    bool insertResult = vo.insertVersion(afterRelistedVersionDate, companyRecordAfterRelistedExpected);
    VERSIONEDOBJECT_DEBUG_MSG( "debug_LOG: vo.insertVersion() afterRelisted-insertResult=" << insertResult);
    unittest::ExpectEqual(bool, insertResultExpected, insertResult);
  }

  {
    const std::array <bool, std::tuple_size_v<t_companyInfo> > lotChangeFlg = {false, false, false, false, true, false, false, false};

    ////////////////////////              SNAPSHOT change test
    t_companyInfo lotChgInfo2 = t_convertFromString::ToVal(",,,0,5,,0,");
    TEST_WITH_METADATA(dsvo::MetaDataSource lotChgMeta("marketLotSpot" COMMA t_eDataBuild::FORWARD COMMA t_eDataPatch::SNAPSHOT));
    t_snapshotDataSet lotChange2 {lotChangeFlg, lotChgInfo2,
                                  TEST_ALTERNATE_METADATA(lotChgMeta, t_eDataBuild::FORWARD)};   // SNAPSHOT Change
    bool insertResult = vob.insertSnapshotVersion(t_versionDate{std::chrono::year(int(2023)), std::chrono::October, std::chrono::day(unsigned(28))}, lotChange2);
    unittest::ExpectEqual(bool, true, insertResult);
  }

  std::vector<typename t_versionObjectBuilder::t_versionDate> startDates{};
  startDates.push_back(listingDate);
  startDates.push_back(t_versionDate{std::chrono::year(int(2022)), std::chrono::January, std::chrono::day(unsigned(12))});

  auto buildResult = vob.buildBiDirectionalTimeline( startDates, vo);
  unittest::ExpectEqual(typename t_versionObjectBuilder::t_deltaEntriesMap_iter_diff_type, 7, buildResult.first); // 7 calls to insertDeltaVersion()/vob.insertSnapshotVersion()
  unittest::ExpectEqual(typename t_versionObjectBuilder::t_deltaEntriesMap_iter_diff_type, 0, buildResult.second);


  //std::cout << "#### vo start ######\n" << vo.toCSV() << "#### vo end ######\n";
  std::string voStrBidirection =
    "13-May-2004," TEST_WITH_METADATA("-#%symbolChange#@nameSpot,") "APPAPER,International Paper APPM Limited,EQ,10,1,INE435A01028,10,LISTED\n"   // listingDate-of-reverse
    "21-Jan-2014," TEST_WITH_METADATA("-#%symbolChange#%nameChange,") "IPAPPM,International Paper APPM Limited,EQ,10,1,INE435A01028,10,LISTED\n"  // REVERSE
    "22-Jan-2020," TEST_WITH_METADATA("-#%symbolChange#@nameSpot,") "ANDPAPER,ANDHRA PAPER LIMITED,EQ,10,1,INE435A01028,10,LISTED\n"              // REVERSE
    "05-Mar-2020," TEST_WITH_METADATA("^#^EQUITY_L,") "ANDHRAPAP,ANDHRA PAPER LIMITED,EQ,10,1,INE435A01028,10,LISTED\n"                           // REVERSE + vo.insertVersion(...)
    "07-Apr-2021," TEST_WITH_METADATA("+#@marketLotSpot,") "ANDHRAPAP,ANDHRA PAPER LIMITED,EQ,10,2,INE435A01028,10,LISTED\n"                      // FORWARD
    "17-Dec-2021," TEST_WITH_METADATA("+#@delistedSpot,") "ANDHRAPAP,ANDHRA PAPER LIMITED,EQ,10,2,INE435A01028,10,DELISTED\n"                     // FORWARD
    "12-Jan-2022," TEST_WITH_METADATA("-#%relistedChange,") "ANDHRAPAPER,ANDHRA PAPER LIMITED,EQ,10,1,INE546B12139,10,LISTED\n"                   // listingDate-2-of-reverse
    "19-Sep-2022," TEST_WITH_METADATA("^#^EQUITY_L,") "ANDHRAPAPLTD,ANDHRA PAPER LIMITED,EQ,10,1,INE546B12139,10,LISTED\n"                        // REVERSE + vo.insertVersion(...)
    "28-Oct-2023," TEST_WITH_METADATA("+#@marketLotSpot,") "ANDHRAPAPLTD,ANDHRA PAPER LIMITED,EQ,10,5,INE546B12139,10,LISTED\n";                  // FORWARD

  unittest::ExpectEqual(std::string, voStrBidirection, vo.toCSV());
}


void endReverseTest(                 t_versionObject& vo,
                    [[maybe_unused]] bool insertResultExpected,
                    [[maybe_unused]] t_versionObjectBuilder& vob,
                    [[maybe_unused]] const t_versionDate&  listingDate)
{
//  ",,,0,2,,0,"
//  ANDHRAPAP,ANDHRA PAPER LIMITED,EQ,10,2,INE435A01028,10
  t_companyInfo companyInfoFifth = t_convertFromString::ToVal(
    "ANDHRAPAP,ANDHRA PAPER LIMITED,EQ,10,2,INE435A01028,10,LISTED"    );

  TEST_WITH_METADATA(dsvo::MetaDataSource lotChgMetaExp("marketLotChange" COMMA t_eDataBuild::FORWARD COMMA t_eDataPatch::SNAPSHOT));
  t_dataSet companyRecordFifthExpected {TEST_WITH_METADATA(lotChgMetaExp COMMA) companyInfoFifth};

  t_versionObject::t_datasetLedger::const_iterator companyRecordFifthActual =
    vo.getVersionAt(t_versionDate{std::chrono::year(int(2021)), std::chrono::April, std::chrono::day(unsigned(07))});

  unittest::ExpectEqual(bool, true, companyRecordFifthActual != vo.getDatasetLedger().cend()); // t_dataSet

  unittest::ExpectEqual(t_dataSet, companyRecordFifthExpected,
                                   companyRecordFifthActual->second);


//  ,,,0,0,,0,DELISTED
//  17-Dec-2021,ANDHRAPAP,ANDHRA PAPER LIMITED,EQ,10,2,INE435A01028,10,DELISTED
  t_companyInfo companyInfoSixth = t_convertFromString::ToVal(
    "ANDHRAPAP,ANDHRA PAPER LIMITED,EQ,10,2,INE435A01028,10,DELISTED"    );

  TEST_WITH_METADATA(dsvo::MetaDataSource delistedChgMeta("delistedSpot" COMMA t_eDataBuild::FORWARD COMMA t_eDataPatch::SNAPSHOT));
  t_dataSet companyRecordSixthExpected {TEST_WITH_METADATA(delistedChgMeta COMMA) companyInfoSixth};

  t_versionObject::t_datasetLedger::const_iterator companyRecordSixthActual =
    vo.getVersionAt(t_versionDate{std::chrono::year(int(2021)), std::chrono::December, std::chrono::day(unsigned(17))});

  unittest::ExpectEqual(bool, true, companyRecordSixthActual != vo.getDatasetLedger().cend()); // has t_dataSet

  unittest::ExpectEqual(t_dataSet, companyRecordSixthExpected,
                                   companyRecordSixthActual->second);


//  ANDHRA PAPER LIMITED,ANDHRAPAPER,ANDHRAPAPLTD,12-JAN-2022  {only for RELISTED_2}
//  NOTE: the row below is not a versioned information, but info from EQUITY_L.csv
//  12-Jan-2022,ANDHRAPAPER,ANDHRA PAPER LIMITED,EQ,10,1,INE546B12139,10,LISTED
  t_companyInfo companyInfoRelisted = t_convertFromString::ToVal(
    "ANDHRAPAPER,ANDHRA PAPER LIMITED,EQ,10,1,INE546B12139,10,LISTED"    );

  TEST_WITH_METADATA(dsvo::MetaDataSource relistedChgMeta("relistedChange" COMMA t_eDataBuild::REVERSE COMMA t_eDataPatch::DELTACHANGE));
  t_dataSet companyRecordRelistedExpected {TEST_WITH_METADATA(relistedChgMeta COMMA) companyInfoRelisted};

  t_versionObject::t_datasetLedger::const_iterator companyRecordRelistedActual =
    vo.getVersionAt(t_versionDate(std::chrono::year(int(2022)), std::chrono::January, std::chrono::day(unsigned(12))));

  unittest::ExpectEqual(bool, true, companyRecordRelistedActual != vo.getDatasetLedger().cend()); // has t_dataSet

  unittest::ExpectEqual(t_dataSet, companyRecordRelistedExpected,
                                   companyRecordRelistedActual->second);



//  NOTE: the row below is not a versioned information, but info from EQUITY_L.csv
//  19-Sep-2022,EQUITY_L,ANDHRAPAPLTD,ANDHRA PAPER LIMITED,EQ,10,1,INE546B12139,10,LISTED
  t_companyInfo companyInfoAfterRelisting = t_convertFromString::ToVal(
    "ANDHRAPAPLTD,ANDHRA PAPER LIMITED,EQ,10,1,INE546B12139,10,LISTED"    );

  TEST_WITH_METADATA(dsvo::MetaDataSource latestMeta2("EQUITY_L" COMMA t_eDataBuild::IsRECORD COMMA t_eDataPatch::UseRECORD));
  t_dataSet companyRecordAfterRelistedExpected {TEST_WITH_METADATA(latestMeta2 COMMA) companyInfoAfterRelisting};

  t_versionObject::t_datasetLedger::const_iterator companyRecordCrown2Actual =
    vo.getVersionAt(t_versionDate{std::chrono::year(int(2022)), std::chrono::September, std::chrono::day(unsigned(19))});

  unittest::ExpectEqual(bool, true, companyRecordCrown2Actual != vo.getDatasetLedger().cend()); // has t_dataSet

  unittest::ExpectEqual(t_dataSet, companyRecordAfterRelistedExpected,
                                   companyRecordCrown2Actual->second);


//  ",,,0,5,,0,"
//  28-Oct-2023,ANDHRAPAPLTD,ANDHRA PAPER LIMITED,EQ,10,5,INE546B12139,10,LISTED
  t_companyInfo companyInfoNine = t_convertFromString::ToVal(
    "ANDHRAPAPLTD,ANDHRA PAPER LIMITED,EQ,10,5,INE546B12139,10,LISTED"    );

//TEST_WITH_METADATA(dsvo::MetaDataSource lotChgMetaExp("marketLotChange" COMMA t_eDataBuild::FORWARD COMMA t_eDataPatch::SNAPSHOT));
  t_dataSet companyRecordNineExpected {TEST_WITH_METADATA(lotChgMetaExp COMMA) companyInfoNine};

  t_versionObject::t_datasetLedger::const_iterator companyRecordNineActual =
    vo.getVersionAt(t_versionDate{std::chrono::year(int(2023)), std::chrono::October, std::chrono::day(unsigned(28))});

  unittest::ExpectEqual(bool, true, companyRecordNineActual != vo.getDatasetLedger().cend()); // has t_dataSet

  unittest::ExpectEqual(t_dataSet, companyRecordNineExpected,
                                   companyRecordNineActual->second);

}
