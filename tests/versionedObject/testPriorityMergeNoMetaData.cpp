
#include <testHelper.h>

#include <versionedObject/VersionedObjectPriorityMerge.h>


using t_versionObjectPriorityMerge = dsvo::VersionedObjectPriorityMerge<t_versionDate, COMPANYMETAINFO_TYPE_LIST>;


void loadVO(t_versionObject& voReload,
            bool firstRun)
{
  bool insertResult;

/* these rows are from symbolchange.csv
ANDHRA PAPER LIMITED,APPAPER,IPAPPM,21-JAN-2014
ANDHRA PAPER LIMITED,IPAPPM,ANDPAPER,22-JAN-2020
ANDHRA PAPER LIMITED,ANDPAPER,ANDHRAPAP,05-MAR-2020
*/

  std::string voStr =
    "13-May-2004," TEST_WITH_METADATA("*|*crown|*manualDeduction,") "APPAPER,International Paper APPM Limited,EQ,10,1,INE435A01028,10,LISTED\n"
    "21-Jan-2014," TEST_WITH_METADATA("+|%symbolchange,") "IPAPPM,International Paper APPM Limited,EQ,10,1,INE435A01028,10,LISTED\n"
    "22-Jan-2020," TEST_WITH_METADATA("+|%symbolchange|%namechange,") "ANDPAPER,ANDHRA PAPER LIMITED,EQ,10,1,INE435A01028,10,LISTED\n";


  if(firstRun) {
    unittest::ExpectEqual(std::string, std::string(""), t_versionObjectStream::createVOstreamer(voReload).toCSV(TEST_WITH_METADATA(dsvo::MetaDataSource::delimiter)));
  } else {
    unittest::ExpectEqual(std::string, voStr, t_versionObjectStream::createVOstreamer(voReload).toCSV(TEST_WITH_METADATA(dsvo::MetaDataSource::delimiter)));
  }

//  NOTE : the row below is manually deduced
//  APPAPER,International Paper APPM Limited,EQ,13-MAY-2004,10,1,INE435A01028,10
  t_companyInfo companyInfoStart = t_convertFromString::ToVal(
    "APPAPER,International Paper APPM Limited,EQ,10,1,INE435A01028,10,LISTED"    );

  TEST_WITH_METADATA(dsvo::MetaDataSource crownMeta{"crown" COMMA t_eDataBuild::IsRECORD COMMA t_eDataPatch::FullRECORD});
  t_dataSet companyRecordStartCrown {TEST_WITH_METADATA(crownMeta COMMA) companyInfoStart};

  const t_versionDate crownDate{std::chrono::year(int(2004)), std::chrono::May, std::chrono::day(unsigned(13))};
  t_versionObject voHighPriority;
  insertResult = voHighPriority.insertVersion(crownDate,
                                              companyRecordStartCrown);
  unittest::ExpectEqual(bool, true, insertResult);

  TEST_WITH_METADATA(dsvo::MetaDataSource manualMeta{"manualDeduction" COMMA t_eDataBuild::IsRECORD COMMA t_eDataPatch::FullRECORD});
  t_dataSet companyRecordStartManual {TEST_WITH_METADATA(manualMeta COMMA) companyInfoStart};

  t_versionObject voLowrPriority;
  insertResult = voLowrPriority.insertVersion(crownDate,
                                              companyRecordStartManual);
  unittest::ExpectEqual(bool, true, insertResult);


  TEST_WITH_METADATA(dsvo::MetaDataSource expectedStartMergeMeta{"crown" COMMA t_eDataBuild::IsRECORD COMMA t_eDataPatch::FullRECORD});
  TEST_WITH_METADATA(expectedStartMergeMeta.merge(dsvo::MetaDataSource{"manualDeduction" COMMA t_eDataBuild::IsRECORD COMMA t_eDataPatch::FullRECORD}));
  t_dataSet companyStartRecordExpected { TEST_WITH_METADATA(expectedStartMergeMeta COMMA) companyInfoStart};

  t_versionObject voExpected;
  static t_versionObject voExpected2; // NOTE :  on 2nd run, voExpected2 has data of the 1st run.

  insertResult = voExpected.insertVersion(crownDate,
                                          companyStartRecordExpected);
  unittest::ExpectEqual(bool, true, insertResult);
  insertResult = voExpected2.insertVersion(crownDate,
                                           companyStartRecordExpected);
  unittest::ExpectEqual(bool, firstRun, insertResult);
  {
    t_versionObjectPriorityMerge vopm{voHighPriority,voLowrPriority};
    t_versionObject voMerged;
    vopm.getMergeResult(voMerged);
    unittest::ExpectEqual(t_versionObject, voExpected, voMerged);

    vopm.getMergeResult(voReload);
    unittest::ExpectEqual(t_versionObject, voExpected2, voReload);

    typename t_versionObject::t_datasetLedger::const_iterator companyRecordMerged = voMerged.getVersionAt(crownDate);
    unittest::ExpectEqual(bool, true, companyRecordMerged != voMerged.getDatasetLedger().cend()); // has t_dataSet

    unittest::ExpectEqual(t_dataSet, companyStartRecordExpected,
                                     companyRecordMerged->second);

#if TEST_ENABLE_METADATA == 1
    unittest::ExpectEqual(dsvo::MetaDataSource, expectedStartMergeMeta, companyRecordMerged->second.getMetaData());
#endif
  }

//  ANDHRA PAPER LIMITED,APPAPER,IPAPPM,21-JAN-2014
  t_companyInfo companyInfoSecond = t_convertFromString::ToVal(
    "IPAPPM,International Paper APPM Limited,EQ,10,1,INE435A01028,10,LISTED"    );

  TEST_WITH_METADATA(dsvo::MetaDataSource symChgMetaExp{"symbolchange" COMMA t_eDataBuild::FORWARD COMMA t_eDataPatch::DELTACHANGE});
  t_dataSet companyRecordSecondExpected { TEST_WITH_METADATA(symChgMetaExp COMMA) companyInfoSecond};

  const t_versionDate symChgDate{std::chrono::year(int(2014)), std::chrono::January, std::chrono::day(unsigned(21))};
  insertResult = voHighPriority.insertVersion(symChgDate,
                                              companyRecordSecondExpected);
  unittest::ExpectEqual(bool, true, insertResult);
  insertResult = voExpected.insertVersion(symChgDate,
                                          companyRecordSecondExpected);
  unittest::ExpectEqual(bool, true, insertResult);
  insertResult = voExpected2.insertVersion(symChgDate,
                                           companyRecordSecondExpected);
  unittest::ExpectEqual(bool, firstRun, insertResult);
  {
    t_versionObjectPriorityMerge vopm{voHighPriority,voLowrPriority};
    t_versionObject voMerged;
    vopm.getMergeResult(voMerged);
    unittest::ExpectEqual(t_versionObject, voExpected, voMerged);

    vopm.getMergeResult(voReload);
    unittest::ExpectEqual(t_versionObject, voExpected2, voReload);

    typename t_versionObject::t_datasetLedger::const_iterator companyRecordMerged = voMerged.getVersionAt(symChgDate);
    unittest::ExpectEqual(bool, true, companyRecordMerged != voMerged.getDatasetLedger().cend()); // has t_dataSet

    unittest::ExpectEqual(t_dataSet, companyRecordSecondExpected,
                                     companyRecordMerged->second);

#if TEST_ENABLE_METADATA == 1
    unittest::ExpectEqual(dsvo::MetaDataSource, symChgMetaExp, companyRecordMerged->second.getMetaData());
#endif
  }

//  ANDHRA PAPER LIMITED,IPAPPM,ANDPAPER,22-JAN-2020
//  ANDHRAPAP,International Paper APPM Limited,ANDHRA PAPER LIMITED,22-JAN-2020
  t_companyInfo companyInfoThird = t_convertFromString::ToVal(
    "ANDPAPER,ANDHRA PAPER LIMITED,EQ,10,1,INE435A01028,10,LISTED"    );

  TEST_WITH_METADATA(dsvo::MetaDataSource symChgNamChgMetaExp{"symbolchange" COMMA t_eDataBuild::FORWARD COMMA t_eDataPatch::DELTACHANGE});
  TEST_WITH_METADATA(symChgNamChgMetaExp.merge(dsvo::MetaDataSource{"namechange" COMMA t_eDataBuild::FORWARD COMMA t_eDataPatch::DELTACHANGE}));
  t_dataSet companyRecordThirdExpected { TEST_WITH_METADATA(symChgNamChgMetaExp COMMA) companyInfoThird};

  const t_versionDate symChgNameChgDate{std::chrono::year(int(2020)), std::chrono::January, std::chrono::day(unsigned(22))};
  insertResult = voLowrPriority.insertVersion(symChgNameChgDate,
                                              companyRecordThirdExpected);
  unittest::ExpectEqual(bool, true, insertResult);

  insertResult = voExpected.insertVersion(symChgNameChgDate,
                                          companyRecordThirdExpected);
  unittest::ExpectEqual(bool, true, insertResult);
  insertResult = voExpected2.insertVersion(symChgNameChgDate,
                                           companyRecordThirdExpected);
  unittest::ExpectEqual(bool, firstRun, insertResult);
  {
    t_versionObjectPriorityMerge vopm{voHighPriority,voLowrPriority};
    t_versionObject voMerged;
    vopm.getMergeResult(voMerged);
    unittest::ExpectEqual(t_versionObject, voExpected, voMerged);

    vopm.getMergeResult(voReload);
    unittest::ExpectEqual(t_versionObject, voExpected2, voReload);

    typename t_versionObject::t_datasetLedger::const_iterator companyRecordMerged = voMerged.getVersionAt(symChgNameChgDate);
    unittest::ExpectEqual(bool, true, companyRecordMerged != voMerged.getDatasetLedger().cend()); // has t_dataSet

    unittest::ExpectEqual(t_dataSet, companyRecordThirdExpected,
                                     companyRecordMerged->second);

#if TEST_ENABLE_METADATA == 1
    unittest::ExpectEqual(dsvo::MetaDataSource, symChgNamChgMetaExp, companyRecordMerged->second.getMetaData());
#endif
  }

  unittest::ExpectEqual(std::string, voStr, t_versionObjectStream::createVOstreamer(voReload).toCSV(TEST_WITH_METADATA(dsvo::MetaDataSource::delimiter)));

//  ANDHRA PAPER LIMITED,ANDPAPER,ANDHRAPAP,05-MAR-2020
//  NOTE: the row below is not a versioned information, but CROWN info from EQUITY_L.csv
//       ANDHRAPAP,ANDHRA PAPER LIMITED,EQ,13-MAY-2004,10,1,INE435A01028,10
  t_companyInfo companyInfoLatest1 = t_convertFromString::ToVal(
    "ANDHRAPAP,ANDHRA PAPER LIMITED:1,EQ,10,1,INE435A01028,10,LISTED"    );

  const t_versionDate crownDate2{std::chrono::year(int(2020)), std::chrono::March, std::chrono::day(unsigned(5))};
  t_dataSet companyRecordLatestExpected1 { TEST_WITH_METADATA(symChgMetaExp COMMA) companyInfoLatest1};

  insertResult = voLowrPriority.insertVersion(crownDate2,
                                              companyRecordLatestExpected1);
  unittest::ExpectEqual(bool, true, insertResult);

  t_companyInfo companyInfoLatest2 = t_convertFromString::ToVal(
    "ANDHRAPAP,ANDHRA PAPER LIMITED:2,EQ,10,1,INE435A01028,10,LISTED"    );

  t_dataSet companyRecordLatestExpected2 {TEST_WITH_METADATA(symChgMetaExp COMMA) companyInfoLatest2};

  insertResult = voHighPriority.insertVersion(crownDate2,
                                              companyRecordLatestExpected2);
  unittest::ExpectEqual(bool, true, insertResult);
  {
    t_versionObjectPriorityMerge vopm{voHighPriority,voLowrPriority};

    t_versionObject voMerged;

    ExpectExceptionMsg( vopm.getMergeResult(voMerged), dsvo::VOPM_Record_Mismatch_exception, \
    "ERROR : failure in VersionedObjectPriorityMerge<VDT, MT...>::getMergeResult() : different 'record' exits " \
    "between 2 merge-candidates of VersionedObject");

    std::cout << "failure_test_end_1" << std::endl;

    ExpectExceptionMsg( vopm.getMergeResult(voReload), dsvo::VOPM_Record_Mismatch_exception, \
    "ERROR : failure in VersionedObjectPriorityMerge<VDT, MT...>::getMergeResult() : different 'record' exits " \
    "between 2 merge-candidates of VersionedObject");

    std::cout << "failure_test_end_2" << std::endl;

  }
}
