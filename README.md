# market-data-engine

A C++20 engine that reads BinaryFILE-framed
[Nasdaq TotalView-ITCH 5.0](https://www.nasdaqtrader.com/content/technicalsupport/specifications/dataproducts/NQTVITCHSpecification.pdf)
data and reconstructs displayed bid and ask price levels across symbols. The
inspection CLI processes files or standard input and reports message counts,
active orders, and why processing stopped.

## What it does

- Reads BinaryFILE frames and parses system events, stock directories, and
  displayed-order additions, executions, cancellations, deletions, and
  replacements (`S/R/A/F/E/C/X/D/U`).
- Tracks active order references and remaining shares, aggregates quantities by
  symbol, side, and price, and exposes best-bid and best-ask queries.
- Checks frame and payload sizes, message types, directory symbols, order
  references, stock locates, and excessive share reductions.

Other ITCH message types are counted under `other` without updating the book.
Reconstruction needs the preceding order history; arbitrary mid-session slices may
be rejected for unknown order references.

## Build

Requires CMake 3.20 or newer and a C++20 compiler:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

## Run

Inspect a decompressed BinaryFILE:

```sh
./build/market_data_inspect <decompressed-binaryfile>
```

Stream gzip-compressed input through standard input:

```sh
gzip -dc sample.NASDAQ_ITCH50.gz | ./build/market_data_inspect -
```

Stop after a bounded number of messages, including unsupported message types:

```sh
gzip -dc sample.NASDAQ_ITCH50.gz |
  ./build/market_data_inspect - --max-messages 1000000
```

The summary includes `end_reason`: `session_terminator`, `eof`, or `message_limit`.

## Validation

The engine processed all 268,744,780 complete frames available in Nasdaq's official
`12302019.NASDAQ_ITCH50.gz` sample without supported-message parsing, directory,
lifecycle, or aggregate-book consistency failures. That sample ends on a complete
frame boundary without the BinaryFILE specification's zero-length session
terminator. The engine reports clean EOF separately from formal session termination
and warns when the terminator is absent.

Unit and integration tests run through CTest. The standalone CLI tests require
Python 3 and use only its standard library. All tests use synthetic data; no
Nasdaq sample or benchmark fixture is required.

```sh
ctest --test-dir build --output-on-failure
python3 tests/market_data_inspect_test.py ./build/market_data_inspect
```

## Design

The BinaryFILE reader separates framing from payload decoding. Parsers produce
typed messages; `OrderTracker` holds individual orders, while `OrderBook` owns
directory validation and aggregates bid/ask levels. The CLI connects the reader
and parsers to the book.
