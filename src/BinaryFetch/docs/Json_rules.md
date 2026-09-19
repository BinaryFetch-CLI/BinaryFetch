
# Config Field Rules — Full Reference

This document is the single source of truth for how every config
block in this file is shaped. If you're ever unsure where a key
goes, this is the page to check — not a previous section, not
guesswork.

---

## 1. The core idea

Everything in this config is built from a small number of building
blocks, reused everywhere:

- **Section** — a top-level named block (`compact_date_and_time`,
  `compact_processor`, etc.)
- **Module** — a named block *inside* a section (`time`, `date`,
  `week` inside `compact_date_and_time`)
- **Sub-module** — a named block *inside* a module (`hour`, `minute`,
  `second` inside `time`), and any level nested deeper than that
- **Label** — the static caption text before a value (e.g. `"Time: "`)
- **Value** — whatever the actual data is (e.g. the hour, the CPU name)

That's it. There is no separate "separator" block anywhere. Joining
text between two sibling fields (like the `:` between hour and
minute) is just a `suffix` on the field before it, or a `prefix` on
the field after it — same mechanism as brackets, same as everything
else.

---

## 2. Depth — how to tell what "level" something is at

Depth = how many `{ }` you are nested inside the section.

```
"compact_date_and_time": {        <- depth 0 (the section)
    "time": {                     <- depth 1 (a module)
        "hour": {                 <- depth 2 (a sub-module)
            "value": { ... }      <- not a depth level — see §4
        }
    }
}
```

Depth is the only thing that decides which keys are legal at a
given spot. Not the name, not what it "feels like" it should be.

---

## 3. Visibility — every depth is toggled, always `"enabled"`

**Every depth has a toggle. There is only ever one toggle name:
`"enabled"`.** There is no `"show"` anywhere in this config —
`enabled` is used at every level, no matter how deep.

| Depth                     | Visibility key            |
|-----------------------------|----------------------------|
| 0 — Section                 | `"enabled": true / false` |
| 1 — Module                  | `"enabled": true / false` |
| 2 — Sub-module               | `"enabled": true / false` |
| 3, 4, ... — deeper nesting   | `"enabled": true / false` |
| `label` / `value`           | **never toggled**          |

`label` and `value` are the only things that never get `enabled` —
they aren't a depth, they're the fixed two-part shape attached to
whatever depth they belong to. Whether something appears at all is
always decided one level above the label/value that describes it.

---

## 4. Label / Value — the fixed shape

Any sub-module (a leaf that actually prints something) is made of
up to two parts:

```jsonc
"label": {
    "prefix": "",
    "prefix_color": "",
    "text": "",
    "color": "",
    "suffix": "",
    "suffix_color": ""
},
"value": {
    "prefix": "",
    "prefix_color": "",
    "color": "",
    "suffix": "",
    "suffix_color": ""
}
```

- `label` = the static caption (optional — most sub-modules don't need one)
- `value` = wraps whatever the getter function returns

Both can carry their own `prefix`/`suffix` completely independently.

**Any key you don't need, just delete.** An omitted key behaves as
empty — nothing breaks, and you can add it back anytime.

---

## 5. Prefix / Suffix — where they're allowed

This is the one rule most likely to be gotten wrong, so it gets its
own section.

> **`prefix` / `suffix` are bare (standalone) keys ONLY at depth 0
> — the section itself.**
> Everywhere else, `prefix`/`suffix` must live inside `label` or
> `value` — never bare next to `enabled`, and never inside a
> dedicated "separator" block.

```jsonc
"compact_date_and_time": {      // depth 0 — bare prefix allowed
    "enabled": true,
    "prefix": "╭📅 ",
    "prefix_color": "red",

    "time": {                   // depth 1 — NO bare prefix/suffix
        "enabled": true,
        "label": { "text": "Time", "suffix": ": " },

        "hour": {                // depth 2 — NO bare prefix/suffix
            "enabled": true,
            "value": { "prefix": "(", "color": "bright_blue" }
            //          ^ prefix lives INSIDE value, never bare
        },
        "second": {
            "enabled": true,
            "value": { "suffix": ")", "color": "bright_blue" }
        }
    }
}
```

**Why this matters:** brackets, parens, and joining text between
fields are not a special concept — they're all just text sitting in
some field's own `value.prefix` / `value.suffix` (or `label.prefix`
/ `label.suffix`). There is never a second place this text could be
hiding.

---

## 6. Joining text between fields (no separate block)

Anything that used to be a "separator" — a `:` between hour and
minute, a `-` between two names — is just a suffix on the field
before it, or a prefix on the field after it. Pick whichever side
makes sense; don't add a third block to hold it.

```jsonc
"hour": {
    "enabled": true,
    "value": { "prefix": "(", "color": "bright_blue", "suffix": ":" }
},
"minute": {
    "enabled": true,
    "value": { "color": "bright_blue", "suffix": ":" }
},
"second": {
    "enabled": true,
    "value": { "color": "bright_blue", "suffix": ")" }
}
```
→ renders `(14:32:07)` — no separator block, no group wrapper. Each
field owns the exact text immediately touching it.

If a field is disabled (`enabled: false`), its neighbor's
suffix/prefix may need manual adjusting so you don't end up with a
stray `:` — this is a small manual trade-off in exchange for zero
special-cased join logic anywhere in the config or the code.

### The same trick covers shared brackets across independent fields

```jsonc
"cores": {
    "value": { "prefix": "(", "color": "cyan", "suffix": "C/" }
},
"threads": {
    "value": { "color": "cyan", "suffix": "t)" }
}
```
→ renders `(4C/8t)` — two fully independent, freely-reorderable
fields, no shared group object required.

---

## 7. `order` — sequencing, separate from visibility

At depth 0, an `"order": [...]` array lists which depth-1 keys
render, and in what sequence.

`order` controls **sequence**, `enabled` controls **visibility** —
they are two separate concerns. A key can be listed in `order` but
still not render if its own `enabled` is `false`. This is
consistent at every depth: `order` only ever exists at the depth
above the things it's sequencing, while `enabled` lives on the
thing itself.

```jsonc
"order": ["time", "date", "week", "leap_year"]
```

---

## 8. Quick lookup table

| What you're editing               | Depth | Visibility key | Can have bare prefix/suffix? |
|-------------------------------------|-------|-----------------|--------------------------------|
| A section (`compact_processor`)     | 0     | `enabled`       | Yes                             |
| A module (`time`, `date`)           | 1     | `enabled`       | No — use `label`/`value`        |
| A sub-module (`hour`, `cores`)       | 2+    | `enabled`       | No — use `label`/`value`        |
| `label` block                       | —     | never toggled   | Yes, but only inside `label`    |
| `value` block                       | —     | never toggled   | Yes, but only inside `value`    |
| "separator"                         | —     | **does not exist as a block** — it's just prefix/suffix on neighboring fields |

---

## 9. Full worked example

```jsonc
"compact_date_and_time": {           // depth 0
    "enabled": true,
    "prefix": "╭📅 ",
    "prefix_color": "red",
    "order": ["time"],

    "time": {                        // depth 1
        "enabled": true,
        "label": { "text": "Time", "color": "red", "suffix": ": " },

        "hour": {                    // depth 2
            "enabled": true,
            "value": { "prefix": "(", "prefix_color": "blue", "color": "bright_blue", "suffix": ":" }
        },
        "minute": {                  // depth 2
            "enabled": true,
            "value": { "color": "bright_blue", "suffix": ":" }
        },
        "second": {                  // depth 2
            "enabled": true,
            "value": { "color": "bright_blue", "suffix": ")", "suffix_color": "blue" }
        }
    }
}
```

Renders: `╭📅 Time: (14:32:07)`

---

## 10. Why this exists

Before this schema, every section invented its own keys —
`unit`, `at_symbol`, `brackets.open`, `separator.text`,
`show`, `value_suffix` — all meaning similar things but spelled
differently everywhere. That made every section something you had
to re-learn from scratch.

Now there is exactly **one shape**, reused everywhere:
- Depth tells you what keys are legal.
- Every depth is toggled the same way — `enabled` — with no
  second toggle name to remember.
- `label`/`value` are the only two building blocks that actually
  print something.
- `prefix`/`suffix` are only ever bare at depth 0 — everywhere else
  they live inside `label` or `value`, including all joining text
  that used to live in a separate "separator" block.

Anyone — human or AI — reading any section of this config for the
first time should be able to predict every other section without
being told twice.
```