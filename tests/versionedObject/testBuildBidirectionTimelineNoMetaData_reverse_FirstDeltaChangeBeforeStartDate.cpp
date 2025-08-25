// compare with testBuildBidirectionTimelineNoMetaData_changeFullList_Reverse.cpp
#include <testHelper.h>

/*
./input/EQUITY_L.csv   has entries
    SYMBOL,NAME OF COMPANY, SERIES, DATE OF LISTING, PAID UP VALUE, MARKET LOT, ISIN NUMBER, FACE VALUE
1?* ALOKINDS,Alok Industries Limited,EQ,19-FEB-2020,1,1,INE270A01029,1
2?* CHEMPLASTS,Chemplast Sanmar Limited,EQ,24-AUG-2021,5,1,INE488A01050,5

./input/namechange.csv
1?- ALOKTEXT,Alok Textile Industries Ltd,Alok Industries Limited,12-DEC-2000
2?- CHEMPLAST,CHEMICALS AND PLASTICS (I) LTD.,Chemplast Sanmar Limited,04-JUL-1996

./input/ListofDelistedCos_20_02_2024.xlsx :: sheet -> delisted.csv
    Symbol,Company Name,Board,Delisted Date,Type of Delisting
1?@ CHEMPLAST,Chemplast Sanmar Limited,Main Board,06/25/2012,Voluntary Delisting


Description
===========
EQUITY_L.csv   :  19-FEB-2020,ALOKINDS,Alok Industries Limited
namechange.csv :  12-DEC-2000,ALOKTEXT,Alok Textile Industries Ltd->Alok Industries Limited
ERROR reason   :  namechange-date(12-DEC-2000) is before listing-date(19-FEB-2020)
                  ChangeBeforeStartTimeline_CheckTemporaryDelisting

EQUITY_L.csv   :  24-AUG-2021,CHEMPLASTS,Chemplast Sanmar Limited
delisted.xlsx  :  25-jun-2012,CHEMPLAST,Chemplast Sanmar Limited,Voluntary Delisting
namechange.csv :  04-JUL-1996,CHEMPLAST,CHEMICALS AND PLASTICS (I) LTD.->Chemplast Sanmar Limited
ERROR reason   :  delisted-date(25-jun-2012) and namechange-date(04-JUL-1996) are before listing-date(24-AUG-2021)
                  ChangeBeforeStartTimeline_CheckTemporaryDelisting

*/

void loadVO([[maybe_unused]]t_versionObject& vo_notused,
            bool insertResultExpected)
{
  bool insertResult;
  insertResultExpected = true;  // override to 'true' when called 2nd time from main.cpp


  const std::array <bool, std::tuple_size_v<t_companyInfo> >   nameChangeFlg = {false, true, false, false, false, false, false, false};
  TEST_WITH_METADATA(dsvo::MetaDataSource namChgMeta("nameChange"   COMMA t_eDataBuild::REVERSE COMMA t_eDataPatch::DELTACHANGE));

  t_versionObject vo1;
  t_versionObjectBuilder vob1;

//  ALOKINDS,Alok Industries Limited,EQ,19-FEB-2020,1,1,INE270A01029,1
  TEST_WITH_METADATA(dsvo::MetaDataSource latestMeta("EQUITY_L" COMMA t_eDataBuild::IsRECORD COMMA t_eDataPatch::FullRECORD));
  t_companyInfo companyInfo1 = t_convertFromString::ToVal(
        "ALOKINDS,Alok Industries Limited,EQ,1,1,INE270A01029,1,LISTED"    );
  t_dataSet companyRecord1 {TEST_WITH_METADATA(latestMeta COMMA) companyInfo1};
  t_versionDate date1{std::chrono::year(int(2020)), std::chrono::February, std::chrono::day(unsigned(19))};
  VERSIONEDOBJECT_DEBUG_MSG( "DEBUG_LOG: vo.insertVersion() -> versionDate: " << date1 << "; DATASET{" << companyRecord1.toCSV() << "}");
  insertResult = vo1.insertVersion( date1, companyRecord1);
  VERSIONEDOBJECT_DEBUG_MSG( "DEBUG_LOG: vo.insertVersion() insertResult=" << insertResult);
  unittest::ExpectEqual(bool, insertResultExpected, insertResult);

//  12-DEC-2000,ALOKTEXT,Alok Textile Industries Ltd->Alok Industries Limited
  t_companyInfo namChgOldInfo1 = t_convertFromString::ToVal(",Alok Textile Industries Ltd,,0,0,,0,");
  t_companyInfo namChgNewInfo1 = t_convertFromString::ToVal(",Alok Industries Limited,,0,0,,0,");
  t_changesInDataSet nameChange1 {nameChangeFlg, namChgOldInfo1, namChgNewInfo1,
                                  TEST_ALTERNATE_METADATA(namChgMeta, t_eDataBuild::REVERSE)};  // DELTA Change
  insertResult = vob1.insertDeltaVersion(t_versionDate{std::chrono::year(int(2000)), std::chrono::December, std::chrono::day(unsigned(12))}, nameChange1);
  unittest::ExpectEqual(bool, true, insertResult);

  std::vector<typename t_versionObjectBuilder::t_versionDate> startDates1{};
  //  19-Feb-2020
  startDates1.push_back(t_versionDate{std::chrono::year(int(2020)), std::chrono::February, std::chrono::day(unsigned(19))});


  std::string errMsg1{  "ERROR(1) : failure in _VersionedObjectBuilderBase<VDT, MT...>::_buildReverseTimeline() : startDate[19-Feb-2020] should be less than first-changeDate[12-Dec-2000]\n"
                        "DataSet :: "  TEST_WITH_METADATA("-|%nameChange,")  "[REVERSE]:,Alok Textile Industries Ltd->Alok Industries Limited,,,,,,\n"
                        "VersionObjectBuilder :: versionDate=12-Dec-2000, deltaEntry = {"  TEST_WITH_METADATA("-|%nameChange,")
                        "[REVERSE]:,Alok Textile Industries Ltd->Alok Industries Limited,,,,,,}\n" };
  ExpectExceptionMsg(
      vob1.buildBiDirectionalTimeline( startDates1 COMMA vo1) ,
      datastructure::versionedObject::FirstDeltaChange_Before_StartDate_Timeline_exception ,
      errMsg1 );



  t_versionObject vo2;
  t_versionObjectBuilder vob2;

//  CHEMPLASTS,Chemplast Sanmar Limited,EQ,24-AUG-2021,5,1,INE488A01050,5
  t_companyInfo companyInfo2 = t_convertFromString::ToVal(
        "CHEMPLASTS,Chemplast Sanmar Limited,EQ,5,1,INE488A01050,5,LISTED"    );
  t_dataSet companyRecord2 {TEST_WITH_METADATA(latestMeta COMMA) companyInfo2};
  t_versionDate date2{std::chrono::year(int(2021)), std::chrono::August, std::chrono::day(unsigned(24))};
  VERSIONEDOBJECT_DEBUG_MSG( "DEBUG_LOG: vo.insertVersion() -> versionDate: " << date2 << "; DATASET{" << companyRecord2.toCSV() << "}");
  insertResult = vo2.insertVersion( date2, companyRecord2);
  VERSIONEDOBJECT_DEBUG_MSG( "DEBUG_LOG: vo.insertVersion() insertResult=" << insertResult);
  unittest::ExpectEqual(bool, insertResultExpected, insertResult);


//  25-jun-2012,CHEMPLAST,Chemplast Sanmar Limited,Voluntary Delisting
    const std::array <bool, std::tuple_size_v<t_companyInfo> > delistedChangeFlg = {false, false, false, false, false, false, false, true};
    TEST_WITH_METADATA(dsvo::MetaDataSource delistedChgMeta("delistedSpot" COMMA t_eDataBuild::FORWARD COMMA t_eDataPatch::SNAPSHOT));

    t_companyInfo delistedChgInfo = t_convertFromString::ToVal(",,,0,0,,0,DELISTED");
    t_snapshotDataSet delistedChange {delistedChangeFlg, delistedChgInfo,
                                      TEST_ALTERNATE_METADATA(delistedChgMeta, t_eDataBuild::FORWARD)};   // DELISTED Change
    insertResult = vob2.insertSnapshotVersion(t_versionDate{std::chrono::year(int(2012)), std::chrono::June, std::chrono::day(unsigned(25))}, delistedChange);
    unittest::ExpectEqual(bool, true, insertResult);



//  04-JUL-1996,CHEMPLAST,CHEMICALS AND PLASTICS (I) LTD.->Chemplast Sanmar Limited
  t_companyInfo namChgOldInfo2 = t_convertFromString::ToVal(",CHEMICALS AND PLASTICS (I) LTD.,,0,0,,0,");
  t_companyInfo namChgNewInfo2 = t_convertFromString::ToVal(",Chemplast Sanmar Limited,,0,0,,0,");
  t_changesInDataSet nameChange2 {nameChangeFlg, namChgOldInfo2, namChgNewInfo2,
                                  TEST_ALTERNATE_METADATA(namChgMeta, t_eDataBuild::REVERSE)};  // DELTA Change
  insertResult = vob2.insertDeltaVersion(t_versionDate{std::chrono::year(int(1996)), std::chrono::July, std::chrono::day(unsigned(04))}, nameChange2);
  unittest::ExpectEqual(bool, true, insertResult);

  std::vector<typename t_versionObjectBuilder::t_versionDate> startDates2{};
  //  24-AUG-2021
  startDates2.push_back(t_versionDate{std::chrono::year(int(2021)), std::chrono::August, std::chrono::day(unsigned(24))});

  std::string errMsg2{  "ERROR(1) : failure in _VersionedObjectBuilderBase<VDT, MT...>::_buildReverseTimeline() : startDate[24-Aug-2021] should be less than first-changeDate[04-Jul-1996]\n"
                        "DataSet :: "  TEST_WITH_METADATA("-|%nameChange,")  "[REVERSE]:,CHEMICALS AND PLASTICS (I) LTD.->Chemplast Sanmar Limited,,,,,,\n"
                        "VersionObjectBuilder :: versionDate=04-Jul-1996, deltaEntry = {"  TEST_WITH_METADATA("-|%nameChange,")
                        "[REVERSE]:,CHEMICALS AND PLASTICS (I) LTD.->Chemplast Sanmar Limited,,,,,,}\n" };
  ExpectExceptionMsg(
      vob2.buildBiDirectionalTimeline( startDates2 COMMA vo2) ,
      datastructure::versionedObject::FirstDeltaChange_Before_StartDate_Timeline_exception ,
      errMsg2 );

}

