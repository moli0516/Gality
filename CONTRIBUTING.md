# Contributing to Gality Engine

Thank you for your interest in contributing! 🎉

## Ways to Contribute

- 🐛 Report bugs
- 💡 Suggest features
- 📝 Improve documentation
- 🔧 Submit pull requests
- 🌍 Translate
- 🎨 Create example games

## Getting Started

### Prerequisites

- C++17 compiler
- CMake 3.20+
- SFML 3.x
- vcpkg
- Python 3.10+ (for tools)

### Build from Source

```bash
git clone https://github.com/moli0516/Gality.git
cd gality
build_dev.bat   # Windows
```

# or

```bash
./build_game.sh  # macOS/Linux
```

### Running Tests

```bash
# Unit tests (if available)

python -m pytest

# Manual test

build_dev.bat
```

## Code Style

### C++

- Follow [Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html)
- 4-space indentation
- `snake_case` for variables, `PascalCase` for classes
- Use `auto` sparingly
- Prefer `const` and `constexpr`

### Python

- Follow PEP 8
- 4-space indentation
- `snake_case` for functions and variables

### JSON

- 2-space indentation
- UTF-8 encoding

## Pull Request Process

1. Fork the repo
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'feat: add amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

### PR Guidelines

- Keep PRs focused (one feature/fix per PR)
- Include tests if applicable
- Update documentation
- Reference related issues
- Use conventional commit messages:
  - `feat:` new feature
  - `fix:` bug fix
  - `docs:` documentation
  - `refactor:` code refactoring
  - `perf:` performance
  - `test:` tests
  - `chore:` maintenance

## Reporting Bugs

Use the [bug report template](.github/ISSUE_TEMPLATE/bug_report.md).

Include:

- OS and version
- Gality version
- Steps to reproduce
- Expected vs actual behavior
- Screenshots/logs

## Suggesting Features

Use the [feature request template](.github/ISSUE_TEMPLATE/feature_request.md).

## Code of Conduct

Be respectful, inclusive, and patient. We follow the
[Contributor Covenant](https://www.contributor-covenant.org/).

## Questions?

- Open a [Discussion](https://github.com/[your-username]/gality/discussions)
- Join our [Discord](https://discord.gg/xxxxx) (if available)

Thank you! ❤️
