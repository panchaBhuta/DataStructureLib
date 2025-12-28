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
| **Ubuntu&nbsp;22.04** | 13 ❌<br> 12 ✅<br>11 ✅ | 15 ✅<br>14 ✅<br>13 ✅ | - | - | - | default: g++-11<br>clang: clang++-14 |
| **Ubuntu&nbsp;22.04**<br>(ARM64) | 13 ✅<br> 12 ✅<br>11 ✅ | 15 ✅<br>14 ✖️<br>13 ✖️ | - | - | - | default: g++-11<br>clang: clang++-14 |
| **macOS-26**<br>(ARM64) | 15 ✅<br> 14 ✅<br> 13 ✅ | - | 17 ✅, 16 ⚠️ | - | - | AppleClang&nbsp;17 |
| **macOS-15**<br>(ARM64) | 15 ✅<br> 14 ✅<br> 13 ✅<br>12 ✖️ | - | 17 ✅, 15 ⚠️ | - | - | AppleClang&nbsp;17 |
| **macOS-14**<br>(ARM64) | 15 ✅<br> 14 ✅<br> 13 ✅<br>12 ⚠️ | - | 15 ✅ | - | - | AppleClang&nbsp;15 |
| **macOS-13🦖**<br>(x86_64) | 14 ✔️<br> 13 ✔️<br>12 ✔️ | - | 15 ✔️ | - | - | AppleClang&nbsp;15 |
| **macOS-12🦖**<br>(x86_64) | 14 ✔️<br> 13 ✖️<br>12 ✔️ | - | - | - | - | AppleClang&nbsp;14 |
| **Windows&nbsp;10<br>VS&nbsp;17&nbsp;-&nbsp;2025** | 15 ✅ | - | - | 19 ✅ | 19 ✅ | msvc 19 |
| **Windows&nbsp;10<br>VS&nbsp;17&nbsp;-&nbsp;2022** | 15 ✅ | - | - | 19 ✅ | 19 ✅ | msvc 19 |
| **Windows&nbsp;10<br>VS&nbsp;16&nbsp;-&nbsp;2019🦖** | - | - | - | - | 12 ✔️ | - |

<br>

[//]: # (for special characters , refer :: https://www.vertex42.com/ExcelTips/unicode-symbols.html)

✅ : Tests success on last execution.

🦖 : OS runner image now Withdrawn.

⚠️ : Was working earlier. Compiler later got removed from runner newer-image.

✔️ : Was working earlier. OS withdrawn, hence tests not repeatable on github.

❌ : Was working earlier. Latest run fails.

✖️ : Compilation failure.

<br>


