#!/usr/bin/env python

#import Levenshtein as dist
import difflib
import sys
import re

#ifile = "tmp/auth.log"
#ifile = "tmp/access.1187827200"
ifile = "messages"
input = file(ifile)

differ = difflib.Differ()
known_patterns = []

escapees = re.compile(r"([.\[\]\{\}\(\)])")
def mkreg(pattern):
  re_list = []
  for i in pattern:
    if i == "XXTOKXX":
      re_list.append("\S+")
    else:
      re_list.append(re.escape(i))
  reg = re.compile("\s+".join(re_list))
  #print reg.pattern
  return reg

def pattok(word):
  try:
    i = int(word)
    return "XX:INT"
  except:
    return "XX:TOK"
    

#print mkreg(["foo", "bar.baz", "1.2.3.4", "[foo]"])
#sys.exit(1)

lineno = 0
for line in input:
  line = line.strip()
  compare_input = file(ifile)
  words = line.split()
  lineno += 1
  print >>sys.stderr, "Line %d" % lineno

  match = None
  for reg in known_patterns:
    match = reg.match(line)
    if match:
      break
  if match:
    #print "Skipping known line %d" % lineno
    continue

  for compare_line in compare_input:
    compare_line = compare_line.strip()
    compare_words = compare_line.split()

    if compare_line == line:
      continue

    match = None
    for reg in known_patterns:
      match = reg.match(compare_line)
      if match:
        break
    if match:
      continue

    result = differ.compare(words, compare_words);
    #result = difflib.unified_diff(words, compare_words);
    similarity = 0
    pattern = []
    op = ""
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

    if similarity > 5:
      reg = mkreg(pattern)
      print " ".join(pattern)
      known_patterns.append(reg);
