# market-data-engine

A C++20 reader for sequential BinaryFILE streams with System Event (`S`) and
Add Order (`A`) parsing for
[Nasdaq TotalView-ITCH 5.0](https://www.nasdaqtrader.com/content/technicalsupport/specifications/dataproducts/NQTVITCHSpecification.pdf).

Build and test with CMake 3.20 or newer and a C++20 compiler:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build --output-on-failure
```

Inspect message counts in a decompressed BinaryFILE:

```sh
./build/market_data_inspect <decompressed-binaryfile>
```
