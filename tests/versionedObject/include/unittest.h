#pragma once

#ifndef _MSC_VER
#include <unistd.h>
#else
#include <io.h>
#endif

#include <cmath>
#include <cstdlib>

#include <fstream>
#include <iomanip>
#include <string>
#include <sstream>
#include <type_traits>
#include <chrono>

#include <map>
#include <set>

#define COMMA ,

#define ExpectEqual(t, a, b) ExpectEqualFun<t>(a, b, #a, #b, __FILE__, __LINE__)
#define ExpectTrue(a) ExpectTrueFun(a, #a, __FILE__, __LINE__)

#define ExpectException(expr, excp)                                                           \
  do                                                                                          \
  {                                                                                           \
    bool success = false;                                                                     \
    try                                                                                       \
    {                                                                                         \
      expr;                                                                                   \
    }                                                                                         \
    catch (const excp &)                                                                      \
    {                                                                                         \
      success = true;                                                                         \
    }                                                                                         \
    catch (const std::exception& ex)                                                          \
    {                                                                                         \
      std::stringstream ss;                                                                   \
      ss << unittest::detail::FileName(__FILE__) << ":" << std::to_string(__LINE__);          \
      ss << " ExpectException failed: unexpected exception '" << typeid(ex).name();           \
      ss << "' thrown.  errMsg: " << ex.what() << std::endl;                                  \
      throw std::runtime_error(ss.str());                                                     \
    }                                                                                         \
                                                                                              \
    if (!success)                                                                             \
    {                                                                                         \
      std::stringstream ss;                                                                   \
      ss << unittest::detail::FileName(__FILE__) << ":" << std::to_string(__LINE__);          \
      ss << " ExpectException failed: expected exception '" << #excp << "' not thrown.";      \
      ss << std::endl;                                                                        \
      throw std::runtime_error(ss.str());                                                     \
    }                                                                                         \
  }                                                                                           \
  while (0)


#define ExpectExceptionMsg(expr, excp, msg)                                                   \
  do                                                                                          \
  {                                                                                           \
    bool success = false;                                                                     \
    try                                                                                       \
    {                                                                                         \
      expr;                                                                                   \
    }                                                                                         \
    catch (const excp& ex)                                                                    \
    {                                                                                         \
      if (std::string(ex.what()) == (msg))                                                    \
      {                                                                                       \
        success = true;                                                                       \
      }                                                                                       \
      else                                                                                    \
      {                                                                                       \
        std::stringstream ss;                                                                 \
        ss << unittest::detail::FileName(__FILE__) << ":" << std::to_string(__LINE__);        \
        ss << " ExpectExceptionMsg failed: unexpected exception message '" << ex.what();      \
        ss << "'." << std::endl;                                                              \
        throw std::runtime_error(ss.str());                                                   \
      }                                                                                       \
    }                                                                                         \
    catch (const std::exception& ex)                                                          \
    {                                                                                         \
      std::stringstream ss;                                                                   \
      ss << unittest::detail::FileName(__FILE__) << ":" << std::to_string(__LINE__);          \
      ss << " ExpectExceptionMsg failed: unexpected exception '" << typeid(ex).name();        \
      ss << "' thrown." << std::endl;                                                         \
      throw std::runtime_error(ss.str());                                                     \
    }                                                                                         \
                                                                                              \
    if (!success)                                                                             \
    {                                                                                         \
      std::stringstream ss;                                                                   \
      ss << unittest::detail::FileName(__FILE__) << ":" << std::to_string(__LINE__);          \
      ss << " ExpectException failed: expected exception '" << #excp << "' not thrown.";      \
      ss << std::endl;                                                                        \
      throw std::runtime_error(ss.str());                                                     \
    }                                                                                         \
  }                                                                                           \
  while (0)


namespace unittest
{
  namespace detail
  {
    inline std::string FileName(const std::string& pPath)
    {
      const std::size_t slash = pPath.rfind("/");
      std::string name = (slash != std::string::npos) ? pPath.substr(slash + 1) : pPath;
      return name;
    }
  }

  inline std::string TempPath()
  {
    char name[] = "versionedObjecttest.XX" "XX" "XX";
#ifndef _MSC_VER
    int fd = mkstemp(name);
    close(fd);
#else
    _mktemp_s(name, strlen(name) + 1);
#endif
    return std::string(name);
  }

  inline void WriteFile(const std::string& pPath, const std::string& pData)
  {
    std::ofstream outfile;
    outfile.open(pPath, std::ifstream::out | std::ifstream::binary);
    outfile << pData;
    outfile.close();
  }

  inline std::string ReadFile(const std::string& pPath)
  {
    std::ifstream infile;
    infile.open(pPath, std::ifstream::in | std::ifstream::binary);
    std::string data((std::istreambuf_iterator<char>(infile)), std::istreambuf_iterator<char>());
    infile.close();
    return data;
  }

  inline void DeleteFile(const std::string& pPath)
  {
    std::remove(pPath.c_str());
  }

  template<typename T>
  struct SScompatible {
    inline static const T& getVal(const T& val) { return val; }
  };


  template <typename T>
  concept c_xchar = (std::is_same_v<T, unsigned char> ||
                     std::is_same_v<T, signed char> ||
                     std::is_same_v<T, wchar_t> ||
                     std::is_same_v<T, char8_t> ||
                     std::is_same_v<T, char16_t> ||
                     std::is_same_v<T, char32_t> ); // Note : excluding 'char' type

  template<c_xchar T>
  struct SScompatible<T> {
    inline static std::string getVal(const T& val)
    {
      std::string sVal{" "};
      sVal.at(0) = static_cast<char>(val);
      return sVal;
    }
  };

  template <typename T>
  concept c_is_base_of_ymd_inclusive = std::is_base_of_v<std::chrono::year_month_day, T>;

  template<c_is_base_of_ymd_inclusive T>
  struct SScompatible<T> {
    inline static std::string getVal(const T& val)
    {
      std::ostringstream oss;
      oss << std::setfill('0') << std::setw(4) << static_cast<int>(val.year()) << "-";
      oss << std::setfill('0') << std::setw(2) << static_cast<unsigned>(val.month()) << "-";
      oss << std::setfill('0') << std::setw(2) << static_cast<unsigned>(val.day());

      return oss.str();
    }
  };

  template<>
  struct SScompatible<std::set<std::string>> {
    inline static std::string getVal(const std::set<std::string>& val)
    {
      std::ostringstream oss;
      oss << "set{";
      for(const std::string& sst : val)
      {
        oss << sst << ",";
      }
      oss << "}";
      return oss.str();
    }
  };

  template<typename T>
  inline bool compareEqual(T pTest, T pRef, [[maybe_unused]]int ulp = -1) { return pTest == pRef; }

  template<typename T>
  concept c_floating_point = std::is_floating_point_v<T>;

  template<typename T>
  constexpr int getBTD()  // BTD -> base-10 digits
  {
    if constexpr (std::is_floating_point_v<T>)
#ifndef BUILD_ENV_MSYS2_GNU
      return std::numeric_limits<T>::digits10;
#else
      return std::numeric_limits<T>::digits10 -3;
#endif

    return -1;
  }

  template<c_floating_point T>
  inline bool compareEqual(T pTest, T pRef, int btd = std::numeric_limits<T>::digits10)
  {
    /*
        Previously, the logic of this function has been commented out.
        here, default ulp = std::numeric_limits<T>::digits10                      ULPs -> units in the last place
    // the machine epsilon has to be scaled to the magnitude of the values used
    // and multiplied by the desired precision in ULPs (units in the last place)
    return std::fabs(pTest - pRef) <= std::numeric_limits<T>::epsilon() * std::fabs(pTest + pRef) * ulp
        // unless the result is subnormal
        || std::fabs(pTest - pRef) < std::numeric_limits<T>::min();
    */

    if (pTest == pRef)
      return true;

    using t_ldb = long double;
    t_ldb diffAB = t_ldb(pTest) - t_ldb(pRef);
    std::cout << std::setprecision(std::numeric_limits<t_ldb>::digits10 + 1);
    std::cout << "FLOATING POINT : equality-test :: (pTest{" << pTest
              << "} - pRef{" << pRef << "})= " << (diffAB) << std::endl;

    const T nxt = std::nextafter(std::min(pTest, pRef), +INFINITY);
    std::cout << "FLOATING POINT : std::nextafter(std::min(pTest, pRef), +INFINITY){" << nxt
              << "} == std::max(pTest, pRef){" << std::max(pTest, pRef) << "}" << std::endl;
    if(nxt == std::max(pTest, pRef))
      return true;

    //  https://en.cppreference.com/w/cpp/types/numeric_limits/epsilon.html
    // Since `epsilon()` is the gap size (ULP, unit in the last place)
    // of floating-point numbers in interval [1, 2), we can scale it to
    // the gap size in interval [2^e, 2^{e+1}), where `e` is the exponent
    // of `pTest` and `pRef`.

    // If `pTest` and `pRef` have different gap sizes (which means they have
    // different exponents), we take the smaller one. Taking the bigger
    // one is also reasonable, I guess.
    const T m = std::min(std::fabs(pTest), std::fabs(pRef));

    // Subnormal numbers have fixed exponent, which is `min_exponent - 1`.
    const int exp = m < std::numeric_limits<T>::min()
                  ? std::numeric_limits<T>::min_exponent - 1
                  : std::ilogb(m);

    /*
    // We consider `pTest` and `pRef` equal if the difference between them is
    // within `n` ULPs.
    return std::fabs(pTest - pRef) <= n * std::ldexp(std::numeric_limits<T>::epsilon(), exp);
    */
    const t_ldb ulpRatio =   std::fabs(diffAB)
                                 / std::ldexp(std::numeric_limits<T>::epsilon(), exp);
    std::cout << "FLOATING POINT : ulpRatio{" << ulpRatio << "} <= 1.5L ? "
              << std::boolalpha << bool(ulpRatio <= 1.5L) << std::noboolalpha << std::endl;
    if( ulpRatio <= 1.5L )
      return true;

    const t_ldb normalizedDiff =  std::pow(t_ldb(10.0L), t_ldb(btd)) * std::fabs(diffAB)
                                 / (t_ldb(pRef) + diffAB/(2.0L)); // (pTest + pRef)/2
    std::cout << "FLOATING POINT : normalizedDiff{" << normalizedDiff << "} <= 1.0L ? "
              << std::boolalpha << bool(normalizedDiff <= 1.0L) << std::noboolalpha << std::endl;
    return (normalizedDiff <= 1.0L);
  }

  template<typename T>
  inline void ExpectEqualFun(T pTest, T pRef, const std::string& testName,
                             const std::string& refName, const std::string& filePath, int lineNo,
                             int btd = getBTD<T>())
  {
    if (!compareEqual<T>(pTest, pRef, btd))
    {
      std::stringstream ss;
      ss << std::setprecision(std::numeric_limits<long double>::digits10 + 1);
      ss << detail::FileName(filePath) << ":" << std::to_string(lineNo);
      ss << " ExpectEqual failed: " << testName << " != " << refName << std::endl;
      ss << testName << " = '" << SScompatible<T>::getVal(pTest) << "'" << std::endl;
      ss << refName << " = '"  << SScompatible<T>::getVal(pRef)  << "'" << std::endl;

      throw std::runtime_error(ss.str());
    }
  }

  inline void ExpectTrueFun(bool pTest, const std::string& testName,
                            const std::string& filePath, int lineNo)
  {
    if (!pTest)
    {
      std::stringstream ss;
      ss << detail::FileName(filePath) << ":" << std::to_string(lineNo);
      ss << " ExpectTrue failed: " << testName << " == false" << std::endl;

      throw std::runtime_error(ss.str());
    }
  }
}
