# QtTest unit tests

This archive adds two `utest/` folders using the QtTest framework, plus the
two-line CMakeLists.txt changes needed to wire them into your existing build.

## What's in here

```
Backend/CMakeLists.txt      <- your existing file, with a BUILD_TESTING block appended
Backend/utest/
    CMakeLists.txt
    tst_csvreader.{h,cpp}         - CSVReader parsing/writing
    tst_kmeansprocessor.{h,cpp}   - KMeansProcessor::process_email_csv
    tst_gruprocessor.{h,cpp}      - GRUProcessor::process_email_csv_gru
    tst_pipeline.{h,cpp}          - Pipeline (file-based entry points)

Frontend/CMakeLists.txt     <- your existing file, with a BUILD_TESTING block appended
Frontend/utest/
    CMakeLists.txt
    tst_filemanager.{h,cpp}        - FileManager (read/write/exists)
    tst_datavisualization.{h,cpp}  - DataVisualization (chart construction)
```

## How to merge

Copy `Backend/utest/` and `Frontend/utest/` into your project as-is, then
replace `Backend/CMakeLists.txt` and `Frontend/CMakeLists.txt` with the ones
here (or just paste in the `BUILD_TESTING` block at the end of your own
copies — that's the only change made to each).

## Design notes

- **Backend tests** only need Qt's `Test` module (`Qt6::Test`) — the code
  under test is plain C++/Eigen, so these stay independent of the
  Frontend's Widgets/Charts dependency and build fast on their own.
- **Frontend tests** link the `filemanager.cpp` / `datavisualization.cpp`
  sources directly (not through `main.cpp`, which owns its own
  `int main`/`QApplication::exec()`), and run with
  `QT_QPA_PLATFORM=offscreen` so `QWidget`/`QChart` code can be exercised
  without a real display (works fine in CI/headless environments).
- `MainWindow` isn't tested here — it's a thin UI-glue class wired directly
  to `QFileDialog`/drag-and-drop/signal-slots, and would need some
  refactoring (e.g. injecting `FileManager`/`Pipeline` so they can be
  mocked) to test meaningfully rather than just smoke-testing widget
  construction. Happy to add that if useful.
- `FileManager::open_file_dialog()` isn't tested either, since it blocks on
  a native interactive dialog.
- `KMeansProcessor`'s clustering uses a random initialization
  (`std::random_device`), so its tests assert structural invariants
  (counts sum to total, one PCA point per message, etc.) rather than exact
  cluster assignments. `GRUProcessor`'s spam heuristic is deterministic, so
  its test does assert a specific ham/spam split.

## Running

```sh
# from a fresh build dir, or your existing one:
cmake -S . -B build
cmake --build build
cd build && ctest --output-on-failure
```

Each test file is its own executable (e.g. `tst_csvreader`), registered
with CTest, so you can also run just one:

```sh
ctest -R tst_kmeansprocessor --output-on-failure
```

I wasn't able to compile these in this sandbox (no Qt6/Eigen3 available
here), so give `ctest` a run and let me know if anything doesn't compile —
happy to fix it up.
