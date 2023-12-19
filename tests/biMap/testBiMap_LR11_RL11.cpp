
// test BiMap with map<Left,Right>::{1,1}  and map<Right,Left>::{1,1}

#include <string>
#include <iostream>

#include <biMap/BiMap.h>

#include "../unittest.h"


int main()
{
  int rv = 0;
  try {
    bimap::BiMap_LR11_RL11<int,std::string> indxName;

//////     tests for    insert_left

    //////////       new inserts

    std::pair<decltype(indxName)::const_lefttMapIterator, bool>
    insertResult = indxName.insert_left(2, "BBBB");
    unittest::ExpectTrue(insertResult.second);
    decltype(indxName)::const_lefttMapIterator
    insertIter = insertResult.first;
    unittest::ExpectEqual(int, 2, insertIter->first);
    unittest::ExpectEqual(std::string, "BBBB", insertIter->second);

    insertResult = indxName.insert_left(1, "AAAA");
    unittest::ExpectTrue(insertResult.second);
    insertIter = insertResult.first;
    unittest::ExpectEqual(int, 1, insertIter->first);
    unittest::ExpectEqual(std::string, "AAAA", insertIter->second);

    insertResult = indxName.insert_left(3, "CCCC");
    unittest::ExpectTrue(insertResult.second);
    insertIter = insertResult.first;
    unittest::ExpectEqual(int, 3, insertIter->first);
    unittest::ExpectEqual(std::string, "CCCC", insertIter->second);

    insertResult = indxName.insert_left(5, "EEEE");
    unittest::ExpectTrue(insertResult.second);
    insertIter = insertResult.first;
    unittest::ExpectEqual(int, 5, insertIter->first);
    unittest::ExpectEqual(std::string, "EEEE", insertIter->second);

    insertResult = indxName.insert_left(4, "DDDD");
    unittest::ExpectTrue(insertResult.second);
    insertIter = insertResult.first;
    unittest::ExpectEqual(int, 4, insertIter->first);
    unittest::ExpectEqual(std::string, "DDDD", insertIter->second);

    /////////      left-right  pair exists 

    insertResult = indxName.insert_left(2, "BBBB");
    unittest::ExpectTrue(!insertResult.second);
    insertIter = insertResult.first;
    unittest::ExpectEqual(int, 2, insertIter->first);
    unittest::ExpectEqual(std::string, "BBBB", insertIter->second);

    insertResult = indxName.insert_left(4, "DDDD");
    unittest::ExpectTrue(!insertResult.second);
    insertIter = insertResult.first;
    unittest::ExpectEqual(int, 4, insertIter->first);
    unittest::ExpectEqual(std::string, "DDDD", insertIter->second);

    /////////         left-exists : right-doesn't

    insertResult = indxName.insert_left(1, "XXXX");
    unittest::ExpectTrue(!insertResult.second);
    insertIter = insertResult.first;
    unittest::ExpectEqual(int, 1, insertIter->first);
    unittest::ExpectEqual(std::string, "AAAA", insertIter->second);

    insertResult = indxName.insert_left(5, "YYYY");
    unittest::ExpectTrue(!insertResult.second);
    insertIter = insertResult.first;
    unittest::ExpectEqual(int, 5, insertIter->first);
    unittest::ExpectEqual(std::string, "EEEE", insertIter->second);

    ////////     even though both left & right exits but not paired-together,
    ////////     then left-comparison takes precedence
    insertResult = indxName.insert_left(3, "AAAA");
    unittest::ExpectTrue(!insertResult.second);
    insertIter = insertResult.first;
    unittest::ExpectEqual(int, 3, insertIter->first);
    unittest::ExpectEqual(std::string, "CCCC", insertIter->second);

    /////////        left-doesn't  :  right-exists

    insertResult = indxName.insert_left(-1, "AAAA");
    unittest::ExpectTrue(!insertResult.second);
    insertIter = insertResult.first;
    unittest::ExpectEqual(int, 1, insertIter->first);
    unittest::ExpectEqual(std::string, "AAAA", insertIter->second);

    insertResult = indxName.insert_left(-5, "EEEE");
    unittest::ExpectTrue(!insertResult.second);
    insertIter = insertResult.first;
    unittest::ExpectEqual(int, 5, insertIter->first);
    unittest::ExpectEqual(std::string, "EEEE", insertIter->second);


//////     tests for    find_left  and  isLeft_end

    decltype(indxName)::const_lefttMapIterator
    searchResultL = indxName.find_left(1);
    unittest::ExpectEqual(int, 1, searchResultL->first);
    unittest::ExpectEqual(std::string, "AAAA", searchResultL->second);

    searchResultL = indxName.find_left(3);
    unittest::ExpectEqual(int, 3, searchResultL->first);
    unittest::ExpectEqual(std::string, "CCCC", searchResultL->second);

    searchResultL = indxName.find_left(5);
    unittest::ExpectEqual(int, 5, searchResultL->first);
    unittest::ExpectEqual(std::string, "EEEE", searchResultL->second);

//////     tests for    find_right  and  isRight_end

    searchResultL = indxName.find_left(0);
    unittest::ExpectTrue(indxName.isLeft_end(searchResultL));

    searchResultL = indxName.find_left(6);
    unittest::ExpectTrue(indxName.isLeft_end(searchResultL));


//////     tests for    find_right  and  isRight_end

    decltype(indxName)::const_rightMapIterator
    searchResultR = indxName.find_right("AAAA");
    unittest::ExpectEqual(std::string, "AAAA", searchResultR->first);
    unittest::ExpectEqual(int, 1, searchResultR->second);

    searchResultR = indxName.find_right("CCCC");
    unittest::ExpectEqual(std::string, "CCCC", searchResultR->first);
    unittest::ExpectEqual(int, 3, searchResultR->second);

    searchResultR = indxName.find_right("EEEE");
    unittest::ExpectEqual(std::string, "EEEE", searchResultR->first);
    unittest::ExpectEqual(int, 5, searchResultR->second);

//////     tests for    find_right  and  isRight_end

    searchResultR = indxName.find_right("aaaa");
    unittest::ExpectTrue(indxName.isRight_end(searchResultR));

    searchResultR = indxName.find_right("eeee");
    unittest::ExpectTrue(indxName.isRight_end(searchResultR));





  } catch (const std::exception& ex) {
    std::cout << ex.what() << std::endl;
    rv = 1;
  }

  return rv;
}