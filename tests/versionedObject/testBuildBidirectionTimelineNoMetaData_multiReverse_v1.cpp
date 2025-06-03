// test Snapshot changes : marketlot + Delisted + Relisted
// compare with testBuildReverseTimelineNoMetaData_ChangeFullList.cpp

#include <testHelper.h>

void interimReverseTest(t_versionObject& vo,
                        [[maybe_unused]] bool insertResultExpected,
                        t_versionObjectBuilder& vob,
                        const t_versionDate&  listingDate)
{
  {
    ////////////////////////              DELISTED change test
    const std::array <bool, std::tuple_size_v<t_companyInfo> > delistedChangeFlg = {false, false, false, false, false, false, false, true};

    TEST_WITH_METADATA(dsvo::MetaDataSource delistedChgMeta("delistedChange" COMMA t_eDataBuild::REVERSE COMMA t_eDataPatch::DELTACHANGE));
    t_companyInfo delistedChgOldInfo = t_convertFromString::ToVal(",,,0,0,,0,LISTED");
    t_companyInfo delistedChgNewInfo = t_convertFromString::ToVal(",,,0,0,,0,DELISTED");
    t_changesInDataSet delistedChange {delistedChangeFlg, delistedChgOldInfo, delistedChgNewInfo,
                                       TEST_ALTERNATE_METADATA(delistedChgMeta, t_eDataBuild::REVERSE)};   // DELISTED Change
    bool insertResult = vob.insertDeltaVersion(t_versionDate{std::chrono::year(int(2021)), std::chrono::December, std::chrono::day(unsigned(17))}, delistedChange);
    unittest::ExpectEqual(bool, true, insertResult);
  }

  {
    ////////////////////////              RELISTED change
    const std::array <bool, std::tuple_size_v<t_companyInfo> > relistedChangeFlg = {true, false, false, false, true, true, false, true};

    TEST_WITH_METADATA(dsvo::MetaDataSource relistedChgMeta("relistedChange" COMMA t_eDataBuild::REVERSE COMMA t_eDataPatch::DELTACHANGE));
    t_companyInfo relistedChgOldInfo = t_convertFromString::ToVal("ANDHRAPAP,,,0,2,INE435A01028,0,DELISTED");
    t_companyInfo relistedChgNewInfo = t_convertFromString::ToVal("ANDHRAPAPER,,,0,1,INE546B12139,0,LISTED");
    t_changesInDataSet relistedChange {relistedChangeFlg, relistedChgOldInfo, relistedChgNewInfo,
                                       TEST_ALTERNATE_METADATA(relistedChgMeta, t_eDataBuild::REVERSE)};   // RELISTED Change
    bool insertResult = vob.insertDeltaVersion(t_versionDate{std::chrono::year(int(2022)), std::chrono::January, std::chrono::day(unsigned(12))}, relistedChange);
    unittest::ExpectEqual(bool, true, insertResult);
  }

  {
    ////////////////////////              symbolchange test
    const std::array <bool, std::tuple_size_v<t_companyInfo> > symbolChangeFlg = {true, false, false, false, false, false, false, false};

    TEST_WITH_METADATA(dsvo::MetaDataSource symbolChgMeta("symbolChange" COMMA t_eDataBuild::REVERSE COMMA t_eDataPatch::DELTACHANGE));
    t_companyInfo symChgOldInfo4 = t_convertFromString::ToVal("ANDHRAPAPER,,,0,0,,0,");
    t_companyInfo symChgNewInfo4 = t_convertFromString::ToVal("ANDHRAPAPLTD,,,0,0,,0,");
    t_changesInDataSet symbolChange4 {symbolChangeFlg, symChgOldInfo4, symChgNewInfo4,
                                      TEST_ALTERNATE_METADATA(symbolChgMeta, t_eDataBuild::REVERSE)};  // DELTA Change
    bool insertResult = vob.insertDeltaVersion(t_versionDate{std::chrono::year(int(2022)), std::chrono::September, std::chrono::day(unsigned(19))}, symbolChange4);
    unittest::ExpectEqual(bool, true, insertResult);
  }

  const dsvo::StreamerHelper sh{};
  {
    t_companyInfo companyInfoAfterRelisting = t_convertFromString::ToVal(
      "ANDHRAPAPLTD,ANDHRA PAPER LIMITED,EQ,10,1,INE546B12139,10,LISTED"    );

    TEST_WITH_METADATA(dsvo::MetaDataSource latestMeta("EQUITY_L" COMMA t_eDataBuild::IsRECORD COMMA t_eDataPatch::FullRECORD));
    t_dataSet companyRecordAfterRelistedExpected {TEST_WITH_METADATA(latestMeta COMMA) companyInfoAfterRelisting};
    t_versionDate afterRelistedVersionDate{std::chrono::year(int(2022)), std::chrono::September, std::chrono::day(unsigned(19))};
    VERSIONEDOBJECT_DEBUG_MSG( "debug_LOG: vo.insertVersion() -> afterRelisted-next-versionDate: " << afterRelistedVersionDate << "; DATASET{" << companyRecordAfterRelistedExpected.toCSV(TEST_WITH_METADATA(sh)) << "}");
    bool insertResult = vo.insertVersion(afterRelistedVersionDate, companyRecordAfterRelistedExpected);
    VERSIONEDOBJECT_DEBUG_MSG( "debug_LOG: vo.insertVersion() afterRelisted-insertResult=" << insertResult);
    unittest::ExpectEqual(bool, insertResultExpected, insertResult);
  }

  std::vector<typename t_versionObjectBuilder::t_versionDate> startDates{};
  startDates.push_back(listingDate);
  startDates.push_back(t_versionDate{std::chrono::year(int(2021)), std::chrono::April, std::chrono::day(unsigned(07))});
  //startDates.push_back(t_versionDate{std::chrono::year(int(2020)), std::chrono::March, std::chrono::day(unsigned(05))});

  auto buildResult = vob.buildBiDirectionalTimeline( startDates, vo);
  unittest::ExpectEqual(typename t_versionObjectBuilder::t_deltaEntriesMap_iter_diff_type, 6, buildResult.first); // 6 calls to insertDeltaVersion()/vob.insertSnapshotVersion()
  unittest::ExpectEqual(typename t_versionObjectBuilder::t_deltaEntriesMap_iter_diff_type, 0, buildResult.second);


  //std::cout << "#### vo start ######\n" << t_versionObjectStream::createVOstreamer(vo).toCSV(TEST_WITH_METADATA(sh)) << "#### vo end ######\n";
  std::string voStrBidirection =
    "13-May-2004," TEST_WITH_METADATA("-|%symbolChange|@nameSpot,") "APPAPER,International Paper APPM Limited,EQ,10,1,INE435A01028,10,LISTED\n"     // listingDate-1-of-reverse
    "21-Jan-2014," TEST_WITH_METADATA("-|%symbolChange|%nameChange,") "IPAPPM,International Paper APPM Limited,EQ,10,1,INE435A01028,10,LISTED\n"    // REVERSE
    "22-Jan-2020," TEST_WITH_METADATA("-|%symbolChange|@nameSpot,") "ANDPAPER,ANDHRA PAPER LIMITED,EQ,10,1,INE435A01028,10,LISTED\n"                // REVERSE
    "05-Mar-2020," TEST_WITH_METADATA("*|*EQUITY_L,") "ANDHRAPAP,ANDHRA PAPER LIMITED,EQ,10,1,INE435A01028,10,LISTED\n"                             // REVERSE + vo.insertVersion(...)
    "07-Apr-2021," TEST_WITH_METADATA("-|%delistedChange,") "ANDHRAPAP,ANDHRA PAPER LIMITED,EQ,10,2,INE435A01028,10,LISTED\n"                       // listingDate-2-of-reverse
    "17-Dec-2021," TEST_WITH_METADATA("-|%relistedChange,") "ANDHRAPAP,ANDHRA PAPER LIMITED,EQ,10,2,INE435A01028,10,DELISTED\n"                     // REVERSE
    "12-Jan-2022," TEST_WITH_METADATA("-|%symbolChange,") "ANDHRAPAPER,ANDHRA PAPER LIMITED,EQ,10,1,INE546B12139,10,LISTED\n"                       // REVERSE
    "19-Sep-2022," TEST_WITH_METADATA("*|*EQUITY_L,") "ANDHRAPAPLTD,ANDHRA PAPER LIMITED,EQ,10,1,INE546B12139,10,LISTED\n";                         // REVERSE + vo.insertVersion(...)

  unittest::ExpectEqual(std::string, voStrBidirection, t_versionObjectStream::createVOstreamer(vo).toCSV(TEST_WITH_METADATA(sh)));
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

  TEST_WITH_METADATA(dsvo::MetaDataSource delisted_lotChgMetaExp("delistedChange" COMMA t_eDataBuild::REVERSE COMMA t_eDataPatch::DELTACHANGE));
  t_dataSet companyRecordFifthExpected {TEST_WITH_METADATA(delisted_lotChgMetaExp COMMA) companyInfoFifth};

  t_versionObject::t_datasetLedger::const_iterator companyRecordFifthActual =
    vo.getVersionAt(t_versionDate{std::chrono::year(int(2021)), std::chrono::April, std::chrono::day(unsigned(07))});

  unittest::ExpectEqual(bool, true, companyRecordFifthActual != vo.getDatasetLedger().cend()); // has t_dataSet

  unittest::ExpectEqual(t_dataSet, companyRecordFifthExpected,
                                   companyRecordFifthActual->second);


//  ,,,0,0,,0,DELISTED
//  17-Dec-2021,ANDHRAPAP,ANDHRA PAPER LIMITED,EQ,10,2,INE435A01028,10,DELISTED
  t_companyInfo companyInfoSixth = t_convertFromString::ToVal(
    "ANDHRAPAP,ANDHRA PAPER LIMITED,EQ,10,2,INE435A01028,10,DELISTED"    );

  TEST_WITH_METADATA(dsvo::MetaDataSource relistedChgMetaExp("relistedChange" COMMA t_eDataBuild::REVERSE COMMA t_eDataPatch::DELTACHANGE));
  t_dataSet companyRecordSixthExpected {TEST_WITH_METADATA(relistedChgMetaExp COMMA) companyInfoSixth};

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

  TEST_WITH_METADATA(dsvo::MetaDataSource symChgMetaExp("symbolChange" COMMA t_eDataBuild::REVERSE COMMA t_eDataPatch::DELTACHANGE));
  t_dataSet companyRecordRelistedExpected {TEST_WITH_METADATA(symChgMetaExp COMMA) companyInfoRelisted};

  t_versionObject::t_datasetLedger::const_iterator companyRecordRelistedActual =
    vo.getVersionAt(t_versionDate(std::chrono::year(int(2022)), std::chrono::January, std::chrono::day(unsigned(12))));

  unittest::ExpectEqual(bool, true, companyRecordRelistedActual != vo.getDatasetLedger().cend()); // has t_dataSet

  unittest::ExpectEqual(t_dataSet, companyRecordRelistedExpected,
                                   companyRecordRelistedActual->second);



//  NOTE: the row below is not a versioned information, but info from EQUITY_L.csv
//  19-Sep-2022,EQUITY_L,ANDHRAPAPLTD,ANDHRA PAPER LIMITED,EQ,10,1,INE546B12139,10,LISTED
  t_companyInfo companyInfoAfterRelisting = t_convertFromString::ToVal(
    "ANDHRAPAPLTD,ANDHRA PAPER LIMITED,EQ,10,1,INE546B12139,10,LISTED"    );

  TEST_WITH_METADATA(dsvo::MetaDataSource latestMetaExp("EQUITY_L" COMMA t_eDataBuild::IsRECORD COMMA t_eDataPatch::FullRECORD));
  t_dataSet companyRecordAfterRelistedExpected {TEST_WITH_METADATA(latestMetaExp COMMA) companyInfoAfterRelisting};

  t_versionObject::t_datasetLedger::const_iterator companyRecordCrown2Actual =
    vo.getVersionAt(t_versionDate{std::chrono::year(int(2022)), std::chrono::September, std::chrono::day(unsigned(19))});

  unittest::ExpectEqual(bool, true, companyRecordCrown2Actual != vo.getDatasetLedger().cend()); // has t_dataSet

  unittest::ExpectEqual(t_dataSet, companyRecordAfterRelistedExpected,
                                   companyRecordCrown2Actual->second);

}
