#!/usr/local/bin/python

import re

class WordExtractor(object):
  memoize = {}
  def __init__(self, string):
    self._string = string
    self._list = None
    self.memoize[string] = self

  def WordIter(self):
    if self._list is None:
      self.Process()
    return iter(self._list)

  def WordListIter(self):
    for i in self:
      yield i.word

  def __iter__(self):
    return self.WordIter()

  def __getitem__(self, i):
    if self._list is None:
      self.Process()
    return self._list[i]

  def __setitem__(self, i, val):
    if self._list is None:
      self.Process()
    self._list[i].new_word = val
    self._list[i].tainted = True

  def __str__(self):
    string = self._string
    offset = 0
    for i in self._list:
      if not i.tainted:
        continue
      string = string[:i.start + offset] + i.new_word + string[i.end + offset:]
      new_offset = -len(i.word) + len(i.new_word)
      offset += new_offset
    return string

  def Process(self):
    word_re = re.compile(r'\w+|%\w+%')
    self._list = []
    for m in word_re.finditer(self._string):
      (start, end) = m.span()
      #print "%d,%d: %s" % (start, end, m.group())
      self._list.append(Word(m.group(), start, end))

class Word(object):
  def __init__(self, word, startpos, endpos):
    self.word = word
    self.start = startpos
    self.end = endpos
    self.tainted = False
    self.new_word = None

if __name__ == "__main__":
  import sys
  if len(sys.argv) != 2:
    print "no file specified"
    sys.exit(1)

  fd = file(sys.argv[1])
  for i in fd:
    we = WordExtractor(i)
    print " ".join(we.WordListIter())
