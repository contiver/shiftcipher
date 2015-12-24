Shiftcipher
===========
Simple shift cipher encryptor and decryptor for the English language, mainly
for learning purposes.

Build
-----
To build, just enter `make`

By default gcc is used to compile and dynamically link the object file.
Ideally, statically link with the musl library by changing the options on the
config.mk file.

Usage
-----
The program reads from `stdin`. Example using [ROT13](https://en.wikipedia.org/wiki/ROT13):

```
echo "AVERAGEENGLISHTEXTTOENCRYPT" | ./shiftcipher -e 13
```

This gives the encrypted output: NIRENTRRATYVFUGRKGGBRAPELCG

To decrypt it knowing the key:

```
echo "NIRENTRRATYVFUGRKGGBRAPELCG" | ./shiftcipher -d 13
```

Alternatively, if you don't know the key and want to try guessing it through
frequency analysis, don't mention any after the `-d` option.

```
echo "NIRENTRRATYVFUGRKGGBRAPELCG" | ./shiftcipher -d
```

Finally, there is also the less elegant choice of plain old bruteforce:

```
echo "NIRENTRRATYVFUGRKGGBRAPELCG" | ./shiftcipher -b
```

Additional notes
----------------
The global variable `plaintext` helps simplify the code, maintaining function
prototypes as you would find them on an average cryptography textbook 
(e.g.`dec(m,k)`). If for some reason concurrency is needed, this should be changed.
