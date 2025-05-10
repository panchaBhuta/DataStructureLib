// compare with testBuildForwardTimelineNoMetaData_ChangeFullList.cpp
#include <testHelper.h>

void interimForwardTest(t_versionObject& vo,
                        [[maybe_unused]] bool insertResultExpected,
                        t_versionObjectBuilder& vob)
{
  {
    ////////////////////////              LOT change test
    t_companyInfo companyInfoLatest = t_convertFromString::ToVal(
      "ANDHRAPAP,ANDHRA PAPER LIMITED,EQ,10,2,INE435A01028,10,LISTED"    );

    TEST_WITH_METADATA(dsvo::MetaDataSource manualMeta("manualMarketLotChange" COMMA t_eDataBuild::IsRECORD COMMA t_eDataPatch::FullRECORD));
    t_dataSet companyRecordLotChangeExpected {TEST_WITH_METADATA(manualMeta COMMA) companyInfoLatest};
    t_versionDate lotChangeVersionDate{std::chrono::year(int(2021)), std::chrono::April, std::chrono::day(unsigned(07))};
    VERSIONEDOBJECT_DEBUG_MSG( "debug_LOG: vo.insertVersion() -> lotChange-versionDate: " << lotChangeVersionDate << "; DATASET{" << companyRecordLotChangeExpected.toCSV(TEST_WITH_METADATA('#')) << "}");
    bool insertResult = vo.insertVersion(lotChangeVersionDate, companyRecordLotChangeExpected);
    VERSIONEDOBJECT_DEBUG_MSG( "debug_LOG: vo.insertVersion() lotChange-insertResult=" << insertResult);
    unittest::ExpectEqual(bool, insertResultExpected, insertResult);
  }

  {
    ////////////////////////              DELISTED change test
    const std::array <bool, std::tuple_size_v<t_companyInfo> > delistedChangeFlg = {false, false, false, false, false, false, false, true};

    t_companyInfo delistedChgInfo = t_convertFromString::ToVal(",,,0,0,,0,DELISTED");
    TEST_WITH_METADATA(dsvo::MetaDataSource delistedMeta("delistedSpot" COMMA t_eDataBuild::FORWARD COMMA t_eDataPatch::SNAPSHOT));
    t_snapshotDataSet delistedChange {delistedChangeFlg, delistedChgInfo,
                                      TEST_ALTERNATE_METADATA(delistedMeta, t_eDataBuild::FORWARD)};   // DELISTED Change
    bool insertResult = vob.insertSnapshotVersion(t_versionDate{std::chrono::year(int(2021)), std::chrono::December, std::chrono::day(unsigned(17))}, delistedChange);
    unittest::ExpectEqual(bool, true, insertResult);
  }

  {
    ////////////////////////              RELISTED change test
    t_companyInfo companyInfoLatest = t_convertFromString::ToVal(
      "ANDHRAPAPER,ANDHRA PAPER LIMITED,EQ,10,1,INE546B12139,10,LISTED"    );

    TEST_WITH_METADATA(dsvo::MetaDataSource relistedMeta("relisted" COMMA t_eDataBuild::IsRECORD COMMA t_eDataPatch::FullRECORD));
    t_dataSet companyRecordRelistedExpected {TEST_WITH_METADATA(relistedMeta COMMA) companyInfoLatest};
    t_versionDate relistedVersionDate{std::chrono::year(int(2022)), std::chrono::January, std::chrono::day(unsigned(12))};
    VERSIONEDOBJECT_DEBUG_MSG( "debug_LOG: vo.insertVersion() -> relisted-versionDate: " << relistedVersionDate << "; DATASET{" << companyRecordRelistedExpected.toCSV(TEST_WITH_METADATA('#')) << "}");
    bool insertResult = vo.insertVersion(relistedVersionDate, companyRecordRelistedExpected);
    VERSIONEDOBJECT_DEBUG_MSG( "debug_LOG: vo.insertVersion() relisted-insertResult=" << insertResult);
    unittest::ExpectEqual(bool, insertResultExpected, insertResult);
  }

  {
    ////////////////////////              SNAPSHOT change test
    const std::array <bool, std::tuple_size_v<t_companyInfo> > lotChangeFlg = {false, false, false, false, true, false, false, false};

    t_companyInfo lotChgInfo2 = t_convertFromString::ToVal(",,,0,5,,0,");
    TEST_WITH_METADATA(dsvo::MetaDataSource lotChgMeta("marketLotSpot" COMMA t_eDataBuild::FORWARD COMMA t_eDataPatch::SNAPSHOT));
    t_snapshotDataSet lotChange2 {lotChangeFlg, lotChgInfo2,
                                  TEST_ALTERNATE_METADATA(lotChgMeta, t_eDataBuild::FORWARD)};   // SNAPSHOT Change
    bool insertResult2 = vob.insertSnapshotVersion(t_versionDate{std::chrono::year(int(2023)), std::chrono::October, std::chrono::day(unsigned(28))}, lotChange2);
    unittest::ExpectEqual(bool, true, insertResult2);
  }

  std::vector<typename t_versionObjectBuilder::t_versionDate> startDates{};  // in pure forward build, 'startDates' is not used, hence we pass it as empty vector.

  auto buildResult = vob.buildBiDirectionalTimeline( startDates, vo);
  unittest::ExpectEqual(typename t_versionObjectBuilder::t_deltaEntriesMap_iter_diff_type, 5, buildResult.first); // 5 calls to combo-insertDeltaVersion()/vob.insertSnapshotVersion()
  unittest::ExpectEqual(typename t_versionObjectBuilder::t_deltaEntriesMap_iter_diff_type, 0, buildResult.second);


  //std::cout << "#### vo start ######\n" << t_versionObjectStream::createVOstreamer(vo).toCSV(TEST_WITH_METADATA('#')) << "#### vo end ######\n";
  std::string voStrForward =
    "13-May-2004," TEST_WITH_METADATA("*#*manualDeduction,") "APPAPER,International Paper APPM Limited,EQ,10,1,INE435A01028,10,LISTED\n"        // vo.insertVersion(...)
    "21-Jan-2014," TEST_WITH_METADATA("+#%symbolChange#@nameSpot,") "IPAPPM,International Paper APPM Limited,EQ,10,1,INE435A01028,10,LISTED\n"  // FORWARD
    "22-Jan-2020," TEST_WITH_METADATA("+#%symbolChange#%nameChange,") "ANDPAPER,ANDHRA PAPER LIMITED,EQ,10,1,INE435A01028,10,LISTED\n"          // FORWARD
    "05-Mar-2020," TEST_WITH_METADATA("+#%symbolChange#@nameSpot,") "ANDHRAPAP,ANDHRA PAPER LIMITED,EQ,10,1,INE435A01028,10,LISTED\n"           // FORWARD
    "07-Apr-2021," TEST_WITH_METADATA("*#*manualMarketLotChange,") "ANDHRAPAP,ANDHRA PAPER LIMITED,EQ,10,2,INE435A01028,10,LISTED\n"            // vo.insertVersion(...)
    "17-Dec-2021," TEST_WITH_METADATA("+#@delistedSpot,") "ANDHRAPAP,ANDHRA PAPER LIMITED,EQ,10,2,INE435A01028,10,DELISTED\n"                   // FORWARD
    "12-Jan-2022," TEST_WITH_METADATA("*#*relisted,") "ANDHRAPAPER,ANDHRA PAPER LIMITED,EQ,10,1,INE546B12139,10,LISTED\n"                       // vo.insertVersion(...)
    "28-Oct-2023," TEST_WITH_METADATA("+#@marketLotSpot,") "ANDHRAPAPER,ANDHRA PAPER LIMITED,EQ,10,5,INE546B12139,10,LISTED\n";                 // FORWARD

  unittest::ExpectEqual(std::string, voStrForward, t_versionObjectStream::createVOstreamer(vo).toCSV(TEST_WITH_METADATA('#')));
}


void endForwardTest([[maybe_unused]] t_versionObject& vo,
                    [[maybe_unused]] bool insertResultExpected,
                    [[maybe_unused]] t_versionObjectBuilder& vob)
{
//  ANDHRAPAP,ANDHRA PAPER LIMITED,EQ,10,2,INE435A01028,10
  t_companyInfo companyInfoFifth = t_convertFromString::ToVal(
    "ANDHRAPAP,ANDHRA PAPER LIMITED,EQ,10,2,INE435A01028,10,LISTED"    );

  TEST_WITH_METADATA(dsvo::MetaDataSource lotChgMetaExp1{"manualMarketLotChange" COMMA t_eDataBuild::IsRECORD COMMA t_eDataPatch::FullRECORD});
  t_dataSet companyRecordFifthExpected {TEST_WITH_METADATA(lotChgMetaExp1 COMMA) companyInfoFifth};

  typename t_versionObject::t_datasetLedger::const_iterator companyRecordFifthActual =
    vo.getVersionAt(t_versionDate{std::chrono::year(int(2021)), std::chrono::April, std::chrono::day(unsigned(07))});

  unittest::ExpectEqual(bool, true, companyRecordFifthActual != vo.getDatasetLedger().cend()); // has t_dataSet

  unittest::ExpectEqual(t_dataSet, companyRecordFifthExpected,
                                   companyRecordFifthActual->second);


//  ,,,0,0,,0,DELISTED
//  17-Dec-2021,ANDHRAPAP,ANDHRA PAPER LIMITED,EQ,10,2,INE435A01028,10,DELISTED
  t_companyInfo companyInfoSixth = t_convertFromString::ToVal(
    "ANDHRAPAP,ANDHRA PAPER LIMITED,EQ,10,2,INE435A01028,10,DELISTED"    );

  TEST_WITH_METADATA(dsvo::MetaDataSource lotChgMetaExp2{"delistedSpot" COMMA t_eDataBuild::FORWARD COMMA t_eDataPatch::SNAPSHOT});
  t_dataSet companyRecordSixthExpected {TEST_WITH_METADATA(lotChgMetaExp2 COMMA) companyInfoSixth};

  t_versionObject::t_datasetLedger::const_iterator companyRecordSixthActual =
    vo.getVersionAt(t_versionDate{std::chrono::year(int(2021)), std::chrono::December, std::chrono::day(unsigned(17))});

  unittest::ExpectEqual(bool, true, companyRecordSixthActual != vo.getDatasetLedger().cend()); // has t_dataSet

  unittest::ExpectEqual(t_dataSet, companyRecordSixthExpected,
                                   companyRecordSixthActual->second);



//  NOTE: the row below is not a versioned information, but info from EQUITY_L.csv
//  12-Jan-2022,ANDHRAPAPER,ANDHRA PAPER LIMITED,EQ,10,1,INE546B12139,10,LISTED
  t_companyInfo companyInfoRelisted = t_convertFromString::ToVal(
    "ANDHRAPAPER,ANDHRA PAPER LIMITED,EQ,10,1,INE546B12139,10,LISTED"    );

  TEST_WITH_METADATA(dsvo::MetaDataSource relistedMetaExp{"relisted" COMMA t_eDataBuild::IsRECORD COMMA t_eDataPatch::FullRECORD});
  t_dataSet companyRecordRelistedExpected {TEST_WITH_METADATA(relistedMetaExp COMMA) companyInfoRelisted};

  t_versionObject::t_datasetLedger::const_iterator companyRecordRelistedActual =
    vo.getVersionAt(t_versionDate(std::chrono::year(int(2022)), std::chrono::January, std::chrono::day(unsigned(12))));

  unittest::ExpectEqual(bool, true, companyRecordRelistedActual != vo.getDatasetLedger().cend()); // has t_dataSet

  unittest::ExpectEqual(t_dataSet, companyRecordRelistedExpected,
                                   companyRecordRelistedActual->second);



//  ",,,0,5,,0,"
//  ANDHRAPAPER,ANDHRA PAPER LIMITED,EQ,10,5,INE546B12139,10,LISTED
  t_companyInfo companyInfoEighth = t_convertFromString::ToVal(
    "ANDHRAPAPER,ANDHRA PAPER LIMITED,EQ,10,5,INE546B12139,10,LISTED"    );

  TEST_WITH_METADATA(dsvo::MetaDataSource lotChgMetaExp{"marketLotChange" COMMA t_eDataBuild::FORWARD COMMA t_eDataPatch::SNAPSHOT});
  t_dataSet companyRecordEigthExpected {TEST_WITH_METADATA(lotChgMetaExp COMMA) companyInfoEighth};

  typename t_versionObject::t_datasetLedger::const_iterator companyRecordEigthActual =
    vo.getVersionAt(t_versionDate{std::chrono::year(int(2023)), std::chrono::October, std::chrono::day(unsigned(28))});

  unittest::ExpectEqual(bool, true, companyRecordEigthActual != vo.getDatasetLedger().cend()); // has t_dataSet

  unittest::ExpectEqual(t_dataSet, companyRecordEigthExpected,
                                   companyRecordEigthActual->second);
}
