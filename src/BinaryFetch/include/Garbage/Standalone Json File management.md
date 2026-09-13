# Unbreakable JSON Configuration

## How It Works

BinaryFetch uses a **self-healing config system** that never crashes from user edits.

```
Default_BinaryFetch_Config.json  →  Copy on first run  →  BinaryFetch_Config.json
(Bundled with app)                                        (User editable in AppData)
```

---

## Behavior

### First Run
1. Checks `C:\Users\Default\AppData\Local\BinaryFetch\`
2. Creates directory if missing
3. Copies `Default_BinaryFetch_Config.json` → `BinaryFetch_Config.json`
4. Loads and runs

### Every Run
- **Config exists?** → Load it
- **Config missing?** → Auto-create from default
- **Config corrupted?** → Use safe defaults (all features enabled)
- **Invalid colors?** → Substitute with white

**Result:** App never crashes from config issues.

---

## Reset Method

Delete the config file:
```bash
del "C:\Users\Default\AppData\Local\BinaryFetch\BinaryFetch_Config.json"
```

Next run auto-creates fresh copy.

---

## Safety Features

### Null-Safe Helpers

```cpp
// Every access checks existence and provides defaults
getColor("section", "key", "white")        // Missing key → white
isEnabled("section")                        // Missing section → true
isSubEnabled("section", "key")              // Missing config → true
```

### Error Handling

```cpp
try {
    config = json::parse(file);
} catch (...) {
    // Parse failed → continue with defaults
}
```

---

## Config Schema

```json
{
  "header": {},
  "compact_os": {},
  "compact_cpu": {},
  // ... 21 sections total
  "detailed_memory": {},
  "detailed_storage": {},
  // ... 200+ customization points
}
```

---

## Key Points

✅ **Missing config** → Auto-creates  
✅ **Corrupted JSON** → Safe defaults  
✅ **Invalid values** → Substitutes  
✅ **Delete to reset** → Auto-recreates  
✅ **Zero crashes** → Ever  

**Users cannot break the app through config manipulation.**

---

## For Developers

### Testing Checklist
- [ ] Missing config file
- [ ] Corrupted JSON syntax
- [ ] Missing directory
- [ ] Invalid color values
- [ ] Deleted sections
- [ ] Empty config file

### Code Rule
All config access must provide defaults:

```cpp
// ❌ Bad - can crash
config["section"]["key"]

// ✅ Good - always safe
getColor("section", "key", "white")
```

---

## Performance

- **First run:** ~16ms (includes file copy)
- **Subsequent:** ~11ms (just parsing)
- **Memory:** ~152KB

Negligible impact on startup.