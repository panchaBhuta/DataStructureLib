
#include <testHelper.h>

void interimReverseTest(t_versionObject& vo,
                        [[maybe_unused]] bool insertResultExpected,
                        t_versionObjectBuilder& vob,
                        const t_versionDate&  listingDate)
{
  vob.buildReverseTimeline( listingDate, vo);

  std::string voStrReverse =
    "13-May-2004," TEST_WITH_METADATA("-|%symbolChange|@nameSpot,") "APPAPER,International Paper APPM Limited,EQ,10,1,INE435A01028,10,LISTED\n"   // listingDate-of-reverse
    "21-Jan-2014," TEST_WITH_METADATA("-|%symbolChange|%nameChange,") "IPAPPM,International Paper APPM Limited,EQ,10,1,INE435A01028,10,LISTED\n"  // REVERSE
    "22-Jan-2020," TEST_WITH_METADATA("-|%symbolChange|@nameSpot,") "ANDPAPER,ANDHRA PAPER LIMITED,EQ,10,1,INE435A01028,10,LISTED\n"              // REVERSE
    "05-Mar-2020," TEST_WITH_METADATA("*|*EQUITY_L,") "ANDHRAPAP,ANDHRA PAPER LIMITED,EQ,10,1,INE435A01028,10,LISTED\n";                          // REVERSE + vo.insertVersion(...)
/*
    "07-Apr-2021," TEST_WITH_METADATA("+|@marketLotSpot,") "ANDHRAPAP,ANDHRA PAPER LIMITED,EQ,10,2,INE435A01028,10,LISTED\n";     // FORWARD

  if(!insertResultExpected)
  {
    // on second run
    voStrReverse += "07-Apr-2021," TEST_WITH_METADATA("+|@marketLotSpot,") "ANDHRAPAP,ANDHRA PAPER LIMITED,EQ,10,2,INE435A01028,10,LISTED\n";       // FORWARD
  }

  unittest::ExpectEqual(std::string, voStrReverse, t_versionObjectStream::createVOstreamer(vo).toCSV(TEST_WITH_METADATA(sh)));

  vob.clear();


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
  //   Only FORWARD Change.  "vob.buildReverseTimeline(listingDate, vo)" doesn't work here

  vob.buildForwardTimeline(vo);


  //std::cout << "#### vo start ######\n" << t_versionObjectStream::createVOstreamer(vo).toCSV(TEST_WITH_METADATA(sh)) << "#### vo end ######\n";
  std::string voStrForward = voStrReverse +
    (insertResultExpected?"07-Apr-2021," TEST_WITH_METADATA("+|@marketLotSpot,") "ANDHRAPAP,ANDHRA PAPER LIMITED,EQ,10,2,INE435A01028,10,LISTED\n":"");  // FORWARD
*/

  const dsvo::StreamerHelper sh{};
  unittest::ExpectEqual(std::string, voStrReverse, t_versionObjectStream::createVOstreamer(vo).toCSV(TEST_WITH_METADATA(sh)));
}


void endReverseTest([[maybe_unused]] t_versionObject& vo,
                    [[maybe_unused]] bool insertResultExpected,
                    [[maybe_unused]] t_versionObjectBuilder& vob,
                    [[maybe_unused]] const t_versionDate&  listingDate)
{
/*
//  ",,,0,2,,0,"
//  ANDHRAPAP,ANDHRA PAPER LIMITED,EQ,10,2,INE435A01028,10
  t_companyInfo companyInfoFifth = t_convertFromString::ToVal(
    "ANDHRAPAP,ANDHRA PAPER LIMITED,EQ,10,2,INE435A01028,10,LISTED"    );

  TEST_WITH_METADATA(dsvo::MetaDataSource lotChgMetaExp("marketLotChange" COMMA t_eDataBuild::FORWARD COMMA t_eDataPatch::SNAPSHOT));
  t_dataSet companyRecordFifthExpected {TEST_WITH_METADATA(lotChgMetaExp COMMA) companyInfoFifth};

  typename t_versionObject::t_datasetLedger::const_iterator companyRecordFifthActual =
    vo.getVersionAt(t_versionDate{std::chrono::year(int(2021)), std::chrono::April, std::chrono::day(unsigned(07))});

  unittest::ExpectEqual(bool, true, companyRecordFifthActual != vo.getDatasetLedger().cend()); // has t_dataSet

  unittest::ExpectEqual(t_dataSet, companyRecordFifthExpected,
                                   companyRecordFifthActual->second);
*/
}
