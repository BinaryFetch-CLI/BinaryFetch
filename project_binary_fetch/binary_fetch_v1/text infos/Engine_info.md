

# ASCII Art Generator Engine — Developer Documentation

## Purpose

The ASCII Art Generator Engine is responsible for **ensuring the presence of user ASCII art** in the proper directory and **copying a default bundled version** when needed. This allows BinaryFetch to function reliably on first run, supports user customization, and separates visual assets from program logic.

---

## Target User File

```
C:\Users\<USERNAME>\AppData\BinaryFetch\AsciiArt.txt
```

* All runtime operations interact with this path.
* The engine **never modifies the bundled default**, only the user copy.
* Bundled default: `Default_Ascii_Art.txt` is **inside the source code / project assets**.

---

## Workflow: Detailed Step-by-Step

### Step 0 — Initialization

* `main.cpp` prepares `AsciiArt` loader object.
* Sets path for:

  * User ASCII: `%LOCALAPPDATA%\BinaryFetch\AsciiArt.txt`
  * Bundled default: `<EXE_DIR>/Default_Ascii_Art.txt`

---

### Step 1 — Main requests ASCII content

```text
main -> ASCII Engine: load user ASCII at path
```

* `main` calls engine to check/load `AsciiArt.txt`.
* Engine attempts to **open the user file**.

---

### Step 2 — Engine reports missing file

```text
ASCII Engine -> main: file missing
```

* Engine returns a **status** (e.g., `FileNotFound` or `Missing`) to `main`.
* Engine does **not** create the file automatically at this stage.
* `main` now knows that the ASCII asset is absent.

---

### Step 3 — Main commands engine to create file

```text
main -> ASCII Engine: "Create AsciiArt.txt from default"
```

* `main` explicitly instructs the engine to generate the missing file.
* This maintains **centralized control** and allows optional logging or user notification in `main`.

---

### Step 4 — Engine generates the file

1. Engine ensures parent directory exists (`BinaryFetch` in AppData).
2. Creates `AsciiArt.txt` in that directory.
3. Opens the **bundled `Default_Ascii_Art.txt`** for reading.
4. Copies content from default into `AsciiArt.txt`:

   * Use atomic write: write to temp file → flush → rename to `AsciiArt.txt`.
5. Returns **status** to `main`:

   * `CreatedFromDefault` on success
   * `IoError` / `PermissionDenied` on failure

---

### Step 5 — Main re-attempts loading

* `main` now tries to load the newly created `AsciiArt.txt`.
* On success → normal rendering begins.
* On failure → fallback or error message (optional, in-memory default or console log).

---

### Step 6 — Summary diagram

```text
main -> ASCII Engine : load user ASCII
ASCII Engine -> main  : file missing
main -> ASCII Engine  : create from default
ASCII Engine -> main  : CreatedFromDefault
main -> ASCII Engine  : load user ASCII again
ASCII Engine -> main  : Success -> continue normal execution
```

---

## Developer API — Recommended

### Status Enum

```cpp
enum class AsciiGeneratorStatus {
    Success,               // File exists or successfully created
    FileAlreadyExists,     // User file already present
    CreatedFromDefault,    // Created new user file from bundled default
    MissingBundledDefault, // Default_Ascii_Art.txt missing in source/bundle
    IoError,               // Generic I/O error
    PermissionDenied,      // Cannot write to target location
};
```

### Main function interface

```cpp
// Check or create user ASCII art
AsciiGeneratorStatus ensureUserAsciiExists(
    const std::filesystem::path& userPath,
    const std::filesystem::path& bundledDefaultPath);
```

* **Input:** user path + bundled default path
* **Output:** status code
* **Behavior:** creates file **only when instructed** by `main`.

---

## Implementation Notes

* **Idempotence:** repeated calls must not overwrite existing user files.
* **Atomic writes:** temp file + rename to avoid partial file corruption.
* **Error handling:** engine returns error codes; `main` decides how to notify the user.
* **Bundled default:** always UTF-8, no BOM, and included inside source for compilation.
* **Directory creation:** engine creates missing directories automatically.

---

## Testing / Edge Cases

1. **User file exists:** status `FileAlreadyExists`; content remains untouched.
2. **User file missing, default present:** status `CreatedFromDefault`; file matches bundled default.
3. **Bundled default missing:** status `MissingBundledDefault`; engine does not create user file.
4. **Permission denied:** engine returns `PermissionDenied`; `main` logs and skips creation.
5. **Concurrent runs:** temp files should be isolated; rename failures handled gracefully.

---

This structure clearly documents the **request/response workflow** between `main` and the ASCII engine and gives developers a complete blueprint for **first-run ASCII asset creation**.

---

