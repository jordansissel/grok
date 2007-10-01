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

escapees = re.compile(r"([.\[\]\{\}\(\)])")
def mkreg(pattern):
  re_list = []
  for i in pattern:
    if i == "%INT%":
      re_list.append("\d+")
    elif i == "%WORD%":
      re_list.append("\S+")
    else:
      re_list.append(re.escape(i))
  reg = re.compile("\s+".join(re_list))
  #print reg.pattern
  return reg

def pattok(word):
  try:
    i = int(word)
    return "%INT%"
  except:
    return "%WORD%"
    

#print mkreg(["foo", "bar.baz", "1.2.3.4", "[foo]"])
#sys.exit(1)

lineno = 0
for line in input:
  line = line.strip()
  we = wordlist.WordExtractor(line)
  words = list(we.WordListIter())
  compare_input = file(ifile)
  lineno += 1

  match = None

  for reg in known_patterns:
    match = reg.match(line)
    if match:
      break
  if match:
    #print "Skipping known line %d" % lineno
    continue

  this_line_similarity = 0
  for compare_line in compare_input:
    compare_line = compare_line.strip()

    if compare_line == line:
      #print "Skipping identical line"
      continue

    compare_we = wordlist.WordExtractor(compare_line)
    compare_words = list(compare_we.WordListIter())

    match = None
    for reg in known_patterns:
      match = reg.match(compare_line)
      if match:
        break
    if match:
      continue

    result = differ.compare(words, compare_words);
    similarity = 0
    pattern = []
    lastop = op = ""
    for i in result:
      op = i[0]
      val = i[2:]
      if op == "?":
        continue
      elif op == " ":
        similarity += 1
        pattern.append(val)
        continue
      elif op == "+":
        if lastop == "-":
          continue
        pattern.append(pattok(val))
      else:
        pattern.append(pattok(val))

      lastop = op

    score = (similarity + 0.0) / min(len(words), len(compare_words))

    if score > .5:
      this_line_similarity += 1
      reg = mkreg(pattern)
      known_patterns.append(reg);
      #print "%.2f: %s" % (score, line)
      #print "    : %s" % (compare_line)
      #print " ==> %s" % " ".join(pattern)
      print " ".join(pattern)

    if this_line_similarity > 30:
      break
