#include <type_traits>

#include <testHelper.h>

void interimForwardTest(t_versionObject& vo,
                        [[maybe_unused]] bool insertResultExpected,
                        dsvo::VersionedObjectBuilder<t_versionDate, COMPANYINFO_TYPE_LIST>& vob)
{
  using t_vob = typename std::remove_reference<decltype(vob)>::type;
  std::vector<typename t_vob::t_versionDate> startDates{};  // in pure forward build, 'startDates' is not used, hence we pass it as empty vector.

  auto buildResult = vob.buildBiDirectionalTimeline( startDates, vo);
  unittest::ExpectEqual(typename t_vob::t_deltaEntriesMap_iter_diff_type, 3, buildResult.first); // 3 calls to combo-insertDeltaVersion()/vob.insertSnapshotVersion()
  unittest::ExpectEqual(typename t_vob::t_deltaEntriesMap_iter_diff_type, 0, buildResult.second);

  std::string voStrFirst =
    "13-May-2004,APPAPER,International Paper APPM Limited,EQ,10,1,INE435A01028,10,LISTED\n"       // vo.insertVersion(...)
    "21-Jan-2014,IPAPPM,International Paper APPM Limited,EQ,10,1,INE435A01028,10,LISTED\n"        // FORWARD
    "22-Jan-2020,ANDPAPER,ANDHRA PAPER LIMITED,EQ,10,1,INE435A01028,10,LISTED\n"                  // FORWARD
    "05-Mar-2020,ANDHRAPAP,ANDHRA PAPER LIMITED,EQ,10,1,INE435A01028,10,LISTED\n";                // FORWARD

  if(!insertResultExpected)
  {
    // on second run
    voStrFirst += "07-Apr-2021,ANDHRAPAP,ANDHRA PAPER LIMITED,EQ,10,2,INE435A01028,10,LISTED\n";  // FORWARD
  }

  unittest::ExpectEqual(std::string, voStrFirst, vo.toCSV());

  vob.clear();


  {
    ////   SNAPSHOT change test  : applicable for 'buildForwardTimeline'  ( NOT for buildReverseTimeline )
    const std::array <bool, std::tuple_size_v<t_companyInfo> > lotChangeFlg = {false, false, false, false, true, false, false, false};

    t_companyInfo lotChgInfo = converter::ConvertFromString<COMPANYINFO_TYPE_LIST>::ToVal(",,,0,2,,0,");
    dsvo::SnapshotDataSet<COMPANYINFO_TYPE_LIST> lotChange {lotChangeFlg, lotChgInfo, dsvo::ApplicableChangeDirection::FORWARD};   // SNAPSHOT Change
    bool insertResult2 = vob.insertSnapshotVersion(t_versionDate{std::chrono::year(int(2021)), std::chrono::April, std::chrono::day(unsigned(07))}, lotChange);
    unittest::ExpectEqual(bool, true, insertResult2);
  }

  buildResult = vob.buildBiDirectionalTimeline( startDates, vo);
  unittest::ExpectEqual(typename t_vob::t_deltaEntriesMap_iter_diff_type, 1, buildResult.first); // 1 calls to combo-insertDeltaVersion()/vob.insertSnapshotVersion()
  unittest::ExpectEqual(typename t_vob::t_deltaEntriesMap_iter_diff_type, 0, buildResult.second);


  //std::cout << "#### vo start ######\n" << vo.toCSV() << "#### vo end ######\n";
  std::string voStrForward = voStrFirst +
    (insertResultExpected?"07-Apr-2021,ANDHRAPAP,ANDHRA PAPER LIMITED,EQ,10,2,INE435A01028,10,LISTED\n":"");

  unittest::ExpectEqual(std::string, voStrForward, vo.toCSV());
}


void endForwardTest([[maybe_unused]] t_versionObject& vo,
                    [[maybe_unused]] bool insertResultExpected,
                    [[maybe_unused]] dsvo::VersionedObjectBuilder<t_versionDate, COMPANYINFO_TYPE_LIST>& vob)
{
//  ",,,0,2,,0,"
//  ANDHRAPAP,ANDHRA PAPER LIMITED,EQ,10,2,INE435A01028,10
  t_companyInfo companyInfoFifth = converter::ConvertFromString<COMPANYINFO_TYPE_LIST>::ToVal(
    "ANDHRAPAP,ANDHRA PAPER LIMITED,EQ,10,2,INE435A01028,10,LISTED"    );

  dsvo::DataSet<COMPANYINFO_TYPE_LIST> companyRecordFifthExpected {companyInfoFifth};

  typename t_versionObject::t_datasetLedger::const_iterator companyRecordFifthActual =
    vo.getVersionAt(t_versionDate{std::chrono::year(int(2021)), std::chrono::April, std::chrono::day(unsigned(07))});

  unittest::ExpectEqual(bool, true, companyRecordFifthActual != vo.getDatasetLedger().cend()); // has dsvo::DataSet<COMPANYINFO_TYPE_LIST>

  unittest::ExpectEqual(dsvo::DataSet<COMPANYINFO_TYPE_LIST>, companyRecordFifthExpected,
                                                              companyRecordFifthActual->second);

}
