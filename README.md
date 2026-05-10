# Store Inventory Management System

A desktop application for managing a shop's product inventory, written in C (C17) with a GTK4 graphical interface.

## Features

- **Add / Edit / Delete** products with full field validation
- **Sort** the product list by code, name or price — ascending or descending (QuickSort)
- **Filter** products whose stock quantity falls below a given threshold
- **Search** by product name, model or price range
- **Save / Load** the inventory to a typed binary file (`.inv`)
- **Export** to Microsoft Excel (`.xlsx`) via libxlsxwriter
- Scrollable table view with six columns

## Product record

| Field | Type | Description |
|-------|------|-------------|
| Group | text | Product category |
| Code | integer | Unique product identifier |
| Name | text | Product name |
| Model | text | Model designation |
| Price | decimal | Unit price |
| Quantity | integer | Stock count |

## Requirements

### Runtime
- Windows 10 / 11 (x64)
- No additional installation required — download the installer below

### Building from source
- [MSYS2](https://www.msys2.org/) with MinGW-w64 toolchain
- CMake ≥ 3.20 (bundled with CLion or standalone)
- Ninja build tool
- GTK4, libxlsxwriter — installed via MSYS2 pacman

```bash
pacman -S mingw-w64-x86_64-gtk4 mingw-w64-x86_64-libxlsxwriter mingw-w64-x86_64-ninja
```

## Building

```bash
git clone https://github.com/mbchl-code/StoreWarehouse.git
cd StoreWarehouse
```

Configure CLion toolchain to use `C:\msys64\mingw64` (MinGW-w64), then open the project — CMake is picked up automatically.

Or build from the terminal:

```bash
cmake -S . -B build -G Ninja -DCMAKE_C_COMPILER=C:/msys64/mingw64/bin/gcc.exe
cmake --build build --target inventory
```

## Running tests

```bash
cmake --build build --target test_runner
./build/test_runner.exe
```

8 unit tests covering list operations and QuickSort correctness.

## Creating the installer

1. Install [NSIS](https://nsis.sourceforge.io/Download)
2. Run in PowerShell:

```powershell
powershell -ExecutionPolicy Bypass -File make_installer.ps1
```

This builds a Release binary, collects all GTK4 runtime DLLs and data files, and produces `Inventory-1.0.0-win64.exe` in `cmake-build-release/`.

## Project structure

```
src/
  list.c/h        — doubly-linked list (CRUD operations)
  sort.c/h        — QuickSort by code / name / price
  filter.c/h      — quantity filter and text/price search logic
  fileio.c/h      — binary file save / load
  export.c/h      — Excel export via libxlsxwriter
  ui/             — GTK4 interface (no business logic)
    main_window   — toolbar, GtkColumnView product table
    product_dialog — add / edit dialog with validation
    filter_window  — filter and search window
    app           — AppState, GListStore management
tests/
  test_list.c     — list unit tests
  test_sort.c     — sort unit tests (including descending)
cmake/
  copy_dlls.py    — copies MinGW DLLs to build directory post-build
  collect_dist.py — collects full distribution for packaging
resources/
  icon.ico        — application icon (embedded in exe via .rc file)
```

## Implementation notes

- Written in **C17** following structural programming principles — no `break`, `continue` or `goto` anywhere in the source
- Business logic (`list`, `sort`, `filter`, `fileio`, `export`) has **no GTK dependency** and is tested headlessly
- GTK4 on Windows uses the **Cairo renderer** and **Win32 backend** to avoid a D3D12 initialisation issue present in GTK4 ≥ 4.14 on MinGW builds
- Binary file format: fixed-size `Product` records written with `fwrite` / read with `fread`

## License

MIT
