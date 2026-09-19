# Linux ASCII Art Database — Design Documentation

## Purpose

The Windows edition of BinaryFetch only ever needs one ASCII art (a
single embedded default, user-overridable via `BinaryArt.txt`). The
Linux edition needs to represent 400+ distributions, each with its own
logo. This document describes the database structure chosen to hold
that data and how it's looked up at runtime.

## Why a separate file

`AsciiArt.cpp` is about **behavior** — loading, color-tag parsing
(`processColorCodes`), width calculation, padding. The distro art
collection is pure **data**. Mixing 400+ raw ASCII strings into the
same file as the loading/rendering logic would:

- bloat `AsciiArt.cpp` into an unreadable multi-thousand-line file,
- force a rebuild of the entire art collection any time loading logic
  changes (and vice versa),
- break the separation-of-concerns pattern already used everywhere
  else in the project (headers = contract, `.cpp` = implementation,
  JSON = configuration data).

So the data lives in its own file: `linux_ascii_database.cpp`, backed
by `linux_ascii_database.h`. `AsciiArt.cpp` (Linux edition) detects the
running distro and asks this database for the matching art string,
then feeds that string into the exact same pipeline it already uses
for the Windows default art (`processColorCodes`, `visible_width`,
etc.) — no new rendering path required.

## File layout

```
include/core/linux_ascii_database.h
cpp/Linux/core/linux_ascii_database.cpp
```

## Header — `linux_ascii_database.h`

```cpp
#ifndef LINUX_ASCII_DATABASE_H
#define LINUX_ASCII_DATABASE_H

#include <string>
#include <vector>
#include <map>

// One database entry: the raw art string (using the same $N color-tag
// syntax AsciiArt.cpp already parses via processColorCodes()), plus
// every alternate ID that should resolve to this same art.
struct DistroArt {
    std::string art;
    std::vector<std::string> aliases;
};

// Returns the full database — one flat map, built once.
const std::map<std::string, DistroArt>& getDistroArtDatabase();

// Main lookup used by AsciiArt.cpp (Linux edition):
//   1. Try distroId directly against the map's keys.
//   2. Try distroId against every entry's aliases.
//   3. Fall back to the generic "linux" entry (always present).
const std::string& getDistroArt(const std::string& distroId);

#endif // LINUX_ASCII_DATABASE_H
```

## Implementation — `linux_ascii_database.cpp`

One flat `std::map` literal — **no grouping functions, no per-family
helper functions.** Every entry lives in the same table, alphabetical
by key, so adding a new distro is a one-line addition with nothing
else to touch.

```cpp
#include "linux_ascii_database.h"

static const std::map<std::string, DistroArt> kDistroArtDB = {

    { "alpine", { R"ASCIIART($4   /\ /\
$4  /  X  \
$4 /_______\
)ASCIIART", {"alpinelinux"} } },

    { "arch", { R"ASCIIART($6      /\
$6     /  \
$6    /\   \
$6   /      \
$6  /   ,,   \
$6 /   |  |   \
$6/_-''    ''-_\
)ASCIIART", {"archlinux", "arch-linux"} } },

    { "debian", { R"ASCIIART($5    _____
$5   /  __ \
$5  |  /    |
$5   \ \___/
$5    -----
)ASCIIART", {} } },

    { "fedora", { R"ASCIIART($12      _____
$12   -/     \-
$12  |  __ __ |
$12   \ |  | |/
$12    ---__---
)ASCIIART", {"fedora-workstation", "fedora-server"} } },

    { "manjaro", { R"ASCIIART($2 ||||||||| ||||
$2 ||||||||| ||||
$2 ||||      ||||
$2 |||| |||| ||||
$2 |||| |||| ||||
)ASCIIART", {"manjaro-linux"} } },

    { "pop", { R"ASCIIART($6  ______
$6 /      \
$6|  POP!  |
$6 \______/
)ASCIIART", {"pop_os", "popos", "pop!_os"} } },

    { "ubuntu", { R"ASCIIART($8         _
$8     ---(_)
$8 _/  ---  \
$8(_) |   |
$8  \  ---  /
$8   ---(_)
)ASCIIART", {"ubuntu-desktop", "ubuntu-server"} } },

    // ... 390+ more entries, same flat pattern ...

    { "linux", { R"ASCIIART($7    .--.
$7   |o_o |
$7   |:_/ |
$7  //   \ \
$7 (|     | )
$7/'\_   _/`\
$7\___)=(___/
)ASCIIART", {"generic", "unknown"} } },
};

const std::map<std::string, DistroArt>& getDistroArtDatabase() {
    return kDistroArtDB;
}

const std::string& getDistroArt(const std::string& distroId) {
    auto it = kDistroArtDB.find(distroId);
    if (it != kDistroArtDB.end()) return it->second.art;

    for (const auto& [key, entry] : kDistroArtDB) {
        for (const auto& alias : entry.aliases) {
            if (alias == distroId) return entry.art;
        }
    }
    return kDistroArtDB.at("linux").art;
}
```

## Key structure

| Aspect | Decision |
|---|---|
| Storage shape | One flat `std::map<std::string, DistroArt>` |
| Organization | No grouping functions — single literal table |
| Primary key | The distro's `ID=` field from `/etc/os-release` (lowercase, single token, stable across releases) |
| Never keyed on | `PRETTY_NAME=` — freeform text, changes wording between point releases |
| Color syntax | Reuses the existing `$N` tags already parsed by `AsciiArt.cpp`'s `processColorCodes()` — no new rendering path |
| Aliases | Flat `vector<string>` per entry — cheap exact-match fallback, no fuzzy matching |
| Variants (workstation/server/desktop) | Share one entry via aliases, unless a variant is deliberately given distinct art |
| Fallback | A mandatory `"linux"` entry, always present — lookup never returns empty or crashes on an unrecognized distro |

## Lookup flow

```
/etc/os-release
    ID=ubuntu          <- primary lookup key
    ID_LIKE=debian     <- optional secondary fallback chain
            |
            v
getDistroArt(distroId)
    1. Direct key match in kDistroArtDB
    2. Alias match across every entry
    3. Fallback to kDistroArtDB["linux"]
            |
            v
Returned art string
            |
            v
AsciiArt.cpp (Linux edition)
    - same processColorCodes() / visible_width() / padding pipeline
      already used for the Windows embedded default art
            |
            v
Terminal Output
```

## Rationale for classification rules

- **Key by `ID=`, not display name** — machine-stable, guaranteed
  lowercase and single-word, doesn't drift between point releases the
  way a `PRETTY_NAME` string might.
- **`ID_LIKE` as a fallback chain** (handled in `AsciiArt.cpp`, not in
  the database itself) — lets derivative distros with no dedicated art
  entry fall back to their parent family (e.g. an unlisted Debian
  derivative falling back to `"debian"`) before hitting the generic
  `"linux"` art.
- **Variants decided per-distro, not by a blanket rule** — most
  workstation/server variants share one entry via aliases (cheaper,
  usually visually identical logos), but a distro is free to get a
  distinct entry when its branding genuinely differs (e.g. Manjaro is
  not aliased to `"arch"` despite being Arch-based, because its art is
  visually its own).
- **Fallback is mandatory** — `"linux"` must always exist in the table;
  this guarantees `getDistroArt()` is a total function with no
  possible crash or empty-string return on an unknown distro.