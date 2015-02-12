## Introduction
Grok is simple software that allows you to easily parse logs and other files. With grok, you can turn unstructured log and event data into structured data.

The grok program is a great tool for parsing log data and program output. You can match any number of complex patterns on any number of inputs (processes and files) and have custom reactions.

The grok library is a great choice when you need the pattern matching features of grok in your own tools. There are currently C and Ruby APIs.

* **GrokConcepts** - If you want general information that applies to both the grok program and library.
* **GrokProgram** - If you want documentation about the grok program.
  * **GrokExamples** - Examples of using grok in the real world
  * **GrokConfig** - Documentation on grok configs
  * **GrokPredicates** - Applying additional conditions (string, numerical compare, etc) to a matched group
  * **GrokProgramTutorial** - A gentle introduction to grok.
* **GrokLibrary** - If you want documentation about using the grok library api
* **GrokDiscovery** - Automatic pattern discovery
* Grok is also available for Ruby (gem install jls-grok)

## What can I use Grok for?
* reporting errors and other patterns from logs and processes
* parsing complex text output and converting it to json for external processing
* apply 'write-once use-everywhere' to regular expressions
* automatically providing patterns for unknown text inputs (logs you want patterns generated for future matching)

## Download Grok
* Latest: http://semicomplete.googlecode.com/files/grok-1.20110630.1.tar.gz
* Archive: http://code.google.com/p/semicomplete/downloads/list?q=label:grok

## Building
* Standard ```make``` and ```make install```
* If you want to build a debian package, you can do 'make package-debian'

## Getting Help
* mail: grok-users@googlegroups.com
