#include <type_traits>

#include <testHelper.h>

void interimForwardTest(t_versionObject& vo,
                        [[maybe_unused]] bool insertResultExpected,
                        dsvo::VersionedObjectBuilder<t_versionDate, COMPANYINFO_TYPE_LIST>& vob)
{
  {
    ////////////////////////              LOT change test
    t_companyInfo companyInfoLatest = converter::ConvertFromString<COMPANYINFO_TYPE_LIST>::ToVal(
      "ANDHRAPAP,ANDHRA PAPER LIMITED,EQ,10,2,INE435A01028,10,LISTED"    );

    dsvo::DataSet<COMPANYINFO_TYPE_LIST> companyRecordLotChangeExpected {companyInfoLatest};
    t_versionDate lotChangeVersionDate{std::chrono::year(int(2021)), std::chrono::April, std::chrono::day(unsigned(07))};
    VERSIONEDOBJECT_DEBUG_MSG( "debug_LOG: vo.insertVersion() -> lotChange-versionDate: " << lotChangeVersionDate << "; DATASET{" << companyRecordLotChangeExpected.toCSV() << "}");
    bool insertResult = vo.insertVersion(lotChangeVersionDate, companyRecordLotChangeExpected);
    VERSIONEDOBJECT_DEBUG_MSG( "debug_LOG: vo.insertVersion() lotChange-insertResult=" << insertResult);
    unittest::ExpectEqual(bool, insertResultExpected, insertResult);
  }

  {
    ////////////////////////              DELISTED change test
    const std::array <bool, std::tuple_size_v<t_companyInfo> > delistedChangeFlg = {false, false, false, false, false, false, false, true};

    t_companyInfo delistedChgInfo = converter::ConvertFromString<COMPANYINFO_TYPE_LIST>::ToVal(",,,0,0,,0,DELISTED");
    dsvo::SnapshotDataSet<COMPANYINFO_TYPE_LIST> delistedChange {delistedChangeFlg, delistedChgInfo, dsvo::ApplicableChangeDirection::FORWARD};   // DELISTED Change
    bool insertResult = vob.insertDeltaVersion(t_versionDate{std::chrono::year(int(2021)), std::chrono::December, std::chrono::day(unsigned(17))}, delistedChange);
    unittest::ExpectEqual(bool, true, insertResult);
  }

  {
    ////////////////////////              RELISTED change test
    t_companyInfo companyInfoLatest = converter::ConvertFromString<COMPANYINFO_TYPE_LIST>::ToVal(
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
    t_companyInfo lotChgInfo2 = converter::ConvertFromString<COMPANYINFO_TYPE_LIST>::ToVal(",,,0,5,,0,");
    dsvo::SnapshotDataSet<COMPANYINFO_TYPE_LIST> lotChange2 {lotChangeFlg, lotChgInfo2, dsvo::ApplicableChangeDirection::FORWARD};   // SNAPSHOT Change
    bool insertResult = vob.insertDeltaVersion(t_versionDate{std::chrono::year(int(2023)), std::chrono::October, std::chrono::day(unsigned(28))}, lotChange2);
    unittest::ExpectEqual(bool, true, insertResult);
  }

  using t_vob = typename std::remove_reference<decltype(vob)>::type;
  std::vector<typename t_vob::t_versionDate> startDates{};  // in pure forward build, 'startDates' is not used, hence we pass it as empty vector.

  auto buildResult = vob.buildBiDirectionalTimeline( startDates, vo);
  unittest::ExpectEqual(typename t_vob::t_deltaEntriesMap_iter_diff_type, 5, buildResult.first); // 5 calls to combo-insertDeltaVersion()/vob.insertSnapshotVersion()
  unittest::ExpectEqual(typename t_vob::t_deltaEntriesMap_iter_diff_type, 0, buildResult.second);


  //std::cout << "#### vo start ######\n" << vo.toCSV() << "#### vo end ######\n";
  std::string voStrForward =
    "13-May-2004,APPAPER,International Paper APPM Limited,EQ,10,1,INE435A01028,10,LISTED\n"       // vo.insertVersion(...)
    "21-Jan-2014,IPAPPM,International Paper APPM Limited,EQ,10,1,INE435A01028,10,LISTED\n"        // FORWARD
    "22-Jan-2020,ANDPAPER,ANDHRA PAPER LIMITED,EQ,10,1,INE435A01028,10,LISTED\n"                  // FORWARD
    "05-Mar-2020,ANDHRAPAP,ANDHRA PAPER LIMITED,EQ,10,1,INE435A01028,10,LISTED\n"                 // FORWARD
    "07-Apr-2021,ANDHRAPAP,ANDHRA PAPER LIMITED,EQ,10,2,INE435A01028,10,LISTED\n"                 // vo.insertVersion(...)
    "17-Dec-2021,ANDHRAPAP,ANDHRA PAPER LIMITED,EQ,10,2,INE435A01028,10,DELISTED\n"               // FORWARD
    "12-Jan-2022,ANDHRAPAPER,ANDHRA PAPER LIMITED,EQ,10,1,INE546B12139,10,LISTED\n"               // vo.insertVersion(...)
    "28-Oct-2023,ANDHRAPAPER,ANDHRA PAPER LIMITED,EQ,10,5,INE546B12139,10,LISTED\n";              // FORWARD

  unittest::ExpectEqual(std::string, voStrForward, vo.toCSV());
}


void endForwardTest([[maybe_unused]] t_versionObject& vo,
                    [[maybe_unused]] bool insertResultExpected,
                    [[maybe_unused]] dsvo::VersionedObjectBuilder<t_versionDate, COMPANYINFO_TYPE_LIST>& vob)
{
//  ANDHRAPAP,ANDHRA PAPER LIMITED,EQ,10,2,INE435A01028,10
  t_companyInfo companyInfoFifth = converter::ConvertFromString<COMPANYINFO_TYPE_LIST>::ToVal(
    "ANDHRAPAP,ANDHRA PAPER LIMITED,EQ,10,2,INE435A01028,10,LISTED"    );

  dsvo::DataSet<COMPANYINFO_TYPE_LIST> companyRecordFifthExpected {companyInfoFifth};

  typename t_versionObject::t_datasetLedger::const_iterator companyRecordFifthActual =
    vo.getVersionAt(t_versionDate{std::chrono::year(int(2021)), std::chrono::April, std::chrono::day(unsigned(07))});

  unittest::ExpectEqual(bool, true, companyRecordFifthActual != vo.getDatasetLedger().cend()); // has dsvo::DataSet<COMPANYINFO_TYPE_LIST>

  unittest::ExpectEqual(dsvo::DataSet<COMPANYINFO_TYPE_LIST>, companyRecordFifthExpected,
                                                              companyRecordFifthActual->second);


//  ,,,0,0,,0,DELISTED
//  17-Dec-2021,ANDHRAPAP,ANDHRA PAPER LIMITED,EQ,10,2,INE435A01028,10,DELISTED
  t_companyInfo companyInfoSixth = converter::ConvertFromString<COMPANYINFO_TYPE_LIST>::ToVal(
    "ANDHRAPAP,ANDHRA PAPER LIMITED,EQ,10,2,INE435A01028,10,DELISTED"    );

  dsvo::DataSet<COMPANYINFO_TYPE_LIST> companyRecordSixthExpected {companyInfoSixth};

  t_versionObject::t_datasetLedger::const_iterator companyRecordSixthActual =
    vo.getVersionAt(t_versionDate{std::chrono::year(int(2021)), std::chrono::December, std::chrono::day(unsigned(17))});

  unittest::ExpectEqual(bool, true, companyRecordSixthActual != vo.getDatasetLedger().cend()); // has dsvo::DataSet<COMPANYINFO_TYPE_LIST>

  unittest::ExpectEqual(dsvo::DataSet<COMPANYINFO_TYPE_LIST>, companyRecordSixthExpected,
                                                              companyRecordSixthActual->second);



//  NOTE: the row below is not a versioned information, but info from EQUITY_L.csv
//  12-Jan-2022,ANDHRAPAPER,ANDHRA PAPER LIMITED,EQ,10,1,INE546B12139,10,LISTED
  t_companyInfo companyInfoRelisted = converter::ConvertFromString<COMPANYINFO_TYPE_LIST>::ToVal(
    "ANDHRAPAPER,ANDHRA PAPER LIMITED,EQ,10,1,INE546B12139,10,LISTED"    );

  dsvo::DataSet<COMPANYINFO_TYPE_LIST> companyRecordRelistedExpected {companyInfoRelisted};

  t_versionObject::t_datasetLedger::const_iterator companyRecordRelistedActual =
    vo.getVersionAt(t_versionDate(std::chrono::year(int(2022)), std::chrono::January, std::chrono::day(unsigned(12))));

  unittest::ExpectEqual(bool, true, companyRecordRelistedActual != vo.getDatasetLedger().cend()); // has dsvo::DataSet<COMPANYINFO_TYPE_LIST>

  unittest::ExpectEqual(dsvo::DataSet<COMPANYINFO_TYPE_LIST>, companyRecordRelistedExpected,
                                                              companyRecordRelistedActual->second);

}
