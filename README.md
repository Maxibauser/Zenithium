# Zenithium

A modern, cross-platform code IDE built with C++20 and Qt6.

## Status

Early scaffold. The app builds and opens an empty main window with the Zenithium dark theme.

## Requirements

- **CMake** ≥ 3.24
- **Qt** ≥ 6.5 (Widgets)
- A C++20 compiler: MSVC 2022, Clang 15+, or GCC 12+
- **Ninja** (recommended)

## Build

```bash
cmake --preset windows-msvc-debug
cmake --build --preset windows-msvc-debug
```

Presets: `windows-msvc-debug`, `windows-msvc-release`, `linux-gcc-debug`, `linux-gcc-release`.

## Layout

- `engine/` — headless, testable core (text buffer, LSP, VCS, indexer, …)
- `ui/` — Qt6 UI layer (shell, editor view, panels, theming)
- `app/` — thin executable entry point wiring engine + ui
- `tests/` — unit, integration, ui, fuzz, benchmarks
- `cmake/` — build helpers, toolchains
- `packaging/` — Windows / Linux installers
- `docs/` — architecture, ADRs, user guide

See `docs/architecture/` for module responsibilities.

## License

See [LICENSE](LICENSE).
