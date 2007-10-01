#!/usr/bin/env python

#import Levenshtein as dist
import difflib
import sys
import re

import wordlist

#ifile = "tmp/auth.log"
#ifile = "tmp/access.1187827200"
ifile = sys.argv[1]
input = file(ifile)

differ = difflib.Differ()
known_patterns = []

def mkpat(word):
  try:
    x = int(word)
    return "%INT%"
  except Exception, e:
    pass
  return "%WORD%"

def mkreg(we, diffs):
  i = 0
  offset = 0
  string = we._string
  for i in diffs:
    we[i] = mkpat(we[i].word)
  return str(we)

#we = wordlist.WordExtractor("foo bar 12 hello 34 blah")
#print mkreg(we, [0,2,3])
#sys.exit(1)
  
def is_known_pattern(string):
  for reg in known_patterns:
    match = reg.match(line)
    if match:
      return True
  return False

def find_differences(a, b):
  diff_indeces = []
  if len(a) != len(b):
    return []

  i = 0
  while i < len(a):
    if (a[i] != b[i]):
      diff_indeces.append(i)
    i += 1

  return diff_indeces

lineno = 0
for line in input:
  line = line.strip()
  we = wordlist.WordExtractor(line)
  words = list(we.WordListIter())
  compare_input = file(ifile)
  lineno += 1

  if is_known_pattern(line):
    #print "Skipping known line %d" % lineno
    continue

  this_line_similarity = 0
  for compare_line in compare_input:
    compare_line = compare_line.strip()

    if compare_line == line:
      continue
    if is_known_pattern(compare_line):
      continue

    compare_we = wordlist.WordExtractor(compare_line)
    compare_words = list(compare_we.WordListIter())

    diffs = find_differences(words, compare_words)
    if len(diffs) == 0:
      continue

    similarity = len(words) - len(diffs)
    score = (similarity + 0.0) / min(len(words), len(compare_words))
    #print score

    if score > .5:
      this_line_similarity += 1
      reg = mkreg(we, diffs)
      #known_patterns.append(reg);
      print "%.2f: %s" % (score, line)
      print "    : %s" % (compare_line)
      print "  ==> %s" % reg
      #print " ".join(pattern)

    if this_line_similarity > 30:
      break
