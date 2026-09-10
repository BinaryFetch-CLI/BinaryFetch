# BinaryFetch — JSON-Driven Ordering System

This document explains how to make **section order** and **sub-field order**
fully configurable from JSON, without recompiling BinaryFetch.

---

## 1. The Problem

BinaryFetch is almost entirely JSON-driven — colors, prefixes, labels,
visibility (`show`/`enabled`), spacing, visualizers — all of it comes from
`BinaryFetch_Config.json`.

**Except for one thing: order.**

Two kinds of order are currently hardcoded into `main.cpp`:

| Order type | Example | Controlled by |
|---|---|---|
| **Section order** | CPU block prints before GPU block | physical position of `if` blocks in `main.cpp` |
| **Field order** | `uptime` prints before `cpu_usage` before `ram_usage` | physical position of `if` blocks *inside* a section |

Because both are baked into source-code order, changing them today means
editing `main.cpp` and recompiling. This document describes a pattern to fix
both, using only C++ features already included in the project
(`<functional>`, `<map>`, `<vector>`).

---

## 2. The Core Idea

> Wrap each printable unit (section or field) in a `std::function<void()>`,
> store it in a `std::map<std::string, std::function<void()>>` keyed by name,
> then call those functions in whatever sequence a JSON array specifies.

Nothing about *what* gets printed changes. Only the *sequence in which the
functions are invoked* changes — and that sequence now comes from JSON
instead of from where the code sits in the file.

This same pattern is applied twice, at two different scopes:

- **Scope A — Top level:** reorder whole sections (`os_info`,
  `detailed_processor`, `detailed_graphics_card`, `performance_info`, etc.)
- **Scope B — Inside a section:** reorder the fields within one section
  (e.g. `uptime`, `cpu_usage`, `ram_usage`, `disk_usage`, `gpu_usage` inside
  `performance_info`)

---

## 3. Scope A — Reordering Whole Sections

### 3.1 Before (current behavior)

```cpp
if (config.isEnabled("detailed_processor")) {
    // ... print CPU info ...
}

if (config.isEnabled("detailed_graphics_card")) {
    // ... print GPU info ...
}
```

CPU always prints before GPU — fixed by source order.

### 3.2 After (JSON-driven)

**Step 1 — Wrap each section body in a lambda, stored in a map:**

```cpp
std::map<std::string, std::function<void()>> sections;

sections["detailed_processor"] = [&]() {
    if (!config.isEnabled("detailed_processor")) return;
    // ...unchanged section body...
};

sections["detailed_graphics_card"] = [&]() {
    if (!config.isEnabled("detailed_graphics_card")) return;
    // ...unchanged section body...
};

// repeat for every remaining section: os_info, display_info,
// bios_mb_info, user_info, performance_info, audio_power_info,
// detailed_system_memory, detailed_disk_storage,
// detailed_network_connection, and all compact_* sections.
```

`[&]` captures everything already declared in `main()` by reference
(`cpu`, `obj_gpu`, `config`, `lp`, `r`, etc.), so the lambda bodies need **zero
changes** from their current form — copy/paste as-is.

**Step 2 — Add a `layout` array to the JSON config:**

```json
"layout": [
  "header_settings",
  "compact_date_and_time",
  "compact_operating_system",
  "compact_processor",
  "compact_graphics_card",
  "compact_display_monitor",
  "compact_system_memory",
  "compact_audio_devices",
  "compact_resource_usage",
  "compact_user_account",
  "compact_network_connection",
  "compact_disk_storage",
  "detailed_system_memory",
  "detailed_disk_storage",
  "detailed_network_connection",
  "os_info",
  "detailed_processor",
  "detailed_graphics_card",
  "display_info",
  "bios_mb_info",
  "user_info",
  "performance_info",
  "audio_power_info"
]
```

**Step 3 — Loop over the array and call each named section:**

```cpp
for (const auto& key : config.getLayoutOrder()) {
    auto it = sections.find(key);
    if (it != sections.end()) {
        it->second();   // run that section's lambda
    }
}
```

### 3.3 Result

To swap GPU above CPU, a user edits only JSON:

```json
"layout": ["detailed_graphics_card", "detailed_processor"]
```

No recompilation. No source edits.

---

## 4. Scope B — Reordering Fields Inside a Section

The same problem exists one level deeper. Inside `performance_info`, the
four fields (`uptime`, `cpu_usage`, `ram_usage`, `disk_usage`) are separate
`if` blocks, printed strictly in file order.

### 4.1 Before (current behavior)

```cpp
if (config.isEnabled("performance_info")) {
    // header ...
    if (...) { /* uptime */ }
    if (...) { /* cpu_usage */ }
    if (...) { /* ram_usage */ }
    if (...) { /* disk_usage */ }
    if (...) { /* gpu_usage */ }
}
```

### 4.2 After (JSON-driven)

Apply the identical map + lambda + loop pattern — just scoped locally to
this section instead of globally to the whole program.

```cpp
if (config.isEnabled("performance_info")) {

    int spacing = config.getNestedInt("performance_info", "top_line_spacing", 0);
    for (int n = 0; n < spacing; n++) { lp.push(""); }

    // Header always prints first — not reorderable, matches its role
    // as a section title rather than a data field.
    if (config.getNestedBool("performance_info", "header.show", true)) {
        // ...unchanged header code...
    }

    // ---- Register each field as a named lambda ----
    std::map<std::string, std::function<void()>> fields;

    fields["uptime"] = [&]() {
        if (!config.getNestedBool("performance_info", "fields.uptime.show", true)) return;
        // ...unchanged uptime body...
    };

    fields["cpu_usage"] = [&]() {
        if (!config.getNestedBool("performance_info", "fields.cpu_usage.show", true)) return;
        // ...unchanged cpu_usage body, including makeVisualizer() call...
    };

    fields["ram_usage"] = [&]() {
        if (!config.getNestedBool("performance_info", "fields.ram_usage.show", true)) return;
        // ...unchanged ram_usage body...
    };

    fields["disk_usage"] = [&]() {
        if (!config.getNestedBool("performance_info", "fields.disk_usage.show", true)) return;
        // ...unchanged disk_usage body...
    };

    fields["gpu_usage"] = [&]() {
        if (!config.getNestedBool("performance_info", "fields.gpu_usage.show", true)) return;
        // ...unchanged gpu_usage body...
    };

    // ---- Run fields in the order JSON specifies ----
    std::vector<std::string> defaultOrder =
        {"uptime", "cpu_usage", "ram_usage", "disk_usage", "gpu_usage"};

    auto order = config.getStringArray("performance_info", "order", defaultOrder);

    for (const auto& key : order) {
        auto it = fields.find(key);
        if (it != fields.end()) it->second();
    }
}
```

### 4.3 JSON change

Add an `"order"` array at the top of the section's block, as a **sibling** of
`"fields"` — not inside it:

```json
"detailed_resource_usage": {
  "enabled": true,
  "order": ["gpu_usage", "cpu_usage", "ram_usage", "disk_usage"],
  "top_line_spacing": 0,
  "header": { "...": "unchanged" },
  "fields": {
    "uptime":     { "...": "unchanged" },
    "cpu_usage":  { "...": "unchanged" },
    "ram_usage":  { "...": "unchanged" },
    "disk_usage": { "...": "unchanged" },
    "gpu_usage":  { "...": "unchanged" }
  }
}
```

> ⚠️ **Common mistake:** `"order"` must use **square brackets `[ ]`**
> (a JSON array), never curly braces `{ }` (a JSON object). Curly braces will
> fail to parse.
>
> ```json
> "order": {"gpu_usage","cpu_usage"}   ❌ invalid JSON
> "order": ["gpu_usage","cpu_usage"]   ✅ correct
> ```

### 4.4 Result

If `uptime` is left out of `"order"`, it simply never prints — same
`fields.find(key)` guard that quietly ignores unregistered/misspelled keys
applies here too (see §6.2 for how to handle that safely).

---

## 5. Required `ConfigManager` Additions

Two small helper functions cover both scopes. They can share one
implementation, since a top-level `layout` array and a per-section `order`
array are structurally identical — both are just "a list of strings read
from some JSON path, with a fallback if missing."

```cpp
// Reads a JSON array of strings at `module.path`.
// Returns `fallback` if the key is missing, empty, or not an array.
std::vector<std::string> getStringArray(const std::string& module,
                                         const std::string& path,
                                         const std::vector<std::string>& fallback) const;

// Convenience wrapper for the top-level "layout" key.
std::vector<std::string> getLayoutOrder() const {
    static const std::vector<std::string> defaultLayout = {
        "header_settings", "compact_date_and_time", /* ... */
    };
    return getStringArray("", "layout", defaultLayout);
}
```

Both functions should:

- Tolerate a missing key (return the fallback silently — this keeps old
  configs working with zero errors, consistent with the project's
  self-healing philosophy).
- Tolerate unknown/misspelled entries in the array (skip them at the call
  site — see §6.2).
- Never throw on malformed JSON; log a warning at most.

---

## 6. Design Notes & Gotchas

### 6.1 Naming consistency is mandatory

The string used as a map key **must exactly match** the string used in the
JSON array. Right now some sections have mismatched names between their
C++ `isEnabled(...)` string and their documented JSON block name — for
example, the code checks `config.isEnabled("os_info")` while the config
file's top-level key is `detailed_operating_system`.

Before implementing ordering, **pick one canonical name per section** and
use it consistently in three places:

1. The `sections[...]` / `fields[...]` map key
2. The top-level JSON object key (or nested field key)
3. Every entry in `layout` / `order` that refers to it

If these three drift out of sync, the lookup (`sections.find(key)`) simply
returns `end()` and that section silently never prints — no crash, no error,
just a missing block that's hard to notice. Recommend adding a debug warning
when a `layout`/`order` entry doesn't resolve to anything registered.

### 6.2 Unknown or missing keys should not crash

```cpp
for (const auto& key : order) {
    auto it = fields.find(key);
    if (it != fields.end()) {
        it->second();
    }
    // else: silently skip (or optionally log a warning in DEV_MODE)
}
```

A user typo in `order` (e.g. `"cpu_usag"` instead of `"cpu_usage"`) should
never crash the program — it should just mean that field doesn't print,
same as if `"show": false` had been set.

### 6.3 Headers are usually pinned, not reorderable

In the example above, `performance_info`'s header always prints **before**
the field loop starts — it is deliberately *not* included in the `fields`
map. This mirrors its role as a section title rather than a data row.
Most sections should follow this rule: header stays pinned to the top,
only the data fields underneath become reorderable via `"order"`.

If a section genuinely needs its header to be reorderable too (rare), it
can be added to the map like any other field and included in `"order"`.

### 6.4 Backward compatibility

Both `layout` and each section's `order` are **optional** keys. If absent,
`getStringArray(...)` returns a hardcoded default vector matching the
current (pre-ordering) source-code sequence. This means:

- Existing configs generated before this feature continue to work with
  identical output.
- `DEV_MODE`'s self-healing config generation (§ config_management.h) does
  not need to force-inject `layout`/`order` into every config immediately —
  they can be added when a user wants to customize, and omitted otherwise.

### 6.5 Two independent axes, not one

Section order and field order are controlled by **two separate arrays** at
two separate scopes:

```
layout: [...]                     <- controls order of SECTIONS
  detailed_resource_usage:
    order: [...]                  <- controls order of FIELDS inside this section only
```

A field's `order` array only affects fields *within that section's own
JSON block* — it has no effect on where the section itself appears relative
to other sections. Both need to be set independently if a user wants full
control over the final layout.

---

## 7. Summary Table

| | Top-level sections | Fields inside a section |
|---|---|---|
| **What it controls** | Which whole block prints first (CPU block vs GPU block vs Memory block, etc.) | Which line prints first *within* one block (e.g. uptime vs cpu_usage vs ram_usage) |
| **Map used** | One global `sections` map, built once in `main()` | One local `fields` map, built fresh inside each section's `if` block |
| **JSON key** | Top-level `"layout"` array | Per-section `"order"` array, sibling of `"fields"` |
| **Default behavior if key missing** | Falls back to current source-code order | Falls back to current source-code order |
| **Recompile needed to reorder?** | No | No |

---

## 8. Rollout Checklist

- [ ] Add `getStringArray(module, path, fallback)` to `ConfigManager`
- [ ] Add `getLayoutOrder()` convenience wrapper
- [ ] Wrap every top-level section body in a `sections[...]` lambda
- [ ] Add the final `for (const auto& key : config.getLayoutOrder())` loop at the bottom of `main()`, replacing the current sequential `if` blocks
- [ ] For each section that has multiple sub-fields (`performance_info`, `detailed_processor`, `detailed_graphics_card`, `detailed_network_connection`, `os_info`, `bios_mb_info`, `user_info`, etc.), repeat the pattern locally with a `fields[...]` map and that section's own `"order"` array
- [ ] Reconcile mismatched section names between code and JSON (§6.1) before shipping, so every `layout`/`order` entry has exactly one matching map key
- [ ] Confirm unresolved/misspelled keys are skipped silently, not crashed on (§6.2)
- [ ] Add default `layout` (and default per-section `order`) arrays matching current output, so existing configs are unaffected
- [ ] Update any user-facing config documentation/comments to describe the new `layout` and `order` keys