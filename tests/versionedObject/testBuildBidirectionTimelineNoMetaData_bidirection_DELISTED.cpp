// test Snapshot changes : marketlot + Delisted
// compare with testBuildReverseTimelineNoMetaData_ChangeSplitList.cpp

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


  std::vector<typename t_versionObjectBuilder::t_versionDate> startDates{};
  startDates.push_back(listingDate);

  auto buildResult = vob.buildBiDirectionalTimeline( startDates, vo);
  unittest::ExpectEqual(typename t_versionObjectBuilder::t_deltaEntriesMap_iter_diff_type, 5, buildResult.first); // 5 calls to insertDeltaVersion()/vob.insertSnapshotVersion()
  unittest::ExpectEqual(typename t_versionObjectBuilder::t_deltaEntriesMap_iter_diff_type, 0, buildResult.second);


  //std::cout << "#### vo start ######\n" << t_versionObjectStream::createVOstreamer(vo).toCSV(TEST_WITH_METADATA('#')) << "#### vo end ######\n";
  std::string voStrBidirection =
    "13-May-2004," TEST_WITH_METADATA("-#%symbolChange#@nameSpot,") "APPAPER,International Paper APPM Limited,EQ,10,1,INE435A01028,10,LISTED\n"   // listingDate-of-reverse
    "21-Jan-2014," TEST_WITH_METADATA("-#%symbolChange#%nameChange,") "IPAPPM,International Paper APPM Limited,EQ,10,1,INE435A01028,10,LISTED\n"  // REVERSE
    "22-Jan-2020," TEST_WITH_METADATA("-#%symbolChange#@nameSpot,") "ANDPAPER,ANDHRA PAPER LIMITED,EQ,10,1,INE435A01028,10,LISTED\n"              // REVERSE
    "05-Mar-2020," TEST_WITH_METADATA("*#*EQUITY_L,") "ANDHRAPAP,ANDHRA PAPER LIMITED,EQ,10,1,INE435A01028,10,LISTED\n"                           // REVERSE + vo.insertVersion(...)
    "07-Apr-2021," TEST_WITH_METADATA("+#@marketLotSpot,") "ANDHRAPAP,ANDHRA PAPER LIMITED,EQ,10,2,INE435A01028,10,LISTED\n"                      // FORWARD
    "17-Dec-2021," TEST_WITH_METADATA("+#@delistedSpot,") "ANDHRAPAP,ANDHRA PAPER LIMITED,EQ,10,2,INE435A01028,10,DELISTED\n";                    // FORWARD

  unittest::ExpectEqual(std::string, voStrBidirection, t_versionObjectStream::createVOstreamer(vo).toCSV(TEST_WITH_METADATA('#')));
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

  TEST_WITH_METADATA(dsvo::MetaDataSource lotChgMetaExp{"marketLotChange" COMMA t_eDataBuild::FORWARD COMMA t_eDataPatch::SNAPSHOT});
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

  TEST_WITH_METADATA(dsvo::MetaDataSource delistedChgMeta{"delistedSpot" COMMA t_eDataBuild::FORWARD COMMA t_eDataPatch::SNAPSHOT});
  t_dataSet companyRecordSixthExpected {TEST_WITH_METADATA(delistedChgMeta COMMA) companyInfoSixth};

  t_versionObject::t_datasetLedger::const_iterator companyRecordSixthActual =
    vo.getVersionAt(t_versionDate{std::chrono::year(int(2021)), std::chrono::December, std::chrono::day(unsigned(17))});

  unittest::ExpectEqual(bool, true, companyRecordSixthActual != vo.getDatasetLedger().cend()); // has t_dataSet

  unittest::ExpectEqual(t_dataSet, companyRecordSixthExpected,
                                   companyRecordSixthActual->second);

}
