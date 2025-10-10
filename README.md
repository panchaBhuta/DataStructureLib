TODO: documentation


Versioned-Object
================

[//]: # (<img src="images/RefactoringInProgress.jpg" width="500">)

[//]: # (## Documentation is **NOT** updated to latest changes.)



| **Linux** | **Mac** | **Windows** |
|-----------|---------|-------------|
| [![Linux](https://github.com/panchaBhuta/DataStructureLib/actions/workflows/linux.yml/badge.svg)](https://github.com/panchaBhuta/DataStructureLib/actions/workflows/linux.yml) | [![macOS](https://github.com/panchaBhuta/DataStructureLib/actions/workflows/macos.yml/badge.svg)](https://github.com/panchaBhuta/DataStructureLib/actions/workflows/macos.yml) | [![Windows](https://github.com/panchaBhuta/DataStructureLib/actions/workflows/windows.yml/badge.svg)](https://github.com/panchaBhuta/DataStructureLib/actions/workflows/windows.yml) |

Versioned state of tuple objects. Each versioned objects can either be complete state-info OR delta-info.

Documentation in process.


Supported Platforms
===================
Rapidcsv is implemented using C++20 with code portable across OS and it's supported compiler's.<br>

Unit-Test results of last run across multiple OS-versions and Compiler-versions :

| <nobr>🤖&nbsp;Compiler&nbsp;➡️</nobr><br><nobr>🖥️ OS ⬇️</nobr> | **g++** | **clang++** | **AppleClang** | **msvc** | **clangCL** | default&nbsp;Compiler |
|------------|------------|------------------|--------------|--------------|--------------|--------------|
| **Ubuntu&nbsp;24.04** | 14 ✅<br>13 ✅<br>12 ✅ | 18 ✅<br>17 ✅<br>16 ✅ | - | - | - | default: g++-13<br>clang: clang++-18 |
| **Ubuntu&nbsp;24.04**<br>(ARM64) | 14 ✅<br>13 ✅<br>12 ✅ | 18 ✅<br>17 ✅<br>16 ✅ | - | - | - | default: g++-13<br>clang: clang++-18 |
| **Ubuntu&nbsp;22.04** | 13 ❌<br> 12 ✅<br>11 ✅<br>&nbsp; | 15 ✅<br>14 ✅<br>13 ✅ | - | - | - | default: g++-11<br>clang: clang++-14 |
| **Ubuntu&nbsp;22.04**<br>(ARM64) | 13 ✅<br> 12 ✅<br>11 ✅ | 15 ✅<br>14 ✖️<br>13 ✖️ | - | - | - | default: g++-11<br>clang: clang++-14 |
| **macOS-15**<br>(ARM64) | 14 ✖️<br> 13 ✖️<br>12 ✖️ | - | 16 ✅ | - | - | AppleClang&nbsp;16 |
| **macOS-14**<br>(ARM64) | 14 ✅<br> 13 ✅<br>12 ✅ | - | 15 ✅ | - | - | AppleClang&nbsp;15 |
| **macOS-13**<br>(x86_64) | 14 ✅<br> 13 ✅<br>12 ✅ | - | 15 ✅ | - | - | AppleClang&nbsp;15 |
| **macOS-12🦖**<br>(x86_64) | 14 ✔️<br> 13 ✖️<br>12 ✔️ | - | - | - | - | AppleClang&nbsp;14 |
| **Windows&nbsp;10<br>VS&nbsp;17&nbsp;-&nbsp;2025** | 15 ✅ | - | - | 19 ✅ | 19 ✅ | msvc 19 |
| **Windows&nbsp;10<br>VS&nbsp;17&nbsp;-&nbsp;2022** | 15 ✅ | - | - | 19 ✅ | 19 ✅ | msvc 19 |
| **Windows&nbsp;10<br>VS&nbsp;16&nbsp;-&nbsp;2019🦖** | - | - | - | - | 12 ✔️ | - |

<br>

[//]: # (for special characters , refer :: https://www.vertex42.com/ExcelTips/unicode-symbols.html)

✅ : Tests success on last execution.

🦖 : OS runner image now Withdrawn.

⚠️ : Was working earlier. Compiler later got removed from runner image.

✔️ : Was working earlier. OS withdrawn, hence tests not repeatable on github.

❌ : Was working earlier. Latest run fails.

✖️ : Compilation failure.

<br>

❌1 : <span style="color:grey">macOS-13</span> : New linker breaks the build. Both Cmake and Linker needs an update for this fix.

❌2 : <span style="color:grey">clang++ 13\#</span> : Last successful run with [Clang 13.0.1](https://github.com/panchaBhuta/converter/actions/runs/6524732682/job/17716666880) and `OS-id-version=Linux-6.2.0-1012-azure`. **\<chorno>** headers stopped compiling in Newer Ubuntu-image OS-version. Refer [ubuntu-latest runners have an incompatible combination of clang and libstdc++](https://github.com/actions/runner-images/issues/8659).

❌2 : <span style="color:grey">clang++ 14\#</span> : Last successful run with [Clang 14.0.0](https://github.com/panchaBhuta/converter/actions/runs/6524732682/job/17716666951) and `OS-id-version=Linux-6.2.0-1012-azure`. **\<chorno>** headers stopped compiling in Newer Ubuntu-image OS-version. Refer [ubuntu-latest runners have an incompatible combination of clang and libstdc++](https://github.com/actions/runner-images/issues/8659).


[//]:  ❌

