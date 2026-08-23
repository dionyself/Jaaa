# Contributing to JAAA

Thank you for your interest in contributing to **JAAA**! Contributions from the open-source community help keep JAAA robust, maintainable, and continuously improving.

Please take a moment to review this document prior to submitting your contributions. Adhering to these guidelines ensures code quality, consistency, and a smooth review process for everyone.

---

## Table of Contents

1. [Code of Conduct](#code-of-conduct)
2. [Licensing & Copyright (GPLv2)](#licensing--copyright-gplv2)
3. [Getting Started](#getting-started)
4. [How to Contribute](#how-to-contribute)
   - [Reporting Bugs](#reporting-bugs)
   - [Suggesting Features](#suggesting-features)
   - [Submitting Pull Requests](#submitting-pull-requests)
5. [Coding & Style Guidelines](#coding--style-guidelines)
   - [C++ Standard (C++11)](#c-standard-c11)
   - [Code Formatting (`clang-format` v14)](#code-formatting-clang-format-v14)
   - [Best Practices](#best-practices)
   - [License Header Template](#license-header-template)
6. [Commit Message Conventions](#commit-message-conventions)
7. [Pull Request Checklist](#pull-request-checklist)

---

## Code of Conduct

We aim to foster an open, welcoming, and collaborative development environment. All contributors are expected to:
* Exercise respect, empathy, and professionalism toward all project participants.
* Offer and accept constructive feedback gracefully.
* Focus on project quality and community collaboration.

---

## Licensing & Copyright (GPLv2)

**JAAA** is open-source software released under the **GNU General Public License v2.0 (GPLv2)**.

By contributing to JAAA, you agree that all submitted code, documentation, and assets will be licensed under the terms of the **GNU General Public License, Version 2 (GPL-2.0)**.

### Licensing Requirements:
1. **Compatible Code:** Do not submit code copied from third-party sources unless it is strictly compatible with GPLv2. Always cite original copyright holders and licenses where applicable.
2. **Copyright Headers:** Every new C++ source (`.cpp`, `.cc`, `.cxx`) and header (`.h`, `.hpp`) file must include the standard GPLv2 copyright notice at the top of the file.
3. **Developer Certificate of Origin (DCO):** By adding a `Signed-off-by` line to your commits (`git commit -s`), you certify that you have the right to submit the code under the GPLv2 license.

---

## Getting Started

### Prerequisites

To build, test, and format code for JAAA, ensure your development environment has the following installed:

* **C++ Compiler:** C++11 compliant compiler (e.g., GCC 4.8+, Clang 3.3+, or MSVC 2015+).
* **Build System:** CMake (>= 3.10) or GNU Make.
* **Formatter:** `clang-format` **version 14.x** (strictly version 14 to avoid style diffs across toolchain releases).

### Repository Setup

1. **Fork** the official JAAA repository on GitHub / GitLab.
2. **Clone** your fork locally:
   ```bash
   git clone https://github.com/your-username/jaaa.git
   cd jaaa
   ```
3. **Configure Upstream Remote:**
   ```bash
   git remote add upstream https://github.com/dionyself/jaaa.git
   git fetch upstream
   ```
4. **Create a Topic Branch:**
   ```bash
   git checkout -b feature/my-feature-name
   ```

---

## How to Contribute

### Reporting Bugs

Before opening a new bug report, search existing issues to avoid duplicates.

When filing an issue, please include:
* **Environment:** OS version, compiler version (`gcc --version` / `clang --version`), and CMake version.
* **Reproduction Steps:** A clear, step-by-step description of how to reproduce the issue.
* **Expected vs. Actual Behavior:** What should happen versus what actually happened.
* **Logs & Stack Traces:** Relevant terminal output, backtraces, or core dumps.

### Suggesting Features

Feature proposals are welcomed. Please include:
* A concise title and description of the proposed capability.
* Rationale and real-world use cases.
* Any technical design ideas or considerations for implementation within JAAA.

### Submitting Pull Requests

1. **Keep PRs Atomic:** Limit each pull request to a single feature, bug fix, or refactoring step.
2. **Rebase Regularly:** Keep your branch up to date with `upstream/main` (or `upstream/master`) using `git rebase`.
3. **Build & Test:** Ensure the project compiles clean without warnings and all unit tests pass.
4. **Format Code:** Verify code complies with `clang-format` version 14.

---

## Coding & Style Guidelines

### C++ Standard (C++11)

JAAA enforces strict adherence to **C++11** (`-std=c++11`).

* **Language Level Constraints:**
  * All code must compile cleanly using the `-std=c++11` flag.
  * Features from C++14, C++17, C++20, or C++23 (e.g., `std::make_unique` [write a C++11 local helper if necessary], `std::filesystem`, generic lambdas, structured bindings) are **prohibited**.
* **Recommended C++11 Idioms:**
  * Use **RAII** for all resource management.
  * Prefer smart pointers (`std::unique_ptr`, `std::shared_ptr`) over raw `new` and `delete`.
  * Use `nullptr` instead of `NULL` or `0`.
  * Use `override` and `final` explicitly when declaring overridden virtual functions.
  * Use `constexpr` and `enum class` for type-safe constants and enumerations.
  * Leverage move semantics (`std::move`, `std::forward`) to optimize performance where appropriate.
  * Use range-based `for` loops and lambdas to simplify container iterations.

### Code Formatting (`clang-format` v14)

All C++ source and header files in JAAA **must** be formatted using **`clang-format` version 14**.

#### Formatting Commands
A `.clang-format` configuration file resides in the repository root. Format your files before committing:

```bash
# Check version (must be 14.x)
clang-format-14 --version

# Format a single file in-place
clang-format-14 -i src/my_module.cpp

# Format all C++ sources and headers in the project
find src include -type f \( -name "*.cpp" -o -name "*.hpp" -o -name "*.h" -o -name "*.cc" \) -exec clang-format-14 -i {} +
```

#### Core Style Rules
* **Indentation:** 2 spaces (no tab characters).
* **Line Width:** 80–100 characters maximum.
* **Brace Style:** Attach braces to control statements (`if`, `for`, `while`, `switch`) and function definitions as specified in `.clang-format`.
* **Explicit Bracing:** Always use braces `{}` for single-line conditional blocks or loops.
* **Naming Conventions:**
  * `PascalCase` for Classes, Structs, Enums, and Type aliases (`class AudioBuffer;`).
  * `camelCase` or `snake_case` for functions and local variables (maintain consistency with existing module style).
  * Private/protected member variables must use the `m_` prefix (e.g., `m_sampleRate`).
  * Preprocessor macros and constants should be `ALL_CAPS`.

### Best Practices

* **Zero Compiler Warnings:** Code must compile cleanly with strict compiler flags (`-Wall -Wextra -Wpedantic -Werror -std=c++11`).
* **Const Correctness:** Mark member functions, parameters, and variable references `const` wherever appropriate.
* **Header Organization:** Use `#pragma once` or inclusion guards in every header file. Order `#include` directives as follows:
  1. Main module header (`#include "my_module.hpp"`)
  2. JAAA internal headers (`#include "jaaa/core/..."`)
  3. Third-party library headers
  4. C++ Standard Library headers (`#include <vector>`, `#include <memory>`)

### License Header Template

Every newly created source (`.cpp`, `.cc`) or header (`.hpp`, `.h`) file must begin with the standard GPLv2 copyright header:

```cpp
/*
 * JAAA - [Brief description of file purpose]
 * Copyright (C) [Year] [Your Name or Organization]
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
```

---

## Commit Message Conventions

Maintain concise, structured, and informative commit messages.

### Format
```text
<type>(<scope>): <short summary in imperative mood>

<optional detailed explanation of changes and motivation>

Fixes #<issue_number>
Signed-off-by: Your Name <your.email@example.com>
```

### Commit Types
* `feat`: A new feature or capability.
* `fix`: A bug fix.
* `docs`: Documentation updates only.
* `style`: Code formatting changes (e.g., `clang-format` updates, whitespace).
* `refactor`: Code restructuring without changing functional behavior.
* `test`: Adding or updating test cases.
* `build`: Build system or configuration tweaks (CMake, flags, scripts).

---

## Pull Request Checklist

Before submitting your Pull Request, double check that:

- [ ] Code compiles without warnings using `-std=c++11 -Wall -Wextra -Werror`.
- [ ] Code is formatted strictly with **`clang-format` version 14**.
- [ ] All new files contain the GPLv2 copyright header.
- [ ] New and existing tests pass.
- [ ] Commit history is clean, rebased against `upstream/main`, and signed off (`git commit -s`).
- [ ] The PR description clearly references any related issues and explains the rationale for the change.

Thank you for contributing to **JAAA**!
