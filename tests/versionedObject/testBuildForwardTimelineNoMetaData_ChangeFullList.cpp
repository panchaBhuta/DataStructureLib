
#include <testHelper.h>

void interimForwardTest(t_versionObject& vo,
                        [[maybe_unused]] bool insertResultExpected,
                        t_versionObjectBuilder& vob)
{
  {
    ////   SNAPSHOT change test  : applicable for 'buildForwardTimeline'  ( NOT for buildReverseTimeline )
    const std::array <bool, std::tuple_size_v<t_companyInfo> > lotChangeFlg = {false, false, false, false, true, false, false, false};

    t_companyInfo lotChgInfo = t_convertFromString::ToVal(",,,0,2,,0,");
    TEST_WITH_METADATA(dsvo::MetaDataSource lotChgMeta("marketLotSpot" COMMA dsvo::MetaDataSource::eDataBuild::FORWARD));
    t_snapshotDataSet lotChange {TEST_WITH_METADATA(lotChgMeta COMMA) lotChangeFlg, lotChgInfo, dsvo::ApplicableChangeDirection::FORWARD};   // SNAPSHOT Change
    bool insertResult2 = vob.insertSnapshotVersion(t_versionDate{std::chrono::year(int(2021)), std::chrono::April, std::chrono::day(unsigned(07))}, lotChange);
    unittest::ExpectEqual(bool, true, insertResult2);
  }


  vob.buildForwardTimeline(vo);


  //std::cout << "#### vo start ######\n" << vo.toCSV() << "#### vo end ######\n";
  std::string voStrForward =
    "13-May-2004," TEST_WITH_METADATA("^manualDeduction,") "APPAPER,International Paper APPM Limited,EQ,10,1,INE435A01028,10,LISTED\n"       // vo.insertVersion(...)
    "21-Jan-2014," TEST_WITH_METADATA("+nameSpot#symbolChange,") "IPAPPM,International Paper APPM Limited,EQ,10,1,INE435A01028,10,LISTED\n"  // FORWARD
    "22-Jan-2020," TEST_WITH_METADATA("+nameChange#symbolChange,") "ANDPAPER,ANDHRA PAPER LIMITED,EQ,10,1,INE435A01028,10,LISTED\n"          // FORWARD
    "05-Mar-2020," TEST_WITH_METADATA("+nameSpot#symbolChange,") "ANDHRAPAP,ANDHRA PAPER LIMITED,EQ,10,1,INE435A01028,10,LISTED\n"           // FORWARD
    "07-Apr-2021," TEST_WITH_METADATA("+marketLotSpot,") "ANDHRAPAP,ANDHRA PAPER LIMITED,EQ,10,2,INE435A01028,10,LISTED\n";                  // FORWARD

  unittest::ExpectEqual(std::string, voStrForward, vo.toCSV());
}


void endForwardTest([[maybe_unused]] t_versionObject& vo,
                    [[maybe_unused]] bool insertResultExpected,
                    [[maybe_unused]] t_versionObjectBuilder& vob)
{}
