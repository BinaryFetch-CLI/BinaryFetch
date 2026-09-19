# ASCII Art Color Prefixes — Feature Documentation

## Summary

BinaryFetch's ASCII art (`BinaryArt.txt`) uses `$N` placeholders (e.g. `$1`,
`$15`) to color art segments. Previously these 15 slots were hardcoded in
`AsciiArt.cpp` and could not be customized. This change makes the `$N`
color table JSON-driven, following the exact same pattern `ConfigManager`
already uses for the `"colors"` section — while staying **100% backward
compatible**: any config that doesn't define the new section behaves
byte-for-byte identically to before.

## New JSON section: `ascii_color_prefixes`

Placed at the top level, directly after `"colors"`:

```jsonc
{
  "colors": {
    "red":    "#F2726B",
    "green":  "#3ECF8E",
    "yellow": "#F2C063",
    "blue":   "#60A5FA",
    "magenta":"#C084FC",
    "cyan":   "#5EEAD4",
    "white":  "#E5E7EB",
    "reset":  "RESET"
  },

  // Colors used by $N placeholders inside BinaryArt.txt.
  // Not limited to $1-$15 — add as many as you want.
  // Each value supports the same 3 formats as "colors" above,
  // PLUS you can reference a name already defined in "colors".
  "ascii_color_prefixes": {
    "$1":  "#F2726B",      // direct hex
    "$2":  "46,207,142",   // direct R,G,B
    "$3":  "cyan",         // name lookup -> resolves via "colors"
    "$4":  "\u001b[35m",   // raw ANSI escape, used as-is
    "$15": "RESET",        // special reset value
    "$16": "magenta"       // user-defined, beyond the old fixed 15 slots
  },

  "art": {
    "Ascii_Art": { "enabled": true, "padding_up": 0, "padding_left": 0, "padding_right": 0 }
  }
}
```

### Supported value formats (identical to `"colors"`)

| Format              | Example              | Notes                                  |
|----------------------|-----------------------|------------------------------------------|
| Hex                 | `"#F2726B"`           | Converted to 24-bit truecolor ANSI       |
| Plain RGB           | `"46,207,142"`        | Converted to 24-bit truecolor ANSI       |
| Raw ANSI escape     | `"\u001b[35m"`        | Used exactly as written                  |
| `"RESET"`           | `"RESET"`             | Special-cased to `\033[0m`               |
| **Name reference**  | `"cyan"`              | **New fallback**: looked up in `"colors"`|

### Key rules

- **Keys** may be written with or without the leading `$` (`"$1"` or `"1"`)
  — both are accepted, the `$` is stripped before parsing the number.
- **Not capped at 15.** Any integer key works (`"$16"`, `"$42"`, ...), so
  users can add as many custom slots as their `BinaryArt.txt` references.
- **Partial overrides only affect the slots you specify.** The built-in
  15-color default table is always the starting point; JSON entries
  override/add slots on top of it. Omitting `$1`–`$14` and only setting
  `$15` still leaves `$1`–`$14` working exactly as before.
- **Resolution order per value:** try direct parse (hex / RGB / escape /
  `RESET`) first; if that fails, fall back to a name lookup against the
  `"colors"` palette (so `"$3": "cyan"` works whether or not `"cyan"` is a
  hex value or something else entirely — same behavior as every other
  color field in the app, e.g. `prefix_color`, `label.color`).
- **Absent section = zero behavior change.** If `ascii_color_prefixes` is
  missing entirely, the map is exactly the old hardcoded 15-color table.

## Code changes

### 1. `config_management.h`

- Added member: `std::map<int, std::string> m_asciiColorMap;`
- Added private method: `void loadAsciiColorPrefixes();`
- Added public method: `const std::map<int, std::string>& getAsciiColorMap() const;`

### 2. `config_management.cpp`

- New method `loadAsciiColorPrefixes()`:
  - Seeds `m_asciiColorMap` with the original 15 hardcoded defaults.
  - If `"ascii_color_prefixes"` is absent/not an object, returns immediately
    (defaults untouched).
  - Otherwise iterates each key/value pair, strips a leading `$` if present,
    parses the key as an integer, resolves the value via `parseColorValue()`
    first and `m_colors` name-lookup second, and **overrides only that slot**
    in `m_asciiColorMap`.
- New method `getAsciiColorMap()` — simple const accessor.
- `loadPlatformConfig()` now calls `loadAsciiColorPrefixes()` right after
  `loadColorPalette()` and before `loadEmojiSettings()`. Order matters:
  the name-lookup fallback depends on `m_colors` already being populated.

### 3. `AsciiArt.h`

- Added `#include <map>`.
- Added public method: `void setColorMap(const std::map<int, std::string>& map);`
- Added private member: `std::map<int, std::string> colorMap;`
  (empty by default — signals "use built-in defaults").

### 4. `AsciiArt.cpp`

- Renamed the old static `colorMap` to `kDefaultColorMap` (same 15 values,
  same meaning: the fallback table).
- `processColorCodes()` now takes the color table as a parameter instead of
  reading a fixed global, so it can resolve against whichever table is
  active (injected or default).
- `loadArtFromPath()` and `loadArtFromEmbedded()` each pick the active table
  before parsing: `colorMap.empty() ? kDefaultColorMap : colorMap` — i.e. if
  no map was ever injected via `setColorMap()`, behavior is identical to the
  pre-change code.
- Added `AsciiArt::setColorMap()` — simple setter, stores the injected map
  for the next `loadFromFile()` / `loadArtFromEmbedded()` call.

### 5. `main.cpp`

- One line added, right before `art.loadFromFile()`:
  ```cpp
  art.setColorMap(config.getAsciiColorMap());
  ```
  This is the only wiring point connecting `ConfigManager` to `AsciiArt` for
  this feature — `AsciiArt` itself still has zero dependency on
  `ConfigManager`, preserving the original single-responsibility separation
  documented in `AsciiArt.cpp`'s header comment.

## Data flow

```
JSON "ascii_color_prefixes"  (+ "colors" for name fallback)
            |
            v
ConfigManager::loadAsciiColorPrefixes()
   - starts from 15 built-in defaults
   - overrides/extends per JSON entry
            |
            v
ConfigManager::getAsciiColorMap()  ->  main.cpp
            |
            v
AsciiArt::setColorMap()
            |
            v
AsciiArt::loadFromFile() / loadArtFromEmbedded()
   - picks injected map, or kDefaultColorMap if never set
            |
            v
processColorCodes(line, activeColorMap)
   - resolves $N tokens in BinaryArt.txt / embedded default art
```

## Backward compatibility guarantees

- No `ascii_color_prefixes` section → `m_asciiColorMap` == old hardcoded
  table → identical rendered output.
- `AsciiArt::colorMap` left unset (e.g. any code path that never calls
  `setColorMap()`) → falls back to `kDefaultColorMap` → identical output.
- Existing `"colors"` section, `loadColorPalette()`, and every other config
  path (`getColor`, `getNestedColor`, emoji styling, layout order, etc.)
  are untouched by this change.
- `AsciiArt`'s public API only gained one new optional method
  (`setColorMap`); every existing caller and every existing signature is
  unchanged.



# `show_colors` — ASCII Art Color Toggle

## Summary

Adds a single boolean switch, `ascii_color_prefixes.show_colors`, that turns
the `$N` color placeholders in `BinaryArt.txt` on or off globally, without
touching the art file itself or removing the per-`$N` color definitions.

- `true` (or key/section omitted) — current behavior, unchanged. Every `$N`
  resolves to its mapped ANSI color.
- `false` — every `$N` still gets consumed and stripped from the rendered
  output, but resolves to no color at all, so the art prints in the
  terminal's current/default foreground instead of the palette.

## Files changed

| File | Change |
|---|---|
| `config_management.h` | New private member `m_asciiShowColors`; new public getter `isAsciiShowColorsEnabled()` |
| `config_management.cpp` | `loadAsciiColorPrefixes()` reads `show_colors` and, when `false`, clears every value in `m_asciiColorMap`; new `isAsciiShowColorsEnabled()` definition |
| `AsciiArt.cpp` / `AsciiArt.h` | **No changes.** `AsciiArt` only ever consumes the map handed to it via `setColorMap()`; it has no awareness of the flag |
| `main.cpp` | **No changes.** |

## JSON usage

```jsonc
"ascii_color_prefixes": {
  "show_colors": true,   // or false — add/remove this line
  "$1":  "#F2726B",
  "$2":  "46,207,142",
  "$3":  "cyan",
  "$4":  "\u001b[35m",
  "$5":  "magenta",
  "$6":  "cyan",
  "$7":  "white",
  "$8":  "#F2726B",
  "$9":  "#3ECF8E",
  "$10": "#F2C063",
  "$11": "#60A5FA",
  "$12": "#C084FC",
  "$13": "#5EEAD4",
  "$14": "#E5E7EB",
  "$15": "RESET"
}
```

## Code changes

### `config_management.h`

```cpp
// private members
std::map<int, std::string> m_asciiColorMap;
bool m_asciiShowColors{true};   // NEW

// public API
const std::map<int, std::string>& getAsciiColorMap() const;
bool isAsciiShowColorsEnabled() const;   // NEW
```

### `config_management.cpp`

```cpp
void ConfigManager::loadAsciiColorPrefixes() {
    m_asciiColorMap = { /* ...15 built-in defaults, unchanged... */ };
    m_asciiShowColors = true;   // reset every (re)load

    if (!m_config.contains("ascii_color_prefixes") || !m_config["ascii_color_prefixes"].is_object())
        return;

    const auto& section = m_config["ascii_color_prefixes"];

    if (section.contains("show_colors") && section["show_colors"].is_boolean())
        m_asciiShowColors = section["show_colors"].get<bool>();

    for (auto& [key, value] : section.items()) {
        if (key == "show_colors") continue;   // not a $N entry
        // ...unchanged per-$N parsing...
    }

    if (!m_asciiShowColors) {
        for (auto& [num, ansi] : m_asciiColorMap) {
            ansi.clear();   // keys 1..15 remain; values become ""
        }
    }
}

bool ConfigManager::isAsciiShowColorsEnabled() const {
    return m_asciiShowColors;
}
```

The clearing step runs **last**, so it overrides both the 15 built-in
defaults and any custom per-`$N` overrides from the JSON — `show_colors`
is a single switch that wins over everything else in the section.

## Behavior reference

| Condition | `m_asciiShowColors` | `$N` in art |
|---|---|---|
| `ascii_color_prefixes` section missing entirely | `true` (default) | Colored, as today |
| Section present, `show_colors` key missing | `true` (default) | Colored, as today |
| `show_colors` present but not a boolean (e.g. `"yes"`, `1`, `null`) | `true` (default) | Colored, as today |
| `"show_colors": true` | `true` | Colored, as today |
| `"show_colors": false` | `false` | `$N` tokens still consumed/removed, but resolve to `""` — no color escape inserted |

`true` is always the fallback: the flag is opt-out, not opt-in. You only
get uncolored art by explicitly writing `"show_colors": false`.

## `show_colors: false` with color flags still present in the art

`processColorCodes()` in `AsciiArt.cpp` never copies the literal `$N`
text into its output — it only appends the text *around* each match and,
if the number is found in the color map, whatever string that number
maps to:

```cpp
while (std::regex_search(remaining, match, colorCodeRegex)) {
    processed += match.prefix();                 // text before $N
    auto it = colors.find(colorNum);
    if (it != colors.end()) processed += it->second;   // "" when show_colors=false
    remaining = match.suffix();                   // text after $N — "$N" itself is dropped either way
}
processed += remaining + "\033[0m";
```

So:

- The `$N` marker itself is **always** stripped from what's printed —
  with `show_colors` on or off, you never see a literal `$1` in the art.
- With `show_colors: false`, the lookup for `$N` still succeeds (the key
  exists in `m_asciiColorMap`), so no warning/fallback path fires — the
  substitution is just an empty string. The art segment after that point
  keeps whatever color was already active (or the terminal's default, if
  none was), instead of switching to the mapped palette color.
- The trailing `\033[0m` reset at the end of the line is unconditional
  and still fires regardless of `show_colors`.

**In short:** `show_colors: false` does not require editing `BinaryArt.txt`
— it neutralizes every existing `$N` flag at render time, leaving the art
in the terminal's plain/default color instead of your defined palette.
  