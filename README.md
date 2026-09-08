# market-data-engine

A C++20 program that reads sequential BinaryFILE streams and reconstructs displayed
bid and ask price levels for multiple stocks from
[Nasdaq TotalView-ITCH 5.0](https://www.nasdaqtrader.com/content/technicalsupport/specifications/dataproducts/NQTVITCHSpecification.pdf)
messages.

Build and test with CMake 3.20 or newer and a C++20 compiler:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build --output-on-failure
```

Inspect message counts and the final active order count in a decompressed BinaryFILE:

```sh
./build/market_data_inspect <decompressed-binaryfile>
```

Use `-` to read standard input and `--max-messages N` to stop after N payloads:

```sh
gzip -dc sample.NASDAQ_ITCH50.gz | ./build/market_data_inspect - --max-messages 1000000
```
