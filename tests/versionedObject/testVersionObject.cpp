
#include <testHelper.h>

void loadVO(t_versionObject& vo,
            bool insertResultExpected)
{
  bool insertResult;


  std::string voStr =
    "13-May-2004,APPAPER,International Paper APPM Limited,EQ,10,1,INE435A01028,10,LISTED\n"
    "21-Jan-2014,IPAPPM,International Paper APPM Limited,EQ,10,1,INE435A01028,10,LISTED\n"
    "22-Jan-2020,ANDPAPER,ANDHRA PAPER LIMITED,EQ,10,1,INE435A01028,10,LISTED\n"
    "05-Mar-2020,ANDHRAPAP,ANDHRA PAPER LIMITED,EQ,10,1,INE435A01028,10,LISTED\n"
    "07-Apr-2021,ANDHRAPAP,ANDHRA PAPER LIMITED,EQ,10,2,INE435A01028,10,LISTED\n";

//  APPAPER,International Paper APPM Limited,EQ,13-MAY-2004,10,1,INE435A01028,10
  t_companyInfo companyInfo1 = converter::ConvertFromString<COMPANYINFO_TYPE_LIST>::ToVal(
        "APPAPER,International Paper APPM Limited,EQ,10,1,INE435A01028,10,LISTED"    );
  dsvo::DataSet<COMPANYINFO_TYPE_LIST> companyRecord1 {companyInfo1};
  t_versionDate date1{std::chrono::year(int(2004)), std::chrono::May, std::chrono::day(unsigned(13))};
  VERSIONEDOBJECT_DEBUG_MSG( "DEBUG_LOG: vo.insertVersion() -> versionDate: " << date1 << "; DATASET{" << companyRecord1.toCSV() << "}");
  insertResult = vo.insertVersion( date1, companyRecord1);
  VERSIONEDOBJECT_DEBUG_MSG( "DEBUG_LOG: vo.insertVersion() insertResult=" << insertResult);
  unittest::ExpectEqual(bool, insertResultExpected, insertResult);


  t_companyInfo companyInfo2 = converter::ConvertFromString<COMPANYINFO_TYPE_LIST>::ToVal(
        "IPAPPM,International Paper APPM Limited,EQ,10,1,INE435A01028,10,LISTED"    );
  dsvo::DataSet<COMPANYINFO_TYPE_LIST> companyRecord2 {companyInfo2};
  t_versionDate date2{std::chrono::year(int(2014)), std::chrono::January, std::chrono::day(unsigned(21))};
  VERSIONEDOBJECT_DEBUG_MSG( "DEBUG_LOG: vo.insertVersion() -> versionDate: " << date2 << "; DATASET{" << companyRecord2.toCSV() << "}");
  insertResult = vo.insertVersion( date2, companyRecord2);
  VERSIONEDOBJECT_DEBUG_MSG( "DEBUG_LOG: vo.insertVersion() insertResult=" << insertResult);
  unittest::ExpectEqual(bool, insertResultExpected, insertResult);


  t_companyInfo companyInfo3 = converter::ConvertFromString<COMPANYINFO_TYPE_LIST>::ToVal(
        "ANDPAPER,ANDHRA PAPER LIMITED,EQ,10,1,INE435A01028,10,LISTED"    );
  dsvo::DataSet<COMPANYINFO_TYPE_LIST> companyRecord3 {companyInfo3};
  t_versionDate date3{std::chrono::year(int(2020)), std::chrono::January, std::chrono::day(unsigned(22))};
  VERSIONEDOBJECT_DEBUG_MSG( "DEBUG_LOG: vo.insertVersion() -> versionDate: " << date3 << "; DATASET{" << companyRecord3.toCSV() << "}");
  insertResult = vo.insertVersion( date3, companyRecord3);
  VERSIONEDOBJECT_DEBUG_MSG( "DEBUG_LOG: vo.insertVersion() insertResult=" << insertResult);
  unittest::ExpectEqual(bool, insertResultExpected, insertResult);


  t_companyInfo companyInfo4 = converter::ConvertFromString<COMPANYINFO_TYPE_LIST>::ToVal(
        "ANDHRAPAP,ANDHRA PAPER LIMITED,EQ,10,1,INE435A01028,10,LISTED"    );
  dsvo::DataSet<COMPANYINFO_TYPE_LIST> companyRecord4 {companyInfo4};
  t_versionDate date4{std::chrono::year(int(2020)), std::chrono::March, std::chrono::day(unsigned(05))};
  VERSIONEDOBJECT_DEBUG_MSG( "DEBUG_LOG: vo.insertVersion() -> versionDate: " << date4 << "; DATASET{" << companyRecord4.toCSV() << "}");
  insertResult = vo.insertVersion( date4, companyRecord4);
  VERSIONEDOBJECT_DEBUG_MSG( "DEBUG_LOG: vo.insertVersion() insertResult=" << insertResult);
  unittest::ExpectEqual(bool, insertResultExpected, insertResult);

  t_companyInfo companyInfo5 = converter::ConvertFromString<COMPANYINFO_TYPE_LIST>::ToVal(
        "ANDHRAPAP,ANDHRA PAPER LIMITED,EQ,10,2,INE435A01028,10,LISTED"    );
  dsvo::DataSet<COMPANYINFO_TYPE_LIST> companyRecord5 {companyInfo5};
  t_versionDate date5{std::chrono::year(int(2021)), std::chrono::April, std::chrono::day(unsigned(07))};
  VERSIONEDOBJECT_DEBUG_MSG( "DEBUG_LOG: vo.insertVersion() -> versionDate: " << date5 << "; DATASET{" << companyRecord5.toCSV() << "}");
  insertResult = vo.insertVersion( date5, companyRecord5);
  VERSIONEDOBJECT_DEBUG_MSG( "DEBUG_LOG: vo.insertVersion() insertResult=" << insertResult);
  unittest::ExpectEqual(bool, insertResultExpected, insertResult);


/////////  getVersionAt()  ///////////

  t_versionObject::t_datasetLedger::const_iterator companyRecordSearch =
    vo.getVersionAt(t_versionDate{std::chrono::year(int(2004)), std::chrono::May, std::chrono::day(unsigned(12))});
  unittest::ExpectEqual(bool, true, vo.getDatasetLedger().cend() == companyRecordSearch);

  companyRecordSearch =
    vo.getVersionAt(t_versionDate{std::chrono::year(int(2004)), std::chrono::May, std::chrono::day(unsigned(13))});
  unittest::ExpectEqual(dsvo::DataSet<COMPANYINFO_TYPE_LIST>, companyRecord1,
                                                              companyRecordSearch->second);

  companyRecordSearch =
    vo.getVersionAt(t_versionDate{std::chrono::year(int(2004)), std::chrono::May, std::chrono::day(unsigned(14))});
  unittest::ExpectEqual(dsvo::DataSet<COMPANYINFO_TYPE_LIST>, companyRecord1,
                                                              companyRecordSearch->second);



  companyRecordSearch =
    vo.getVersionAt(t_versionDate{std::chrono::year(int(2020)), std::chrono::January, std::chrono::day(unsigned(21))});
  unittest::ExpectEqual(dsvo::DataSet<COMPANYINFO_TYPE_LIST>, companyRecord2,
                                                              companyRecordSearch->second);

  companyRecordSearch =
    vo.getVersionAt(t_versionDate{std::chrono::year(int(2020)), std::chrono::January, std::chrono::day(unsigned(22))});
  unittest::ExpectEqual(dsvo::DataSet<COMPANYINFO_TYPE_LIST>, companyRecord3,
                                                              companyRecordSearch->second);

  companyRecordSearch =
    vo.getVersionAt(t_versionDate{std::chrono::year(int(2020)), std::chrono::January, std::chrono::day(unsigned(23))});
  unittest::ExpectEqual(dsvo::DataSet<COMPANYINFO_TYPE_LIST>, companyRecord3,
                                                              companyRecordSearch->second);



  companyRecordSearch =
    vo.getVersionAt(t_versionDate{std::chrono::year(int(2021)), std::chrono::April, std::chrono::day(unsigned(6))});
  unittest::ExpectEqual(dsvo::DataSet<COMPANYINFO_TYPE_LIST>, companyRecord4,
                                                              companyRecordSearch->second);

  companyRecordSearch =
    vo.getVersionAt(t_versionDate{std::chrono::year(int(2021)), std::chrono::April, std::chrono::day(unsigned(7))});
  unittest::ExpectEqual(dsvo::DataSet<COMPANYINFO_TYPE_LIST>, companyRecord5,
                                                              companyRecordSearch->second);

  companyRecordSearch =
    vo.getVersionAt(t_versionDate{std::chrono::year(int(2021)), std::chrono::April, std::chrono::day(unsigned(8))});
  unittest::ExpectEqual(dsvo::DataSet<COMPANYINFO_TYPE_LIST>, companyRecord5,
                                                              companyRecordSearch->second);


/////////  getVersionBefore()  ///////////

  companyRecordSearch =
    vo.getVersionBefore(t_versionDate{std::chrono::year(int(2004)), std::chrono::May, std::chrono::day(unsigned(12))});
  unittest::ExpectEqual(bool, true, vo.getDatasetLedger().cend() == companyRecordSearch);

  companyRecordSearch =
    vo.getVersionBefore(t_versionDate{std::chrono::year(int(2004)), std::chrono::May, std::chrono::day(unsigned(13))});
  unittest::ExpectEqual(bool, true, vo.getDatasetLedger().cend() == companyRecordSearch);

  companyRecordSearch =
    vo.getVersionBefore(t_versionDate{std::chrono::year(int(2004)), std::chrono::May, std::chrono::day(unsigned(14))});
  unittest::ExpectEqual(dsvo::DataSet<COMPANYINFO_TYPE_LIST>, companyRecord1,
                                                              companyRecordSearch->second);



  companyRecordSearch =
    vo.getVersionBefore(t_versionDate{std::chrono::year(int(2020)), std::chrono::January, std::chrono::day(unsigned(21))});
  unittest::ExpectEqual(dsvo::DataSet<COMPANYINFO_TYPE_LIST>, companyRecord2,
                                                              companyRecordSearch->second);

  companyRecordSearch =
    vo.getVersionBefore(t_versionDate{std::chrono::year(int(2020)), std::chrono::January, std::chrono::day(unsigned(22))});
  unittest::ExpectEqual(dsvo::DataSet<COMPANYINFO_TYPE_LIST>, companyRecord2,
                                                              companyRecordSearch->second);

  companyRecordSearch =
    vo.getVersionBefore(t_versionDate{std::chrono::year(int(2020)), std::chrono::January, std::chrono::day(unsigned(23))});
  unittest::ExpectEqual(dsvo::DataSet<COMPANYINFO_TYPE_LIST>, companyRecord3,
                                                              companyRecordSearch->second);



  companyRecordSearch =
    vo.getVersionBefore(t_versionDate{std::chrono::year(int(2021)), std::chrono::April, std::chrono::day(unsigned(6))});
  unittest::ExpectEqual(dsvo::DataSet<COMPANYINFO_TYPE_LIST>, companyRecord4,
                                                              companyRecordSearch->second);

  companyRecordSearch =
    vo.getVersionBefore(t_versionDate{std::chrono::year(int(2021)), std::chrono::April, std::chrono::day(unsigned(7))});
  unittest::ExpectEqual(dsvo::DataSet<COMPANYINFO_TYPE_LIST>, companyRecord4,
                                                              companyRecordSearch->second);

  companyRecordSearch =
    vo.getVersionBefore(t_versionDate{std::chrono::year(int(2021)), std::chrono::April, std::chrono::day(unsigned(8))});
  unittest::ExpectEqual(dsvo::DataSet<COMPANYINFO_TYPE_LIST>, companyRecord5,
                                                              companyRecordSearch->second);
}