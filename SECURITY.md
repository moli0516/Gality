
# Security Policy

## Supported Versions

Gality Engine is under active development. Security updates are provided
for the following versions:

| Version | Supported          | Notes |
|---------|--------------------|-------|
| 0.3.x   | ✅ Active support  | Current development line |
| 0.2.x   | ⚠️ Critical only   | No new features |
| 0.1.x   | ❌ End of life     | Upgrade to 0.3.x |
| < 0.1   | ❌ Not supported   | Pre-release only |

**Recommendation**: Always use the latest release. See
[CHANGELOG.md](CHANGELOG.md) for version history.

---

## Reporting a Vulnerability

**Please do not open a public GitHub issue for security vulnerabilities.**

If you discover a security issue in Gality Engine, report it privately to:

📧 **Email**: `[your-email@example.com]`

Please include:

- **Description** of the vulnerability
- **Steps to reproduce** (minimal example if possible)
- **Affected versions** (e.g., `v0.3.0`)
- **Potential impact** (data loss, code execution, etc.)
- **Suggested fix** (if you have one)
- **Your contact info** (optional, for follow-up)

### What to Expect

| Timeline | Action |
|----------|--------|
| **Within 72 hours** | Acknowledgment of receipt |
| **Within 7 days**   | Initial assessment and severity classification |
| **Within 30 days**  | Patch released (for critical issues) |
| **After patch**     | Public disclosure with credit (if desired) |

We follow a **coordinated disclosure** policy. We ask that you:

- Give us reasonable time to investigate and patch before public disclosure
- Do not exploit the vulnerability beyond what's needed to demonstrate it
- Do not access, modify, or delete data that isn't yours

We will credit you in the release notes unless you prefer to remain anonymous.

---

## Security Considerations

Gality Engine is a **local-first desktop application** for creating and
playing visual novels. It does not connect to the internet, run remote code,
or manage user accounts. The attack surface is deliberately small.

That said, the following aspects deserve attention:

### 1. Asset Archive Encryption (`data.pak`)

**Status**: ⚠️ Not cryptographically secure.

Gality uses **XOR obfuscation** (key: `GalityEngine2026`) to pack assets into
a single `data.pak` file. This is intended to:

- Prevent casual browsing of assets by end users
- Keep the distribution clean (single file instead of folders)
- Obscure story spoilers from curious players

**It is NOT intended to:**

- Protect against determined reverse engineering
- Secure sensitive data
- Provide any cryptographic guarantee

**For creators**: If your assets contain sensitive material (personal
images, licensed content, etc.), do not rely on `data.pak` for confidentiality.
Use OS-level file permissions, disk encryption, or dedicated DRM solutions.

**For contributors**: Do not treat XOR as "encryption" in documentation or
code comments. It is obfuscation only.

### 2. Script Execution (`.gality` DSL)

**Status**: ✅ Safe by design.

Gality scripts are compiled to a JSON AST and executed by an FSM loop. They
**cannot**:

- Execute arbitrary system commands
- Access the file system beyond pre-declared assets
- Open network connections
- Allocate unbounded memory
- Cause undefined behavior in the C++ runtime

The DSL is deliberately non-Turing-complete in its executable form. Loops,
recursion, and dynamic code generation are **not** supported.

**What the DSL can do**:

- Set blackboard variables (integers, booleans)
- Branch on conditions
- Trigger audio, visuals, and transitions
- Advance to another node

**What the DSL cannot do**:

- Read or write files
- Execute external programs
- Load dynamic libraries
- Perform network I/O

### 3. Save Files

**Status**: ⚠️ Plain JSON, user-editable.

Save files are stored as plain JSON in the `saves/` directory:

```
saves/
├── save_slot_1.json
├── save_slot_2.json
└── ...
```

**Implications**:

- Users can freely edit save files (this is intentional)
- A corrupted save file cannot crash the engine — it will fail to load gracefully
- Save files may contain story spoilers if viewed in a text editor

**Schema validation**:

On load, save files are validated against a schema. Unknown fields are
ignored. Missing required fields cause a graceful failure with an error
message, not a crash.

**For creators**: If your story involves sensitive choices or secrets,
consider that players can inspect save files. Do not store secrets in the
blackboard unless they are meant to be discoverable.

### 4. Asset Loading (`AssetPack.hpp`)

**Status**: ⚠️ Trusts the archive.

`AssetPack::readFileFromPak()` reads assets from `data.pak` based on a
header index. The following validations are performed:

- Header length bounds check
- File count bounds check
- Per-entry path length bounds check
- Per-entry offset/size bounds check
- Buffer allocation matches declared size

**What is NOT validated**:

- Whether asset file content matches its claimed type
- Whether image dimensions are sane (e.g., huge PNG bombs)
- Whether audio buffers are valid before passing to SFML

**Potential attack vector**: A maliciously crafted `data.pak` could cause:

- Excessive memory allocation
- Long decompression times
- SFML-level crashes on malformed image/audio data

**Mitigation**: Gality does not download or fetch `data.pak` from anywhere.
The archive is provided by the user or the game publisher. If you distribute
a game to players, use a trusted channel (itch.io, Steam) and sign your
releases with a checksum.

**For contributors**: If you add new asset types, extend `AssetPack` with
proper validation for that type (e.g., dimension checks for images, duration
limits for audio).

### 5. Font Loading

**Status**: ✅ Trusts bundled fonts, but SFML handles the parsing.

Fonts are loaded via SFML + FreeType. Malformed font files could theoretically
exploit FreeType vulnerabilities, but:

- Fonts are typically bundled by the creator (trusted)
- FreeType is widely used and regularly patched
- SFML fails gracefully on invalid fonts

**For creators**: Only bundle fonts you have the right to distribute. See
[Free Font Licenses](https://www.gnu.org/licenses/license-list.html#Fonts)
for guidance.

### 6. Dependency Management

**Status**: ✅ Managed via vcpkg.

Gality's dependencies are pinned in `vcpkg.json`:

```json
{
  "dependencies": [
    "sfml",
    "nlohmann-json"
  ]
}
```

**Updating dependencies**:

- Run `vcpkg upgrade` to get the latest compatible versions
- Review the changelog of each dependency for security fixes
- Test after upgrading

**Known considerations**:

- **SFML** depends on system libraries (OpenGL, X11, etc.). Keep your OS
  updated.
- **nlohmann/json** is header-only and has no known security issues.
- **FreeType** (transitively via SFML) has had CVEs in the past. Use a
  recent version.

### 7. Build-Time Security

**Status**: ✅ No remote code execution.

The build process:

1. Uses **vcpkg** to download and compile dependencies
2. Uses **CMake** to configure the project
3. Compiles C++ sources with the system compiler

**No** code is downloaded at build time from untrusted sources. All
dependencies come from vcpkg's verified registry.

**For CI/CD**: If you set up automated builds, pin vcpkg to a specific
commit hash. See [vcpkg baseline](https://learn.microsoft.com/en-us/vcpkg/users/project-config#baseline).

### 8. Debug Tools (F1 / F2)

**Status**: ⚠️ Dev builds only.

`DebugOverlay` (F1) and `NodeGraphViewer` (F2) are powerful tools that
allow runtime modification of:

- Blackboard variables
- Current story node
- Dialogue text
- Node properties

**These are compiled ONLY in dev builds** (`GALITY_DEV_BUILD=ON`).

Production builds (`build_prod.bat`, `GALITY_DEV_BUILD=OFF`) exclude:

- `DebugOverlay.hpp`
- `NodeGraphViewer.hpp`
- `HotReloader.hpp`

**For publishers**: Always ship `build_prod.bat` output. Do not ship dev
builds to end users. The debug tools allow cheating and save-file manipulation.

---

## Threat Model

Gality assumes the following trust model:

| Component | Trust Level |
|-----------|-------------|
| **Engine source code** | Trusted (open source) |
| **`.gality` scripts** | Trusted (provided by creator) |
| **`data.pak` archive** | Trusted (provided by creator) |
| **Save files** | User-editable, safe to modify |
| **Fonts** | Trusted (bundled by creator) |
| **OS / system libraries** | Trusted (updated by user) |
| **Network** | Not used |

**Out of scope**:

- Protecting assets from determined reverse engineering
- Preventing save-file editing
- Preventing cheating in single-player games
- Enforcing DRM
- Sandboxing malicious creators (creators are trusted)

**In scope**:

- Preventing crashes from malformed input files
- Ensuring graceful failure on corrupted data
- Bounds-checking all binary parsing
- Avoiding undefined behavior in C++ code

---

## Security Best Practices for Creators

If you're using Gality to build a game:

### 1. Validate Your Inputs

If your game reads any external files (future feature), validate them
against expected schemas. The `.gality` compiler already does this for
scripts, but future extensions may not.

### 2. Don't Store Secrets

Do not store API keys, passwords, or secrets in:

- `ui_theme.json`
- `settings.json`
- `data.pak`
- Save files

These can all be inspected by players.

### 3. Sign Your Releases

When distributing your game:

- Generate a SHA-256 checksum of your `.zip` or `.pak`
- Publish the checksum alongside the download
- Users can verify integrity after download

```bash
# Generate checksum (Windows)
certutil -hashfile Gality.zip SHA256

# Generate checksum (macOS/Linux)
shasum -a 256 Gality.zip
```

### 4. Use HTTPS for Distribution

Only distribute through trusted channels:

- ✅ itch.io
- ✅ Steam
- ✅ GitHub Releases
- ✅ Your own HTTPS site

Avoid:

- ❌ Direct file sharing (email, USB)
- ❌ HTTP-only sites
- ❌ Unverified mirrors

### 5. Keep the Engine Updated

Check the [CHANGELOG.md](CHANGELOG.md) and [GitHub Releases](https://github.com/moli0516/Gality/releases)
regularly. Security fixes are documented there.

---

## Security Best Practices for Contributors

If you're contributing code to Gality:

### 1. Bounds-Check All Binary Parsing

When parsing `data.pak`, save files, or any binary format:

```cpp
// ❌ Bad
uint32_t size = readU32(data, pos);
buffer.resize(size);

// ✅ Good
if (pos + 4 > data.size()) return false;
uint32_t size = readU32(data, pos);
if (size > MAX_ALLOWED_SIZE) return false;
buffer.resize(size);
```

### 2. Avoid Undefined Behavior

- Use `std::vector::at()` when indices come from user input
- Check for null pointers before dereferencing
- Use `std::optional` for values that might not exist
- Prefer `std::string_view` over raw `char*`

### 3. No Dynamic Code Execution

Never add features that:

- Evaluate strings as code
- Load dynamic libraries from user input
- Execute system commands
- Deserialize objects with polymorphic types

### 4. Sanitize File Paths

When resolving asset paths, prevent path traversal:

```cpp
// ❌ Bad
auto path = assetsDir + "/" + userInput;

// ✅ Good
auto path = assetsDir / std::filesystem::path(userInput).filename();
```

### 5. Report Suspicious Dependencies

If a PR adds a new dependency, review it for:

- Recent security advisories
- Active maintenance
- Trusted source
- Reasonable scope

Explain why the dependency is needed in the PR description.

---

## Past Security Issues

No security issues have been reported as of 2026-09-17.

Future issues will be documented here with:

- CVE identifier (if assigned)
- Affected versions
- Fixed version
- Mitigation steps

---

## References

- [OWASP Top 10](https://owasp.org/www-project-top-ten/)
- [CWE Top 25](https://cwe.mitre.org/top25/)
- [SFML Security](https://www.sfml-dev.org/)
- [vcpkg Security](https://learn.microsoft.com/en-us/vcpkg/about/security)

---

## Contact

For security-related inquiries:

- 📧 **Email**: `[your-email@example.com]`
- 🔒 **PGP Key**: `[optional, if you have one]`
- 🐙 **GitHub**: [@moli0516](https://github.com/moli0516)

For non-security issues, please use:

- **Bug reports**: [GitHub Issues](https://github.com/moli0516/Gality/issues)
- **Feature requests**: [GitHub Discussions](https://github.com/moli0516/Gality/discussions)
- **Questions**: See [CONTRIBUTING.md](CONTRIBUTING.md)

---

**Last updated**: 2026-09-17
