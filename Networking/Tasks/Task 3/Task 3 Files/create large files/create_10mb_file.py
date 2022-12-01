#! /usr/bin/env/ python3

with open("10mb.txt", "wb") as out:
    out.seek(( 1024 * 1024 * 10) - 1)
    out.write('\0')
