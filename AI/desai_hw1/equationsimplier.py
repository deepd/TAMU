#!/usr/bin/python
import eqparser 

while 1:
    try:
        s = raw_input('eq > ')   # use input() on Python 3
    except EOFError:
        print
        break
    p = eqparser.parse(s)
    print "This is parsed at: " + repr(p)
    print "In infix form: " + str(p)


