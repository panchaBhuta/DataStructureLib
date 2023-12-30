
// test BiMap with map<Left,Right>::{1,1}  and map<Right,Left>::{1,1}

#include <string>
#include <iostream>

#include <biMap/BiMap.h>

#include "../unittest.h"

namespace dsbm = datastructure::bimap;


int main()
{
  int rv = 0;
  try {
    dsbm::BiMap_LR1M_RL11<int,std::string> indxNames;

//////     tests for    insert_left

    //////////       new inserts

    std::pair<decltype(indxNames)::const_lefttMapIterator, bool>
    insertResult = indxNames.insert_left(2, "BBBB1");
    unittest::ExpectTrue(insertResult.second);
    decltype(indxNames)::const_lefttMapIterator
    insertIter = insertResult.first;
    unittest::ExpectEqual(int, 2, insertIter->first);
    std::set<std::string> checkSet2;  checkSet2.insert("BBBB1");
    unittest::ExpectEqual(std::set<std::string>, checkSet2, insertIter->second);
    unittest::ExpectTrue(indxNames.size_left() == 1);
    unittest::ExpectTrue(indxNames.size_right() == 1);

    insertResult = indxNames.insert_left(1, "AAAA1");
    unittest::ExpectTrue(insertResult.second);
    insertIter = insertResult.first;
    unittest::ExpectEqual(int, 1, insertIter->first);
    std::set<std::string> checkSet1;  checkSet1.insert("AAAA1");
    unittest::ExpectEqual(std::set<std::string>, checkSet1, insertIter->second);
    unittest::ExpectTrue(indxNames.size_left() == 2);
    unittest::ExpectTrue(indxNames.size_right() == 2);

    insertResult = indxNames.insert_left(3, "CCCC1");
    unittest::ExpectTrue(insertResult.second);
    insertIter = insertResult.first;
    unittest::ExpectEqual(int, 3, insertIter->first);
    std::set<std::string> checkSet3;  checkSet3.insert("CCCC1");
    unittest::ExpectEqual(std::set<std::string>, checkSet3, insertIter->second);
    unittest::ExpectTrue(indxNames.size_left() == 3);
    unittest::ExpectTrue(indxNames.size_right() == 3);

    insertResult = indxNames.insert_left(5, "EEEE1");
    unittest::ExpectTrue(insertResult.second);
    insertIter = insertResult.first;
    unittest::ExpectEqual(int, 5, insertIter->first);
    std::set<std::string> checkSet5;  checkSet5.insert("EEEE1");
    unittest::ExpectEqual(std::set<std::string>, checkSet5, insertIter->second);
    unittest::ExpectTrue(indxNames.size_left() == 4);
    unittest::ExpectTrue(indxNames.size_right() == 4);

    insertResult = indxNames.insert_left(4, "DDDD1");
    unittest::ExpectTrue(insertResult.second);
    insertIter = insertResult.first;
    unittest::ExpectEqual(int, 4, insertIter->first);
    std::set<std::string> checkSet4;  checkSet4.insert("DDDD1");
    unittest::ExpectEqual(std::set<std::string>, checkSet4, insertIter->second);
    unittest::ExpectTrue(indxNames.size_left() == 5);
    unittest::ExpectTrue(indxNames.size_right() == 5);

    /////////      left-right  pair exists 

    insertResult = indxNames.insert_left(2, "BBBB1");
    unittest::ExpectTrue(!insertResult.second);
    insertIter = insertResult.first;
    unittest::ExpectEqual(int, 2, insertIter->first);
    unittest::ExpectEqual(std::set<std::string>, checkSet2, insertIter->second);

    insertResult = indxNames.insert_left(4, "DDDD1");
    unittest::ExpectTrue(!insertResult.second);
    insertIter = insertResult.first;
    unittest::ExpectEqual(int, 4, insertIter->first);
    unittest::ExpectEqual(std::set<std::string>, checkSet4, insertIter->second);

    /////////         left-exists : right-doesn't

    insertResult = indxNames.insert_left(1, "AAAA2");
    unittest::ExpectTrue(insertResult.second);
    insertIter = insertResult.first;
    unittest::ExpectEqual(int, 1, insertIter->first);
    checkSet1.insert("AAAA2");
    unittest::ExpectEqual(std::set<std::string>, checkSet1, insertIter->second);
    unittest::ExpectTrue(indxNames.size_left() == 5);
    unittest::ExpectTrue(indxNames.size_right() == 6);
    unittest::ExpectTrue(insertIter->second.size() == 2);

    insertResult = indxNames.insert_left(5, "EEEE2");
    unittest::ExpectTrue(insertResult.second);
    insertIter = insertResult.first;
    unittest::ExpectEqual(int, 5, insertIter->first);
    checkSet5.insert("EEEE2");
    unittest::ExpectEqual(std::set<std::string>, checkSet5, insertIter->second);
    unittest::ExpectTrue(indxNames.size_left() == 5);
    unittest::ExpectTrue(indxNames.size_right() == 7);
    unittest::ExpectTrue(insertIter->second.size() == 2);

    ////////     even though both left & right exits but not paired-together
    insertResult = indxNames.insert_left(3, "AAAA1");
    unittest::ExpectTrue(!insertResult.second);
    insertIter = insertResult.first;
    unittest::ExpectEqual(int, 1, insertIter->first);
    unittest::ExpectEqual(std::set<std::string>, checkSet1, insertIter->second);

    /////////        left-doesn't  :  right-exists (right is mapped to different-existing-left)

    insertResult = indxNames.insert_left(-1, "AAAA2");
    unittest::ExpectTrue(!insertResult.second);
    insertIter = insertResult.first;
    unittest::ExpectEqual(int, 1, insertIter->first);
    unittest::ExpectEqual(std::set<std::string>, checkSet1, insertIter->second);

    insertResult = indxNames.insert_left(-5, "EEEE1");
    unittest::ExpectTrue(!insertResult.second);
    insertIter = insertResult.first;
    unittest::ExpectEqual(int, 5, insertIter->first);
    unittest::ExpectEqual(std::set<std::string>, checkSet5, insertIter->second);

//////     tests for    find_left

    decltype(indxNames)::const_lefttMapIterator
    searchResultL = indxNames.find_left(1);
    unittest::ExpectEqual(int, 1, searchResultL->first);
    unittest::ExpectEqual(std::set<std::string>, checkSet1, searchResultL->second);
    unittest::ExpectTrue(!indxNames.isLeft_end(searchResultL));

    searchResultL = indxNames.find_left(3);
    unittest::ExpectEqual(int, 3, searchResultL->first);
    unittest::ExpectEqual(std::set<std::string>, checkSet3, searchResultL->second);
    unittest::ExpectTrue(!indxNames.isLeft_end(searchResultL));

    searchResultL = indxNames.find_left(5);
    unittest::ExpectEqual(int, 5, searchResultL->first);
    unittest::ExpectEqual(std::set<std::string>, checkSet5, searchResultL->second);
    unittest::ExpectTrue(!indxNames.isLeft_end(searchResultL));

//////     tests for    isLeft_end

    searchResultL = indxNames.find_left(0);
    unittest::ExpectTrue(indxNames.isLeft_end(searchResultL));

    searchResultL = indxNames.find_left(6);
    unittest::ExpectTrue(indxNames.isLeft_end(searchResultL));


//////     tests for    find_right

    decltype(indxNames)::const_rightMapIterator
    searchResultR = indxNames.find_right("AAAA1");
    unittest::ExpectEqual(std::string, "AAAA1", searchResultR->first);
    unittest::ExpectEqual(int, 1, searchResultR->second);

    searchResultR = indxNames.find_right("AAAA2");
    unittest::ExpectEqual(std::string, "AAAA2", searchResultR->first);
    unittest::ExpectEqual(int, 1, searchResultR->second);

    searchResultR = indxNames.find_right("CCCC1");
    unittest::ExpectEqual(std::string, "CCCC1", searchResultR->first);
    unittest::ExpectEqual(int, 3, searchResultR->second);

    searchResultR = indxNames.find_right("EEEE1");
    unittest::ExpectEqual(std::string, "EEEE1", searchResultR->first);
    unittest::ExpectEqual(int, 5, searchResultR->second);

    searchResultR = indxNames.find_right("EEEE2");
    unittest::ExpectEqual(std::string, "EEEE2", searchResultR->first);
    unittest::ExpectEqual(int, 5, searchResultR->second);

//////     tests for    isRight_end

    searchResultR = indxNames.find_right("aaaa1");
    unittest::ExpectTrue(indxNames.isRight_end(searchResultR));

    searchResultR = indxNames.find_right("eeee1");
    unittest::ExpectTrue(indxNames.isRight_end(searchResultR));

  } catch (const std::exception& ex) {
    std::cout << ex.what() << std::endl;
    rv = 1;
  }

  return rv;
}