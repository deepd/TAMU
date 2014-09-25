__author__ = 'deep'

import search
import eqparser
import helperfns

s=raw_input("eq>")
v=raw_input("var>")
p = eqparser.parse(s)
pbm = search.equationSolver(initial=p,variable=v)

print pbm.astar_search(pbm)

