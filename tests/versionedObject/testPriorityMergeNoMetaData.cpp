
#include <testHelper.h>

#include <versionedObject/VersionedObjectPriorityMerge.h>


namespace unittest
{
  template<>
  struct SScompatible<t_versionObject> {
    inline static std::string getVal(const t_versionObject& val)
    {
      return val.toCSV();
    }
  };
}

void loadVO(t_versionObject& vo,
            bool firstRun)
{
  bool insertResult;

/* these rows are from symbolchange.csv
ANDHRA PAPER LIMITED,APPAPER,IPAPPM,21-JAN-2014
ANDHRA PAPER LIMITED,IPAPPM,ANDPAPER,22-JAN-2020
ANDHRA PAPER LIMITED,ANDPAPER,ANDHRAPAP,05-MAR-2020
*/

//  NOTE : the row below is manually deduced
//  APPAPER,International Paper APPM Limited,EQ,13-MAY-2004,10,1,INE435A01028,10
  t_companyInfo companyInfoStart = t_convertFromString::ToVal(
    "APPAPER,International Paper APPM Limited,EQ,10,1,INE435A01028,10"    );

  dsvo::DataSet<COMPANYINFO_TYPE_LIST> companyRecordStart {companyInfoStart};

  const t_versionDate crownDate{std::chrono::year(int(2004)), std::chrono::May, std::chrono::day(unsigned(13))};
  t_versionObject voHighPriority;
  insertResult = voHighPriority.insertVersion(crownDate,
                                              companyRecordStart);
  unittest::ExpectEqual(bool, true, insertResult);

  t_versionObject voLowrPriority;
  insertResult = voLowrPriority.insertVersion(crownDate,
                                              companyRecordStart);
  unittest::ExpectEqual(bool, true, insertResult);


  t_versionObject voExpected;
  static t_versionObject voExpected2; // NOTE :  on 2nd run, voExpected2 has data of the 1st run.
  insertResult = voExpected.insertVersion(crownDate,
                                          companyRecordStart);
  unittest::ExpectEqual(bool, true, insertResult);
  insertResult = voExpected2.insertVersion(crownDate,
                                           companyRecordStart);
  unittest::ExpectEqual(bool, firstRun, insertResult);
  {
    dsvo::VersionedObjectPriorityMerge<t_fmtdbY, COMPANYINFO_TYPE_LIST> vopm{voHighPriority,voLowrPriority};
    t_versionObject voMerged;
    vopm.getMergeResult(voMerged);
    unittest::ExpectEqual(t_versionObject, voExpected, voMerged);

    vopm.getMergeResult(vo);
    unittest::ExpectEqual(t_versionObject, voExpected2, vo);
  }

//  ANDHRA PAPER LIMITED,APPAPER,IPAPPM,21-JAN-2014
  t_companyInfo companyInfoSecond = t_convertFromString::ToVal(
    "IPAPPM,International Paper APPM Limited,EQ,10,1,INE435A01028,10"    );

  dsvo::DataSet<COMPANYINFO_TYPE_LIST> companyRecordSecondExpected {companyInfoSecond};

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
    dsvo::VersionedObjectPriorityMerge<t_fmtdbY, COMPANYINFO_TYPE_LIST> vopm{voHighPriority,voLowrPriority};
    t_versionObject voMerged;
    vopm.getMergeResult(voMerged);
    unittest::ExpectEqual(t_versionObject, voExpected, voMerged);

    vopm.getMergeResult(vo);
    unittest::ExpectEqual(t_versionObject, voExpected2, vo);
  }

//  ANDHRA PAPER LIMITED,IPAPPM,ANDPAPER,22-JAN-2020
//  ANDHRAPAP,International Paper APPM Limited,ANDHRA PAPER LIMITED,22-JAN-2020
  t_companyInfo companyInfoThird = t_convertFromString::ToVal(
    "ANDPAPER,ANDHRA PAPER LIMITED,EQ,10,1,INE435A01028,10"    );

  dsvo::DataSet<COMPANYINFO_TYPE_LIST> companyRecordThirdExpected {companyInfoThird};

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
    dsvo::VersionedObjectPriorityMerge<t_fmtdbY, COMPANYINFO_TYPE_LIST> vopm{voHighPriority,voLowrPriority};
    t_versionObject voMerged;
    vopm.getMergeResult(voMerged);
    unittest::ExpectEqual(t_versionObject, voExpected, voMerged);

    vopm.getMergeResult(vo);
    unittest::ExpectEqual(t_versionObject, voExpected2, vo);
  }


//  ANDHRA PAPER LIMITED,ANDPAPER,ANDHRAPAP,05-MAR-2020
//  NOTE: the row below is not a versioned information, but CROWN info from EQUITY_L.csv
//       ANDHRAPAP,ANDHRA PAPER LIMITED,EQ,13-MAY-2004,10,1,INE435A01028,10
  t_companyInfo companyInfoLatest1 = t_convertFromString::ToVal(
    "ANDHRAPAP,ANDHRA PAPER LIMITED:1,EQ,10,1,INE435A01028,10"    );

  const t_versionDate crownDate2{std::chrono::year(int(2020)), std::chrono::March, std::chrono::day(unsigned(5))};
  dsvo::DataSet<COMPANYINFO_TYPE_LIST> companyRecordLatestExpected1 {companyInfoLatest1};

  insertResult = voLowrPriority.insertVersion(crownDate2,
                                              companyRecordLatestExpected1);
  unittest::ExpectEqual(bool, true, insertResult);

  t_companyInfo companyInfoLatest2 = t_convertFromString::ToVal(
    "ANDHRAPAP,ANDHRA PAPER LIMITED:2,EQ,10,1,INE435A01028,10"    );

  dsvo::DataSet<COMPANYINFO_TYPE_LIST> companyRecordLatestExpected2 {companyInfoLatest2};

  insertResult = voHighPriority.insertVersion(crownDate2,
                                              companyRecordLatestExpected2);
  unittest::ExpectEqual(bool, true, insertResult);
  {
    dsvo::VersionedObjectPriorityMerge<t_fmtdbY, COMPANYINFO_TYPE_LIST> vopm{voHighPriority,voLowrPriority};

    t_versionObject voMerged;

    ExpectExceptionMsg( vopm.getMergeResult(voMerged), dsvo::VOPM_Record_Mismatch_exception, \
    "ERROR : failure in VersionedObjectPriorityMerge<VDT, MT...>::getMergeResult() : different 'record' exits " \
    "between 2 merge-candidates of VersionedObject");

    std::cout << "failure_test_end_1" << std::endl;

    ExpectExceptionMsg( vopm.getMergeResult(vo), dsvo::VOPM_Record_Mismatch_exception, \
    "ERROR : failure in VersionedObjectPriorityMerge<VDT, MT...>::getMergeResult() : different 'record' exits " \
    "between 2 merge-candidates of VersionedObject");

    std::cout << "failure_test_end_2" << std::endl;

  }
}
