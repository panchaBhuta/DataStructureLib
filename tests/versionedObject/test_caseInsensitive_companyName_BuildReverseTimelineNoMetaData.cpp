
#include <testHelper.h>


void loadVO(t_versionObject& vo,
            bool insertResultExpected)
{
  dsvo::VersionedObjectBuilder<t_fmtdbY, COMPANYINFO_TYPE_LIST> vob;

  bool insertResult;

/* these rows are from symbolchange.csv
A2Z Infra Engineering Limited,A2ZMES,A2ZINFRA,31-DEC-2014
*/
  const std::array <bool, std::tuple_size_v<t_companyInfo> > symbolChangeFlg = {true, false, false, false, false, false, false};

  t_companyInfo symChgOldInfo1 = converter::ConvertFromString<COMPANYINFO_TYPE_LIST>::ToVal("A2ZMES,,,0,0,,0");
  t_companyInfo symChgNewInfo1 = converter::ConvertFromString<COMPANYINFO_TYPE_LIST>::ToVal("A2ZINFRA,,,0,0,,0");
  dsvo::ChangesInDataSet<COMPANYINFO_TYPE_LIST> symbolChange1 {symbolChangeFlg, symChgOldInfo1, symChgNewInfo1, dsvo::ApplicableChangeDirection::REVERSE};
  insertResult = vob.insertDeltaVersion(t_listingDate(std::chrono::year(int(2014)), std::chrono::December, std::chrono::day(unsigned(31))), symbolChange1);
  unittest::ExpectEqual(bool, true, insertResult);



  dsvo::MetaDataSource namChgMeta("namechange",'-');
  const std::array <bool, std::tuple_size_v<t_companyInfo> > nameChangeFlg = {false, true, false, false, false, false, false};

//  NOTE : the row below is from namechange.csv
//  A2ZINFRA,A2Z Maintenance & Engineering Services Limited,A2Z INFRA ENGINEERING LIMITED,31-DEC-2014
  t_companyInfo namChgOldInfo1 = converter::ConvertFromString<COMPANYINFO_TYPE_LIST>::ToVal(",A2Z Maintenance & Engineering Services Limited,,0,0,,0");
  t_companyInfo namChgNewInfo1 = converter::ConvertFromString<COMPANYINFO_TYPE_LIST>::ToVal(",A2Z INFRA ENGINEERING LIMITED,,0,0,,0");  //  TEST-NOTE : Company-name in upper-case
  dsvo::ChangesInDataSet<COMPANYINFO_TYPE_LIST> nameChange1 {nameChangeFlg, namChgOldInfo1, namChgNewInfo1, dsvo::ApplicableChangeDirection::REVERSE};
  insertResult = vob.insertDeltaVersion(t_listingDate(std::chrono::year(int(2014)), std::chrono::December, std::chrono::day(unsigned(31))), nameChange1);
  unittest::ExpectEqual(bool, false, insertResult);



//  NOTE : the row below is not a versioned information, but info from EQUITY_L.csv
//       A2ZINFRA,A2Z Infra Engineering Limited,BE,23-DEC-2010,10,1,INE619I01012,10,LISTED
  t_companyInfo companyInfoLatest = converter::ConvertFromString<COMPANYINFO_TYPE_LIST>::ToVal(
    "A2ZINFRA,A2Z Infra Engineering Limited,BE,10,1,INE619I01012,10"    );  //  TEST-NOTE : Company-name in lower-case

  dsvo::DataSet<COMPANYINFO_TYPE_LIST> companyRecordLatestExpected {companyInfoLatest};

  insertResult = vo.insertVersion(t_listingDate(std::chrono::year(int(2014)), std::chrono::December, std::chrono::day(unsigned(31))),
                                  companyRecordLatestExpected);
  unittest::ExpectEqual(bool, insertResultExpected, insertResult);
  vob.buildReverseTimeline( t_listingDate(std::chrono::year(int(2010)), std::chrono::December, std::chrono::day(unsigned(23))),
                            vo);


//#################### check all versioned objects

//  NOTE : the row below is manually deduced
//  A2Z Infra Engineering Limited,A2ZMES,A2ZINFRA,31-DEC-2014
//  A2ZINFRA,A2Z Maintenance & Engineering Services Limited,A2Z INFRA ENGINEERING LIMITED,31-DEC-2014

//  A2ZMES,A2Z Maintenance & Engineering Services Limited,BE,23-DEC-2010,10,1,INE619I01012,10
  t_companyInfo companyInfoStart = converter::ConvertFromString<COMPANYINFO_TYPE_LIST>::ToVal(
    "A2ZMES,A2Z Maintenance & Engineering Services Limited,BE,10,1,INE619I01012,10"    );

  dsvo::DataSet<COMPANYINFO_TYPE_LIST> companyRecordStart {companyInfoStart};

  t_versionObject::t_datasetLedger::const_iterator companyRecordFirstActual =
    vo.getVersionAt(t_listingDate(std::chrono::year(int(2010)), std::chrono::December, std::chrono::day(unsigned(23))));

  unittest::ExpectEqual(bool, true, companyRecordFirstActual != vo.getDatasetLedger().cend()); // has dsvo::DataSet<COMPANYINFO_TYPE_LIST>

  unittest::ExpectEqual(dsvo::DataSet<COMPANYINFO_TYPE_LIST>, companyRecordStart,
                                                              companyRecordFirstActual->second);



//  NOTE: the row below is not a versioned information, but info from EQUITY_L.csv
//  A2ZINFRA,A2Z Infra Engineering Limited,BE,31-DEC-2014,10,1,INE619I01012,10,LISTED
  t_versionObject::t_datasetLedger::const_iterator companyRecordLatestActual =
    vo.getVersionAt(t_listingDate(std::chrono::year(int(2014)), std::chrono::December, std::chrono::day(unsigned(31))));

  unittest::ExpectEqual(bool, true, companyRecordLatestActual != vo.getDatasetLedger().cend()); // has dsvo::DataSet<COMPANYINFO_TYPE_LIST>

  unittest::ExpectEqual(dsvo::DataSet<COMPANYINFO_TYPE_LIST>, companyRecordLatestExpected,
                                                              companyRecordLatestActual->second);

}
