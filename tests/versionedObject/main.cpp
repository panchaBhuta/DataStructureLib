#include <testHelper.h>

int main()
{
  int rv = 0;
  t_versionObject vo;

  try {
    std::cout << "TEST_LOG : FIRST VO load , no initial data. fresh start." << std::endl;
    loadVO(vo, true);
    std::cout << "TEST_LOG : SECOND VO load , with existing data reloaded again. scenario after first load." << std::endl;
    loadVO(vo, false);

  } catch (const std::exception& ex) {
    std::cout << ex.what() << std::endl;
    rv = 1;
  }

  return rv;
}
