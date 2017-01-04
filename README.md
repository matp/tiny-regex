tiny-regex
==========

tiny-regex is a small implementation of a regular expression matching engine,
written in standard C11. It is based on two papers by Russ Cox <rsc@swtch.com>:

* ["Regular Expression Matching Can Be Simple And Fast"](https://swtch.com/~rsc/regexp/regexp1.html)
* ["Regular Expression Matching: the Virtual Machine Approach"](https://swtch.com/~rsc/regexp/regexp2.html)

This library was mainly written for fun and is not intended as a full-featured
general purpose regular expression matching engine.

Engine features
---------------

* `^` and `$` anchors
* `.` match any single character
* `[...]` and `[^...]` character classes
* `?`, `*`, `+`, and `{x,y}` greedy quantifiers
* `??`, `*?`, `+?`, and `{x,y}?` non-greedy quantifiers
* `(...)` capturing groups
