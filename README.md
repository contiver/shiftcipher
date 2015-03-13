shiftcipher
===========
Shift cipher encryptor and decryptor, mainly for learning purposes.
It currently supports only the English language (expects letters [A-Za-z]), and
has automatic decryption (based on letter frequency analysis) only for English.

Build
-----
By default gcc is used to compile and dynamically link the object file.
Ideally, statically link with the musl library by changing the options on the
config.mk file.

To build, just enter `make`
