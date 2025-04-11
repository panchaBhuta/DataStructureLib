// test Snapshot changes : marketlot + Delisted + Relisted
#include <type_traits>

#include <testHelper.h>

void interimReverseTest(t_versionObject& vo,
                        [[maybe_unused]] bool insertResultExpected,
                        dsvo::VersionedObjectBuilder<t_versionDate, COMPANYINFO_TYPE_LIST>& vob,
                        [[maybe_unused]] const t_versionDate&  listingDate)
{
/*
  {
    //startDates.push_back(t_versionDate{std::chrono::year(int(2021)), std::chrono::April, std::chrono::day(unsigned(07))});
    const std::array <bool, std::tuple_size_v<t_companyInfo> > lotChangeFlg = {false, false, false, false, true, false, false, false};

    t_companyInfo lotChgOldInfo = converter::ConvertFromString<COMPANYINFO_TYPE_LIST>::ToVal(",,,0,1,,0,");
    t_companyInfo lotChgNewInfo = converter::ConvertFromString<COMPANYINFO_TYPE_LIST>::ToVal(",,,0,2,,0,");
    dsvo::ChangesInDataSet<COMPANYINFO_TYPE_LIST> lotChange {lotChangeFlg, lotChgOldInfo, lotChgNewInfo, dsvo::ApplicableChangeDirection::REVERSE};   // Delta Change
    bool insertResult = vob.insertDeltaVersion(t_versionDate{std::chrono::year(int(2021)), std::chrono::April, std::chrono::day(unsigned(07))}, lotChange);
    unittest::ExpectEqual(bool, true, insertResult);
  }
*/

  {
    ////////////////////////              DELISTED change test
    const std::array <bool, std::tuple_size_v<t_companyInfo> > delistedChangeFlg = {false, false, false, false, false, false, false, true};

    t_companyInfo delistedChgOldInfo = converter::ConvertFromString<COMPANYINFO_TYPE_LIST>::ToVal(",,,0,0,,0,LISTED");
    t_companyInfo delistedChgNewInfo = converter::ConvertFromString<COMPANYINFO_TYPE_LIST>::ToVal(",,,0,0,,0,DELISTED");
    dsvo::ChangesInDataSet<COMPANYINFO_TYPE_LIST> delistedChange {delistedChangeFlg, delistedChgOldInfo, delistedChgNewInfo, dsvo::ApplicableChangeDirection::REVERSE};   // DELISTED Change
    bool insertResult = vob.insertDeltaVersion(t_versionDate{std::chrono::year(int(2021)), std::chrono::December, std::chrono::day(unsigned(17))}, delistedChange);
    unittest::ExpectEqual(bool, true, insertResult);
  }

  {
    ////////////////////////              RELISTED change
    const std::array <bool, std::tuple_size_v<t_companyInfo> > relistedChangeFlg = {true, false, false, false, true, true, false, true};

    t_companyInfo relistedChgOldInfo = converter::ConvertFromString<COMPANYINFO_TYPE_LIST>::ToVal("ANDHRAPAP,,,0,2,INE435A01028,0,DELISTED");
    t_companyInfo relistedChgNewInfo = converter::ConvertFromString<COMPANYINFO_TYPE_LIST>::ToVal("ANDHRAPAPER,,,0,1,INE546B12139,0,LISTED");
    dsvo::ChangesInDataSet<COMPANYINFO_TYPE_LIST> relistedChange {relistedChangeFlg, relistedChgOldInfo, relistedChgNewInfo, dsvo::ApplicableChangeDirection::REVERSE};   // DELISTED Change
    bool insertResult = vob.insertDeltaVersion(t_versionDate{std::chrono::year(int(2022)), std::chrono::January, std::chrono::day(unsigned(12))}, relistedChange);
    unittest::ExpectEqual(bool, true, insertResult);
  }

  {
    ////////////////////////              symbolchange test
    const std::array <bool, std::tuple_size_v<t_companyInfo> > symbolChangeFlg = {true, false, false, false, false, false, false, false};

    t_companyInfo symChgOldInfo4 = converter::ConvertFromString<COMPANYINFO_TYPE_LIST>::ToVal("ANDHRAPAPER,,,0,0,,0,");
    t_companyInfo symChgNewInfo4 = converter::ConvertFromString<COMPANYINFO_TYPE_LIST>::ToVal("ANDHRAPAPLTD,,,0,0,,0,");
    dsvo::ChangesInDataSet<COMPANYINFO_TYPE_LIST> symbolChange4 {symbolChangeFlg, symChgOldInfo4, symChgNewInfo4, dsvo::ApplicableChangeDirection::REVERSE};  // DELTA Change
    bool insertResult = vob.insertDeltaVersion(t_versionDate{std::chrono::year(int(2022)), std::chrono::September, std::chrono::day(unsigned(19))}, symbolChange4);
    unittest::ExpectEqual(bool, true, insertResult);
  }

  {
    t_companyInfo companyInfoAfterRelisting = converter::ConvertFromString<COMPANYINFO_TYPE_LIST>::ToVal(
      "ANDHRAPAPLTD,ANDHRA PAPER LIMITED,EQ,10,1,INE546B12139,10,LISTED"    );

    dsvo::DataSet<COMPANYINFO_TYPE_LIST> companyRecordAfterRelistedExpected {companyInfoAfterRelisting};
    t_versionDate afterRelistedVersionDate{std::chrono::year(int(2022)), std::chrono::September, std::chrono::day(unsigned(19))};
    VERSIONEDOBJECT_DEBUG_MSG( "debug_LOG: vo.insertVersion() -> afterRelisted-next-versionDate: " << afterRelistedVersionDate << "; DATASET{" << companyRecordAfterRelistedExpected.toCSV() << "}");
    bool insertResult = vo.insertVersion(afterRelistedVersionDate, companyRecordAfterRelistedExpected);
    VERSIONEDOBJECT_DEBUG_MSG( "debug_LOG: vo.insertVersion() afterRelisted-insertResult=" << insertResult);
    unittest::ExpectEqual(bool, insertResultExpected, insertResult);
  }

  using t_vob = typename std::remove_reference<decltype(vob)>::type;
  std::vector<typename t_vob::t_versionDate> startDates{};
  startDates.push_back(t_versionDate{std::chrono::year(int(2004)), std::chrono::May, std::chrono::day(unsigned(13))});
  startDates.push_back(t_versionDate{std::chrono::year(int(2021)), std::chrono::April, std::chrono::day(unsigned(07))});
  //startDates.push_back(t_versionDate{std::chrono::year(int(2020)), std::chrono::March, std::chrono::day(unsigned(05))});

  auto buildResult = vob.buildBiDirectionalTimeline( startDates, vo);
  unittest::ExpectEqual(typename t_vob::t_deltaEntriesMap_iter_diff_type, 6, buildResult.first); // 6 calls to insertDeltaVersion()/vob.insertSnapshotVersion()
  unittest::ExpectEqual(typename t_vob::t_deltaEntriesMap_iter_diff_type, 0, buildResult.second);


  //std::cout << "#### vo start ######\n" << vo.toCSV() << "#### vo end ######\n";
  std::string voStrBidirection =
    "13-May-2004,APPAPER,International Paper APPM Limited,EQ,10,1,INE435A01028,10,LISTED\n"              // listingDate-1-of-reverse
    "21-Jan-2014,IPAPPM,International Paper APPM Limited,EQ,10,1,INE435A01028,10,LISTED\n"               // REVERSE
    "22-Jan-2020,ANDPAPER,ANDHRA PAPER LIMITED,EQ,10,1,INE435A01028,10,LISTED\n"                         // REVERSE
    "05-Mar-2020,ANDHRAPAP,ANDHRA PAPER LIMITED,EQ,10,1,INE435A01028,10,LISTED\n"                        // REVERSE + vo.insertVersion(...)
    "07-Apr-2021,ANDHRAPAP,ANDHRA PAPER LIMITED,EQ,10,2,INE435A01028,10,LISTED\n"                        // listingDate-2-of-reverse
    "17-Dec-2021,ANDHRAPAP,ANDHRA PAPER LIMITED,EQ,10,2,INE435A01028,10,DELISTED\n"                      // REVERSE
    "12-Jan-2022,ANDHRAPAPER,ANDHRA PAPER LIMITED,EQ,10,1,INE546B12139,10,LISTED\n"                      // REVERSE
    "19-Sep-2022,ANDHRAPAPLTD,ANDHRA PAPER LIMITED,EQ,10,1,INE546B12139,10,LISTED\n";                    // REVERSE + vo.insertVersion(...)

  unittest::ExpectEqual(std::string, voStrBidirection, vo.toCSV());
}


void endReverseTest(                 t_versionObject& vo,
                    [[maybe_unused]] bool insertResultExpected,
                    [[maybe_unused]] dsvo::VersionedObjectBuilder<t_versionDate, COMPANYINFO_TYPE_LIST>& vob,
                    [[maybe_unused]] const t_versionDate&  listingDate)
{

//  ",,,0,2,,0,"
//  ANDHRAPAP,ANDHRA PAPER LIMITED,EQ,10,2,INE435A01028,10
  t_companyInfo companyInfoFifth = converter::ConvertFromString<COMPANYINFO_TYPE_LIST>::ToVal(
    "ANDHRAPAP,ANDHRA PAPER LIMITED,EQ,10,2,INE435A01028,10,LISTED"    );

  dsvo::DataSet<COMPANYINFO_TYPE_LIST> companyRecordFifthExpected {companyInfoFifth};

  t_versionObject::t_datasetLedger::const_iterator companyRecordFifthActual =
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


//  ANDHRA PAPER LIMITED,ANDHRAPAPER,ANDHRAPAPLTD,12-JAN-2022  {only for RELISTED_2}
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



//  NOTE: the row below is not a versioned information, but info from EQUITY_L.csv
//  19-Sep-2022,EQUITY_L,ANDHRAPAPLTD,ANDHRA PAPER LIMITED,EQ,10,1,INE546B12139,10,LISTED
  t_companyInfo companyInfoAfterRelisting = converter::ConvertFromString<COMPANYINFO_TYPE_LIST>::ToVal(
    "ANDHRAPAPLTD,ANDHRA PAPER LIMITED,EQ,10,1,INE546B12139,10,LISTED"    );

  dsvo::DataSet<COMPANYINFO_TYPE_LIST> companyRecordAfterRelistedExpected {companyInfoAfterRelisting};

  t_versionObject::t_datasetLedger::const_iterator companyRecordCrown2Actual =
    vo.getVersionAt(t_versionDate{std::chrono::year(int(2022)), std::chrono::September, std::chrono::day(unsigned(19))});

  unittest::ExpectEqual(bool, true, companyRecordCrown2Actual != vo.getDatasetLedger().cend()); // has dsvo::DataSet<COMPANYINFO_TYPE_LIST>

  unittest::ExpectEqual(dsvo::DataSet<COMPANYINFO_TYPE_LIST>, companyRecordAfterRelistedExpected,
                                                              companyRecordCrown2Actual->second);

}
