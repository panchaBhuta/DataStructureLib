// test Snapshot changes : marketlot + Delisted + Relisted
#include <type_traits>

#include <testHelper.h>

void interimReverseTest(t_versionObject& vo,
                        [[maybe_unused]] bool insertResultExpected,
                        t_versionObjectBuilder& vob,
                        [[maybe_unused]] const t_versionDate&  listingDate)
{
  {
    ////   SNAPSHOT change test  : applicable for 'buildForwardTimeline'  ( NOT for buildReverseTimeline )
    const std::array <bool, std::tuple_size_v<t_companyInfo> > lotChangeFlg = {false, false, false, false, true, false, false, false};

    t_companyInfo lotChgInfo = t_convertFromString::ToVal(",,,0,2,,0,");
    dsvo::SnapshotDataSet<COMPANYINFO_TYPE_LIST> lotChange {lotChangeFlg, lotChgInfo, t_eDataBuild::FORWARD};   // SNAPSHOT Change
    bool insertResult = vob.insertSnapshotVersion(t_versionDate{std::chrono::year(int(2021)), std::chrono::April, std::chrono::day(unsigned(07))}, lotChange);
    unittest::ExpectEqual(bool, true, insertResult);
  }

  {
    ////////////////////////              DELISTED change test
    const std::array <bool, std::tuple_size_v<t_companyInfo> > delistedChangeFlg = {false, false, false, false, false, false, false, true};

    t_companyInfo delistedChgInfo = t_convertFromString::ToVal(",,,0,0,,0,DELISTED");
    dsvo::SnapshotDataSet<COMPANYINFO_TYPE_LIST> delistedChange {delistedChangeFlg, delistedChgInfo, t_eDataBuild::FORWARD};   // DELISTED Change
    bool insertResult = vob.insertSnapshotVersion(t_versionDate{std::chrono::year(int(2021)), std::chrono::December, std::chrono::day(unsigned(17))}, delistedChange);
    unittest::ExpectEqual(bool, true, insertResult);
  }

  {
    ////////////////////////              RELISTED change test
    t_companyInfo companyInfoLatest = t_convertFromString::ToVal(
      "ANDHRAPAPER,ANDHRA PAPER LIMITED,EQ,10,1,INE546B12139,10,LISTED"    );

    dsvo::DataSet<COMPANYINFO_TYPE_LIST> companyRecordRelistedExpected {companyInfoLatest};
    t_versionDate relistedVersionDate{std::chrono::year(int(2022)), std::chrono::January, std::chrono::day(unsigned(12))};
    VERSIONEDOBJECT_DEBUG_MSG( "debug_LOG: vo.insertVersion() -> relisted-versionDate: " << relistedVersionDate << "; DATASET{" << companyRecordRelistedExpected.toCSV() << "}");
    bool insertResult = vo.insertVersion(relistedVersionDate, companyRecordRelistedExpected);
    VERSIONEDOBJECT_DEBUG_MSG( "debug_LOG: vo.insertVersion() relisted-insertResult=" << insertResult);
    unittest::ExpectEqual(bool, insertResultExpected, insertResult);
  }

  {
    const std::array <bool, std::tuple_size_v<t_companyInfo> > lotChangeFlg = {false, false, false, false, true, false, false, false};

    ////////////////////////              SNAPSHOT change test
    t_companyInfo lotChgInfo2 = t_convertFromString::ToVal(",,,0,5,,0,");
    dsvo::SnapshotDataSet<COMPANYINFO_TYPE_LIST> lotChange2 {lotChangeFlg, lotChgInfo2, t_eDataBuild::FORWARD};   // SNAPSHOT Change
    bool insertResult = vob.insertSnapshotVersion(t_versionDate{std::chrono::year(int(2023)), std::chrono::October, std::chrono::day(unsigned(28))}, lotChange2);
    unittest::ExpectEqual(bool, true, insertResult);
  }

    using t_vob = typename std::remove_reference<decltype(vob)>::type;
    std::vector<typename t_vob::t_versionDate> startDates{};
    startDates.push_back(t_versionDate{std::chrono::year(int(2004)), std::chrono::May, std::chrono::day(unsigned(13))});

    auto buildResult = vob.buildBiDirectionalTimeline( startDates, vo);
    unittest::ExpectEqual(typename t_vob::t_deltaEntriesMap_iter_diff_type, 6, buildResult.first); // 6 calls to insertDeltaVersion()/vob.insertSnapshotVersion()
    unittest::ExpectEqual(typename t_vob::t_deltaEntriesMap_iter_diff_type, 0, buildResult.second);


    //std::cout << "#### vo start ######\n" << vo.toCSV() << "#### vo end ######\n";
    std::string voStrBidirection =
    "13-May-2004,APPAPER,International Paper APPM Limited,EQ,10,1,INE435A01028,10,LISTED\n"              // listingDate-of-reverse
    "21-Jan-2014,IPAPPM,International Paper APPM Limited,EQ,10,1,INE435A01028,10,LISTED\n"               // REVERSE
    "22-Jan-2020,ANDPAPER,ANDHRA PAPER LIMITED,EQ,10,1,INE435A01028,10,LISTED\n"                         // REVERSE
    "05-Mar-2020,ANDHRAPAP,ANDHRA PAPER LIMITED,EQ,10,1,INE435A01028,10,LISTED\n"                        // REVERSE + vo.insertVersion(...)
    "07-Apr-2021,ANDHRAPAP,ANDHRA PAPER LIMITED,EQ,10,2,INE435A01028,10,LISTED\n"                        // FORWARD
    "17-Dec-2021,ANDHRAPAP,ANDHRA PAPER LIMITED,EQ,10,2,INE435A01028,10,DELISTED\n"                      // FORWARD
    "12-Jan-2022,ANDHRAPAPER,ANDHRA PAPER LIMITED,EQ,10,1,INE546B12139,10,LISTED\n"                      // vo.insertVersion(...)
    "28-Oct-2023,ANDHRAPAPER,ANDHRA PAPER LIMITED,EQ,10,5,INE546B12139,10,LISTED\n";                     // FORWARD

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

  dsvo::DataSet<COMPANYINFO_TYPE_LIST> companyRecordFifthExpected {companyInfoFifth};

  t_versionObject::t_datasetLedger::const_iterator companyRecordFifthActual =
    vo.getVersionAt(t_versionDate{std::chrono::year(int(2021)), std::chrono::April, std::chrono::day(unsigned(07))});

  unittest::ExpectEqual(bool, true, companyRecordFifthActual != vo.getDatasetLedger().cend()); // has dsvo::DataSet<COMPANYINFO_TYPE_LIST>

  unittest::ExpectEqual(dsvo::DataSet<COMPANYINFO_TYPE_LIST>, companyRecordFifthExpected,
                                                              companyRecordFifthActual->second);


//  ,,,0,0,,0,DELISTED
//  17-Dec-2021,ANDHRAPAP,ANDHRA PAPER LIMITED,EQ,10,2,INE435A01028,10,DELISTED
  t_companyInfo companyInfoSixth = t_convertFromString::ToVal(
    "ANDHRAPAP,ANDHRA PAPER LIMITED,EQ,10,2,INE435A01028,10,DELISTED"    );

  dsvo::DataSet<COMPANYINFO_TYPE_LIST> companyRecordSixthExpected {companyInfoSixth};

  t_versionObject::t_datasetLedger::const_iterator companyRecordSixthActual =
    vo.getVersionAt(t_versionDate{std::chrono::year(int(2021)), std::chrono::December, std::chrono::day(unsigned(17))});

  unittest::ExpectEqual(bool, true, companyRecordSixthActual != vo.getDatasetLedger().cend()); // has dsvo::DataSet<COMPANYINFO_TYPE_LIST>

  unittest::ExpectEqual(dsvo::DataSet<COMPANYINFO_TYPE_LIST>, companyRecordSixthExpected,
                                                              companyRecordSixthActual->second);



//  NOTE: the row below is not a versioned information, but info from EQUITY_L.csv
//  12-Jan-2022,ANDHRAPAPER,ANDHRA PAPER LIMITED,EQ,10,1,INE546B12139,10,LISTED
  t_companyInfo companyInfoRelisted = t_convertFromString::ToVal(
    "ANDHRAPAPER,ANDHRA PAPER LIMITED,EQ,10,1,INE546B12139,10,LISTED"    );

  dsvo::DataSet<COMPANYINFO_TYPE_LIST> companyRecordRelistedExpected {companyInfoRelisted};

  t_versionObject::t_datasetLedger::const_iterator companyRecordRelistedActual =
    vo.getVersionAt(t_versionDate(std::chrono::year(int(2022)), std::chrono::January, std::chrono::day(unsigned(12))));

  unittest::ExpectEqual(bool, true, companyRecordRelistedActual != vo.getDatasetLedger().cend()); // has dsvo::DataSet<COMPANYINFO_TYPE_LIST>

  unittest::ExpectEqual(dsvo::DataSet<COMPANYINFO_TYPE_LIST>, companyRecordRelistedExpected,
                                                              companyRecordRelistedActual->second);


//  ",,,0,5,,0,"
//  28-Oct-2023,ANDHRAPAPER,ANDHRA PAPER LIMITED,EQ,10,5,INE546B12139,10,LISTED
  t_companyInfo companyInfoEigth = t_convertFromString::ToVal(
    "ANDHRAPAPER,ANDHRA PAPER LIMITED,EQ,10,5,INE546B12139,10,LISTED"    );

  dsvo::DataSet<COMPANYINFO_TYPE_LIST> companyRecordEightExpected {companyInfoEigth};

  t_versionObject::t_datasetLedger::const_iterator companyRecordEightActual =
    vo.getVersionAt(t_versionDate{std::chrono::year(int(2023)), std::chrono::October, std::chrono::day(unsigned(28))});

  unittest::ExpectEqual(bool, true, companyRecordEightActual != vo.getDatasetLedger().cend()); // has dsvo::DataSet<COMPANYINFO_TYPE_LIST>

  unittest::ExpectEqual(dsvo::DataSet<COMPANYINFO_TYPE_LIST>, companyRecordEightExpected,
                                                              companyRecordEightActual->second);

}
