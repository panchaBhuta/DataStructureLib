
// test default conversions for inbuilt-types


#include <iostream>

#include <converter/specializedTypes/date.h>
#include <converter/specializedTypes/case_insensitive_string.h>

#include <versionedObject/VersionedObject.h>
#include <versionedObject/VersionedObjectBuilder.h>

#include "../unittest.h"

using t_fmtdbY = converter::format_year_month_day<converter::dbY_fmt, converter::FailureS2Tprocess::THROW_ERROR>;


/*
masg_EQUITY_L.csv:A2ZINFRA,A2Z Infra Engineering Limited,BE,23-DEC-2010,10,1,INE619I01012,10,LISTED
masg_namechange.csv:A2ZINFRA,A2Z Maintenance & Engineering Services Limited,A2Z INFRA ENGINEERING LIMITED,31-DEC-2014
masg_symbolchange.csv:A2Z Infra Engineering Limited,A2ZMES,A2ZINFRA,31-DEC-2014

NOTE: company name is changes from upper to lower case in both files : masg_namechange.csv | masg_symbolchange.csv  } 
*/

using t_symbol      = std::string;
using t_companyName = converter::ci_string;
using t_series      = std::string;
using t_listingDate = t_fmtdbY; // std::chrono::year_month_day;
using t_paidUpValue = uint16_t;
using t_marketLot   = uint16_t;
using t_isinNumber  = std::string;
using t_faceValue   = uint16_t;

#define COMPANYINFO_TYPE_LIST t_symbol, t_companyName, t_series,     \
                              t_paidUpValue, t_marketLot, t_isinNumber, t_faceValue
using t_companyInfo = std::tuple<COMPANYINFO_TYPE_LIST>;

namespace dsvo = datastructure::versionedObject;



namespace unittest
{
  template<>
  struct SScompatible<dsvo::DataSet<COMPANYINFO_TYPE_LIST>> {
    inline static std::string getVal(const dsvo::DataSet<COMPANYINFO_TYPE_LIST>& val)
    {
      return val.toCSV();
    }
  };
}

int main()
{
  int rv = 0;
  try {
    dsvo::VersionedObjectBuilder<t_fmtdbY, COMPANYINFO_TYPE_LIST> vob;

    bool insertResult;

/* these rows are from symbolchange.csv
A2Z Infra Engineering Limited,A2ZMES,A2ZINFRA,31-DEC-2014
*/
    const std::array <bool, std::tuple_size_v<t_companyInfo> > symbolChangeFlg = {true, false, false, false, false, false, false};

    t_companyInfo symChgOldInfo1 = converter::ConvertFromString<COMPANYINFO_TYPE_LIST>::ToVal("A2ZMES,,,0,0,,0");
    t_companyInfo symChgNewInfo1 = converter::ConvertFromString<COMPANYINFO_TYPE_LIST>::ToVal("A2ZINFRA,,,0,0,,0");
    dsvo::ChangesInDataSet<COMPANYINFO_TYPE_LIST> symbolChange1 {symbolChangeFlg, symChgOldInfo1, symChgNewInfo1};
    insertResult = vob.insertDeltaVersion(t_listingDate(std::chrono::year(int(2014)), std::chrono::December, std::chrono::day(unsigned(31))), symbolChange1);
    unittest::ExpectEqual(bool, true, insertResult);



    dsvo::MetaDataSource namChgMeta("namechange",'-');
    const std::array <bool, std::tuple_size_v<t_companyInfo> > nameChangeFlg = {false, true, false, false, false, false, false};

//  NOTE : the row below is from namechange.csv
//  A2ZINFRA,A2Z Maintenance & Engineering Services Limited,A2Z INFRA ENGINEERING LIMITED,31-DEC-2014
    t_companyInfo namChgOldInfo1 = converter::ConvertFromString<COMPANYINFO_TYPE_LIST>::ToVal(",A2Z Maintenance & Engineering Services Limited,,0,0,,0");
    t_companyInfo namChgNewInfo1 = converter::ConvertFromString<COMPANYINFO_TYPE_LIST>::ToVal(",A2Z INFRA ENGINEERING LIMITED,,0,0,,0");
    dsvo::ChangesInDataSet<COMPANYINFO_TYPE_LIST> nameChange1 {nameChangeFlg, namChgOldInfo1, namChgNewInfo1};
    insertResult = vob.insertDeltaVersion(t_listingDate(std::chrono::year(int(2014)), std::chrono::December, std::chrono::day(unsigned(31))), nameChange1);
    unittest::ExpectEqual(bool, true, insertResult);



//  NOTE : the row below is not a versioned information, but info from EQUITY_L.csv
//       A2ZINFRA,A2Z Infra Engineering Limited,BE,23-DEC-2010,10,1,INE619I01012,10,LISTED
    t_companyInfo companyInfoLatest = converter::ConvertFromString<COMPANYINFO_TYPE_LIST>::ToVal(
      "A2ZINFRA,A2Z Infra Engineering Limited,BE,10,1,INE619I01012,10"    );

    dsvo::DataSet<COMPANYINFO_TYPE_LIST> companyRecordLatestExpected {companyInfoLatest};

    dsvo::VersionedObject<t_fmtdbY, COMPANYINFO_TYPE_LIST> vo
                  = vob.buildReverseTimeline(t_listingDate(std::chrono::year(int(2010)), std::chrono::December, std::chrono::day(unsigned(23))),
                                             companyRecordLatestExpected);


//#################### check all versioned objects

//  NOTE : the row below is manually deduced
//  A2Z Infra Engineering Limited,A2ZMES,A2ZINFRA,31-DEC-2014
//  A2ZINFRA,A2Z Maintenance & Engineering Services Limited,A2Z INFRA ENGINEERING LIMITED,31-DEC-2014

//  A2ZMES,A2Z Maintenance & Engineering Services Limited,BE,23-DEC-2010,10,1,INE619I01012,10
    t_companyInfo companyInfoStart = converter::ConvertFromString<COMPANYINFO_TYPE_LIST>::ToVal(
      "A2ZMES,A2Z Maintenance & Engineering Services Limited,BE,10,1,INE619I01012,10"    );

    dsvo::DataSet<COMPANYINFO_TYPE_LIST> companyRecordStart {companyInfoStart};

    std::optional<dsvo::DataSet<COMPANYINFO_TYPE_LIST>> companyRecordFirstActual =
      vo.getVersionAt(t_listingDate(std::chrono::year(int(2010)), std::chrono::December, std::chrono::day(unsigned(23))));

    unittest::ExpectEqual(bool, true, companyRecordFirstActual.has_value()); // has dsvo::DataSet<COMPANYINFO_TYPE_LIST>

    unittest::ExpectEqual(dsvo::DataSet<COMPANYINFO_TYPE_LIST>, companyRecordStart,
                                                                companyRecordFirstActual.value());



//  NOTE: the row below is not a versioned information, but info from EQUITY_L.csv
//  A2ZINFRA,A2Z Infra Engineering Limited,BE,31-DEC-2014,10,1,INE619I01012,10,LISTED
    std::optional<dsvo::DataSet<COMPANYINFO_TYPE_LIST>> companyRecordLatestActual =
      vo.getVersionAt(t_listingDate(std::chrono::year(int(2014)), std::chrono::December, std::chrono::day(unsigned(31))));

    unittest::ExpectEqual(bool, true, companyRecordLatestActual.has_value()); // has dsvo::DataSet<COMPANYINFO_TYPE_LIST>

    unittest::ExpectEqual(dsvo::DataSet<COMPANYINFO_TYPE_LIST>, companyRecordLatestExpected,
                                                                companyRecordLatestActual.value());

  } catch (const std::exception& ex) {
    std::cout << ex.what() << std::endl;
    rv = 1;
  }

  return rv;
}
