# UAXlib

An implementation of the Unicode bidirectional algorithm for a (since-abandoned) project. Written a few years ago, released here for the six other people on the planet who might find it useful.

Was intended to bundle implementations of many of the core Unicode algorithms (and still may, if I or others have time).

## Usage

Clone, `cd` into `UAXlib/Unicode/6.3.0/UAX`, `make`, then `make test` to run the bidi test suite. It passes everything except those tests involving reordering (which is technically dependent on the line-breaking algorithm, which isn't implemented, but a naive implementation that assumes no-breaking might be fine for the tests).

Building will convert UCD data into something code-friendly in `_Derived`.

The bidi function itself is `UAX::Bidi::Run(...)`. See `UAX.h` for more info.

Could probably use an update to Unicode 8.0...