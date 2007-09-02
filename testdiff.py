#!/usr/bin/env python

import difflib

s1 = "hello foo[2345] there".split()
s2 = "hello foo[2352] there".split()

d = difflib.Differ()
for i in d.compare(s1, s2):
  print i
