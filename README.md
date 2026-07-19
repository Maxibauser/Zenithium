# Zenithium

A modern, cross-platform code IDE built with C++20 and Qt 6.

Zenithium aims to be a small, fast, and clean editor — thoughtful defaults, a
minimal chrome, and everything that matters one keystroke away.

**[Download the latest release for Windows →](https://github.com/Maxibauser/Zenithium/releases/latest)**

## Features

- **Editor** — syntax highlighting, line numbers, in-gutter change bars,
  configurable font family / size / tab width, word-wrap and whitespace
  rendering, optional auto-save.
- **Explorer** — folder tree with quick open, wired to the tab bar.
- **Source Control** — first-class git panel: branch + upstream card,
  staged / unstaged file lists with per-status colored badges, inline commit
  message, one-click *Publish Branch to Remote…* when there's no upstream,
  *Initialize Repository* for plain folders, and an activity stream that
  shows each git command as a message with its output and exit status.
- **Terminal** — bottom panel with a persistent shell (PowerShell on Windows,
  bash on Linux). Follows the workspace's working directory.
- **Command Palette** — `Ctrl+Shift+P` fuzzy-searches every file / edit / view
  / settings / git action in the app.
- **Find** — `Ctrl+F` opens an in-editor find bar with match-case, whole-word,
  and wrap-around.
- **Settings** — dark / light themes, accent color, editor font family and
  size, tab width, word wrap, show whitespace, auto-save on focus loss,
  syntax highlighting, line numbers, change bars, and the modified indicator.
- **Status bar** — workspace and branch chips (branch shows ahead/behind).
- **Preferences persistence** — everything above is saved via `QSettings`.

## Keyboard shortcuts

| Action                    | Shortcut          |
| ------------------------- | ----------------- |
| Command Palette           | `Ctrl+Shift+P`    |
| Find                      | `Ctrl+F`          |
| New File                  | `Ctrl+N`          |
| Open File                 | `Ctrl+O`          |
| Open Folder               | `Ctrl+K` `Ctrl+O` |
| Save / Save As            | `Ctrl+S` / `Ctrl+Shift+S` |
| Close Tab                 | `Ctrl+W`          |
| Toggle Explorer           | `Ctrl+B`          |
| Toggle Source Control     | `Ctrl+Shift+G`    |
| Toggle Terminal           | `` Ctrl+` ``      |
| Preferences               | `Ctrl+,`          |

## Requirements

- **CMake** ≥ 3.24
- **Qt** ≥ 6.5 (Core, Gui, Widgets, Svg)
- A C++20 compiler: MSVC 2022, Clang 15+, or GCC 12+
- **Ninja** (recommended)
- **git** on `PATH` (used by the Source Control panel)

## Build

```bash
cmake --preset windows-msvc-debug
cmake --build --preset windows-msvc-debug
```

Presets: `windows-msvc-debug`, `windows-msvc-release`,
`linux-gcc-debug`, `linux-gcc-release`.

On Windows the build target automatically runs `windeployqt` to stage the Qt
runtime next to the executable.

## Layout

- `engine/` — headless, testable core
  - `core/` — versioning + shared helpers
  - `document/` — in-memory document + line-diff model
  - `syntax/` — language rule tables for highlighting
  - `text/` — text buffer primitives
  - `git/` — async `git` CLI wrapper used by the Source Control panel
- `ui/` — Qt 6 UI layer
  - `shell/` — main window, title bar, status bar, find bar, command palette
  - `editor/` — editor view, gutter, syntax highlighter
  - `panels/` — Explorer, Git, Terminal
  - `theming/` — dark / light QSS + theme engine
  - `dialogs/` — settings dialog
  - `welcome/` — welcome page
- `app/` — executable entry point wiring engine + ui
- `tests/` — unit tests
- `cmake/` — build helpers, sanitizer + compiler-flag modules
- `packaging/` — Windows / Linux installer configs
- `docs/` — architecture notes, ADRs

## Packaging (Windows)

Two artifacts, built from the same install tree:

```powershell
.\packaging\windows\build-installer.ps1
```

That script imports MSVC env, configures + builds a Release, stages a clean
install tree via `cmake --install` (windeployqt drops Qt DLLs + plugins next
to the exe), then runs Inno Setup to produce `dist-installer\Zenithium-Setup-<ver>.exe`.

The installer offers:

- **Start Menu** shortcut, optional Desktop shortcut, optional PATH entry
- **File associations** for C/C++, Python, web (JS/TS/JSON/HTML/CSS), and text/Markdown — each group is a separate opt-in task on the wizard
- **Shell context menu** — right-click a file or folder → *Open with Zenithium* (and the folder background — right-click empty space inside a folder to open it as a workspace)
- Proper **uninstaller** that removes registry entries and shortcuts

Prereqs: [Inno Setup 6 or 7](https://jrsoftware.org/isdl.php) installed
(`iscc.exe` on PATH or in the default Program Files location), plus the
normal Zenithium build toolchain.

For a **portable** ZIP without an installer:

```powershell
cmake --preset windows-msvc-release
cmake --build --preset windows-msvc-release
cpack --config build\windows-msvc-release\CPackConfig.cmake -G ZIP
```

## Status

Early but usable. The editor, Source Control panel, Terminal, Command
Palette, Find, and Settings all work end-to-end. This repo is under active
development — expect breaking changes to the internal APIs while things
settle.

## License

See [LICENSE](LICENSE).
