__author__ = 'deep'

import eqparser
from eqparser import Node as eqNode
import operator
import search
from helperfns import isX
from helperfns import isSimplifiedSubTree

ops = { "+": operator.add, "-": operator.sub, "*": operator.mul, "/":operator.div, "^":operator.pow, "=":operator.eq }
specials = ["sin", "cos", "tan", "log", "ln", "sqrt"]
arithtypes = ["INT", "FLOAT"]
counter = 0


s=raw_input("eq>")
r=raw_input("eq2>")
p = eqparser.parse(s)
p2=eqparser.parse(r)
q = p.children[1]
currentstate = p

# Function to do arithmetic operations for more than one levels

def solveThis(x):
    c1 = x.children[0]
    c2 = x.children[1]
    if c1.children != []:
        c1 = solveThis(c1)
        if c1.children[0].leaf in specials or c1.children[0].type in ["VARIABLENAME", "SYMBOL"]:
            return c1.children[0]
        if c1.children[1].leaf in specials or c1.children[1].type in ["VARIABLENAME", "SYMBOL"]:
            return c1.children[1]
    if c2.children != []:
        c2 = solveThis(c2)
        if c2.children[0].leaf in specials or c2.children[0].type in ["VARIABLENAME", "SYMBOL"]:
            return c2.children[0]
        if c2.children[1].leaf in specials or c2.children[1].type in ["VARIABLENAME", "SYMBOL"]:
            return c2.children[1]
    opr = x.leaf
    res = ops[opr](c1.leaf,c2.leaf)
    return eqNode(c1.type,children=None,leaf=res)

# Function to apply inverse

#def inverseThis(x):

#while (temp != None):
 #   temp =

# Function to get the pointer to Node in state x with c1 and c2 children and l is the leaf

def findPointer(x,c1,l,c2):
    if x.children != []:
        if x.leaf in ops.keys() and x.leaf == l and x.children[0].leaf == c1 and x.children[1].leaf == c2:
            return x
        else:
            if x.leaf in ops.keys():
                y1 = findPointer(x.children[0], c1, l, c2)
                y2 = findPointer(x.children[1], c1, l, c2)
                if y1:
                    return y1
                elif y2:
                    return y2
    return

# Function to simplify one arithmetic

def simplify(temp,c1,l,c2):
    try:
        v1 = int(c1) and int (c2)
        x = findPointer(temp,c1,l,c2)
        if x == None:
            return temp
        c1 = x.children[0]
        c2 = x.children[1]
        if c1.leaf in specials or c2.leaf in specials:
            return temp
        x.leaf=ops[x.leaf](c1.leaf,c2.leaf)
        x.children=[]
        x.type=c1.type
        return temp
    except ValueError:
        return temp

# Function to do apply an inverse identity on given tree with child1 leaf and child2

def inverseIdentity(temp, c1, l, c2):
    temp2 = temp
    x = findPointer(temp, c1, l, c2)
    print repr(x)
    rleaf = lleaf = temp
    if x.leaf == "*":
        x.leaf = "/"
    elif x.leaf == "/":
        x.leaf = "*"
    elif x.leaf == "+":
        x.leaf = "-"
    elif x.leaf == "-":
        x.leaf = "+"
    while (temp.children != []):
        if x in temp.children:
            if x == temp.children[0]:
                temp.children[0] = x.children[0]
                x.children[0] = temp2.children[1]
                temp2.children[1] = x
                return temp2
        temp = temp.children[0]
    temp = temp2
    while (temp.children != []):
        if x in temp.children:
            if x == temp.children[1]:
                temp.children[1] = x.children[0]
                x.children[0] = temp2.children[0]
                temp2.children[0] = x
                return temp2
        temp = temp.children[1]
    return temp2

# Function for associative property

def commutative(temp, c1, l, c2):
    x = findPointer(temp, c1, l, c2)
    temp1 = x.children[0]
    temp2 = x.children[1]
    x.children[0] = temp2
    x.children[1] = temp1
    return temp

# Function to make a copy of whole tree

def makeCopy(x):
    copyNode = eqNode("EQUALS",children=None,leaf="=")
    if x.leaf in specials:
        copyNode = eqNode(x.type, children=[x.children],leaf=x.leaf)
    elif x.leaf in ops.keys():
        copyNode = eqNode(x.type, children=[makeCopy(x.children[0]),makeCopy(x.children[1])],leaf=x.leaf)
    else:
        copyNode = eqNode(x.type, children=None, leaf=x.leaf)
    return copyNode

def actions(x):
        global currentstate
        global counter
        acts = {}
        if x.children != [] and x.leaf not in specials:
            if  x.leaf == '=' and x.children[0].leaf == "sqrt" and x.children[0].children.leaf == self.variable :
                acts[counter] = [squarIt,makeCopy(currentstate),self.variable]
                counter+=1
                acts = dict(acts.items() + actions(x.children[1]).items())
                return acts
            elif x.leaf == '=' and x.children[0].leaf == "log" and x.children[0].children.leaf == self.variable:
                acts[counter] = [unLogIt,makeCopy(currentstate),self.variable]
                counter+=1
                acts = dict(acts.items() + actions(x.children[1]).items())
                return acts
            elif x.leaf == '=' and x.children[0].leaf == "ln" and x.children[0].children.leaf == self.variable:
                acts[counter] = [unLnIt(),makeCopy(currentstate),self.variable]
                counter+=1
                acts = dict(acts.items() + actions(x.children[1]).items())
                return acts
            elif x.children[0].children != [] and x.children[1].children == [] and (x.leaf == "+" or x.leaf == "-"):
                acts[counter] = [inverseIdentity,makeCopy(currentstate),x.children[0].leaf,x.leaf,x.children[1].leaf]
                counter+=1
                acts = dict(acts.items() + actions(x.children[0]).items() + actions(x.children[1]).items())
                return acts
            elif x.children[0].children == [] and x.children[1].children != [] and (x.leaf == "+" or x.leaf == "-"):
                acts[counter] = [inverseIdentity,makeCopy(currentstate),x.children[0].leaf,x.leaf,x.children[1].leaf]
                counter+=1
                if x.leaf == '+':
                    acts[counter] = [commutative,makeCopy(currentstate),x.children[0].leaf,x.leaf,x.children[1].leaf]
                    counter+=1
                acts = dict(acts.items() + actions(x.children[0]).items() + actions(x.children[1]).items())
                return acts
            elif x.leaf == "+" or x.leaf == "*":
                acts[counter] = [simplify,makeCopy(currentstate),x.children[0].leaf,x.leaf,x.children[1].leaf]
                counter+=1
                if (x.children[0].type in arithtypes+specials) and (x.children[1].type in arithtypes+specials or x.children[1].type in ["VARIABLENAME", "SYMBOL"]):
                    acts[counter] = [commutative,makeCopy(currentstate),x.children[0].leaf,x.leaf,x.children[1].leaf]
                    counter+=1
                elif (x.children[0].type in ["VARIABLENAME", "SYMBOL"]) and x.children[1].type in arithtypes+specials:
                    acts[counter] = [commutative,makeCopy(currentstate),x.children[0].leaf,x.leaf,x.children[1].leaf]
                    counter+=1
                    acts[counter] = [inverseIdentity,makeCopy(currentstate),x.children[0].leaf,x.leaf,x.children[1].leaf]
                    counter+=1
                elif x.children[0].type in ["VARIABLENAME", "SYMBOL"] and x.children[0].leaf == self.variable:
                    acts[counter] = [inverseIdentity,makeCopy(currentstate),x.children[0].leaf,x.leaf,x.children[1].leaf]
                    counter+=1
                elif x.children[0].type in ["VARIABLENAME", "SYMBOL"] and x.children[1].leaf.type in ["VARIABLENAME", "SYMBOL"]:
                    acts[counter] = [commutative,makeCopy(currentstate),x.children[0].leaf,x.leaf,x.children[1].leaf]
                    counter+=1
                acts = dict(acts.items() + actions(x.children[0]).items() + actions(x.children[1]).items())
                return acts
            elif x.leaf == "-" or x.leaf == "/" or x.leaf == "^":
                acts[counter] = [simplify,makeCopy(currentstate),x.children[0].leaf,x.leaf,x.children[1].leaf]
                counter+=1
                if x.children[0].type in ["VARIABLENAME", "SYMBOL"] and x.children[0].leaf == self.variable:
                    acts[counter] = [inverseIdentity,makeCopy(currentstate),x.children[0].leaf,x.leaf,x.children[1].leaf]
                    counter+=1
                elif x.children[0].type in ["VARIABLENAME", "SYMBOL"] and x.children[1].leaf.type in ["VARIABLENAME", "SYMBOL"]:
                    acts[counter] = [inverseIdentity,makeCopy(currentstate),x.children[0].leaf,x.leaf,x.children[1].leaf]
                    counter+=1
                acts = dict(acts.items() + actions(x.children[0]).items() + actions(x.children[1]).items())
                return acts
            else:
                acts[counter] = [simplify,makeCopy(currentstate),x.children[0].leaf,x.leaf,x.children[1].leaf]
                counter+=1
                acts = dict(acts.items() + actions(x.children[0]).items() + actions(x.children[1]).items())
                return acts
        else:
            return {}


def result(x, act):
    return act[0](act[1],act[2],act[3],act[4])

def goal_test(state):
        """Return True if the state is a goal. The default method compares the
        state to self.goal, as specified in the constructor. Override this
        method if checking against a single self.goal is not enough."""
        if state.children[0].leaf != 'x' and not (state.children[0].leaf == "sin" or state.children[0].leaf == "cos" or\
                                                    state.children[0].leaf == "tan"):
            return False
        else:
            return isSimplified(state)

def isSimplified(x, v):
    if ifIdentityLeft(x):
        return False
    if x.children != [] and x.leaf in ops.keys()  and x.leaf != '='  and (x.children[0].leaf == v or x.children[1].leaf == v):
        return False
    elif x.children != [] and x.leaf in ops.keys():
        if x.children[0].leaf not in ops.keys() and x.children[1].leaf not in ops.keys():
            if (x.children[0].leaf in specials and x.children[1].leaf in specials) or\
                    (x.children[0].leaf in specials and x.children[1].type in arithtypes) or\
                    (x.children[0].leaf in specials and x.children[1].type in ["VARIABLENAME", "SYMBOL"]) or\
                    (x.children[0].type in ["VARIABLENAME", "SYMBOL"] and x.children[1].leaf in specials) or\
                    (x.children[0].type in ["VARIABLENAME", "SYMBOL"] and x.children[1].type in arithtypes)or\
                    (x.children[0].type in ["VARIABLENAME", "SYMBOL"] and x.children[1].type in ["VARIABLENAME", "SYMBOL"]) or\
                    (x.children[0].type in arithtypes and x.children[1].leaf in specials) or\
                    (x.children[0].type in arithtypes and x.children[1].type in ["VARIABLENAME", "SYMBOL"]):
                left = True
                right = True
            elif (x.children[0].type in arithtypes and x.children[1].type in arithtypes):
                left = False
                right = False
            elif x.children[0].leaf in specials and x.children[1].leaf not in specials:
                left = True
                right = isSimplified(x.children[1], v)
            elif x.children[0].leaf not in specials and x.children[1].leaf in specials:
                left = isSimplified(x.children[0], v)
                right = True
            elif x.children[0].leaf not in specials and x.children[1].leaf not in specials:
                left = isSimplified(x.children[0], v)
                right = isSimplified(x.children[1], v)
        elif x.children[0].leaf not in ops.keys() and x.children[1].leaf in ops.keys():
            left = True
            right = isSimplified(x.children[1], v)
        elif x.children[0].leaf in ops.keys() and x.children[1].leaf not in ops.keys():
            left = isSimplified(x.children[0], v)
            right = True
        elif x.children[0].leaf in ops.keys() and x.children[1].leaf in ops.keys():
            left = isSimplified(x.children[0], v)
            right = isSimplified(x.children[1], v)
        return (left and right)
    else:
        return True

def h(x):
    lops = 0
    rops = 0
    temp = x.children[0]
    while(temp.leaf in ops.keys()):
        if temp.children[0] in specials or temp.children[0] in ops.keys() or temp.children[1] in specials or temp.children[1] in ops.keys() \
                or temp.children[0].type in ["VARIABLENAME", "SYMBOL"] or temp.children[1].type in ["VARIABLENAME", "SYMBOL"]:
            lops+=1
        temp = temp.children[0]
    temp = x.children[1]
    while(temp.leaf in ops.keys()):
        if temp.children[0] in specials or temp.children[0] in ops.keys() or temp.children[1] in specials or temp.children[1] in ops.keys() \
                or temp.children[0].type in ["VARIABLENAME", "SYMBOL"] or temp.children[1].type in ["VARIABLENAME", "SYMBOL"]:
            rops+=1
        temp = temp.children[1]
    return (2*lops+rops)

def newActions(x):
    global counter
    global currentState
    acts = {}
    if x.children != []:
        if x.children[0].children != [] and x.children[1].children != []:
            if x.leaf == '+':
                acts[counter] = [commutative,makeCopy(currentState),x.children[0].leaf,x.leaf,x.children[1].leaf]
                counter+=1
            if x.leaf == "*" and x.children[0].type in arithtypes and x.children[1].type in arithtypes:
                acts[counter] = [commutative,makeCopy(currentState),x.children[0].leaf,x.leaf,x.children[1].leaf]
                counter+=1
            acts = dict(acts.items() + newActions(x.children[0]).items() + newActions(x.children[1]).items())
            return acts
        elif x.children[0].children != [] and x.children[1].children == [] and (x.leaf == "+" or x.leaf == "-"):
            acts[counter] = [inverseIdentity,makeCopy(currentState),x.children[0].leaf,x.leaf,x.children[1].leaf]
            counter+=1
            acts = dict(acts.items() + newActions(x.children[0]).items() + newActions(x.children[1]).items())
            return acts
        elif x.children[0].children == [] and x.children[1].children != [] and (x.leaf == "+" or x.leaf == "-"):
            acts[counter] = [inverseIdentity,makeCopy(currentState),x.children[0].leaf,x.leaf,x.children[1].leaf]
            counter+=1
            acts = dict(acts.items() + newActions(x.children[0]).items() + newActions(x.children[1]).items())
            return acts
        elif x.leaf == "+" or x.leaf == "*":
            acts[counter] = [simplify,makeCopy(currentState),x.children[0].leaf,x.leaf,x.children[1].leaf]
            counter+=1
            acts[counter] = [commutative,makeCopy(currentState),x.children[0].leaf,x.leaf,x.children[1].leaf]
            counter+=1
            acts = dict(acts.items() + newActions(x.children[0]).items() + newActions(x.children[1]).items())
            return acts
        else:
            acts[counter] = [simplify,makeCopy(currentState),x.children[0].leaf,x.leaf,x.children[1].leaf]
            counter+=1
            acts = dict(acts.items() + newActions(x.children[0]).items() + newActions(x.children[1]).items())
            return acts
    else:
        return {}

def squarIt(x):
    temp = x
    temp2 = eqNode("BINARYOP",children=[temp.children[1],eqNode("INT",children=None,leaf=2)],leaf='^')
    temp.children[0] = temp.children[0].children
    temp.children[1] = temp2
    return temp

def unLogIt(x):
    temp = x
    temp2 = eqNode("BINARYOP",children=[eqNode("INT",children=None,leaf=10),temp.children[1]],leaf='^')
    temp.children[0] = temp.children[0].children
    temp.children[1] = temp2
    return temp

def unLnIt(x):
    temp = x
    temp2 = eqNode("BINARYOP",children=[eqNode("INT",children=None,leaf='e'),temp.children[1]],leaf='^')
    temp.children[0] = temp.children[0].children
    temp.children[1] = temp2
    return temp

def findOperations(x):
    if x.children != [] and x.leaf in ops.keys():
        if x.leaf == '=':
            return findOperations(x.children[0]) + findOperations(x.children[1])
        else:
            return 1 + findOperations(x.children[0]) + findOperations(x.children[1])
    else:
        return 0

def findDepthOfX(x, v):
    if x.leaf == v:
        return 0
    elif x.children != [] and x.leaf not in specials:
        return max(1 + findDepthOfX(x.children[0], v), 1 + findDepthOfX(x.children[1], v))
    elif x.leaf in specials and x.children.leaf == v:
        return 2
    else:
        return 0

def ifXInLeft(x,v):
    if (x.children == [] and x.leaf != v):
        return 10
    elif x.leaf in specials and x.children.leaf == v:
        return 2
    elif x.leaf == v:
        return 1
    else:
        return ifXInLeft(x.children[0],v)

def ifXAtLeft(x,v):
    if x.children[0].leaf == v:
        return 1
    if x.children[0].leaf in specials and x.children[0].children.leaf == v:
        return 2
    else:
        return 10

def solveIdentities(temp, c1, l, c2):
    temp2 = temp
    x = findPointer(temp, c1, l, c2)
    while (temp.children != []):
        if temp.leaf not in specials and x in temp.children:
            if x == temp.children[0]:
                temp.children[0] = eqNode("INT",children=None,leaf=1)
                return temp2
        elif temp.leaf in specials:
            return temp2
        temp = temp.children[0]
    temp = temp2
    while (temp.children != []):
        if temp.leaf not in specials and x in temp.children:
            if x == temp.children[1]:
                temp.children[1] = eqNode("INT",children=None,leaf=1)
                return temp2
            elif temp.leaf in specials:
                return temp2
        temp = temp.children[1]
    return temp2

def ifIdentityLeft(x):
    if x.children != [] and x.leaf not in specials:
        if (x.leaf == '+') and x.children[0].leaf == '^' and x.children[1].leaf == '^' and \
            ((x.children[0].children[0].leaf == "sin" and  x.children[1].children[0].leaf == "cos" and x.children[0].children[1].leaf == x.children[1].children[1].leaf == 2) or\
            (x.children[0].children[0].leaf == "cos" and  x.children[1].children[0].leaf == "sin" and x.children[0].children[1].leaf == x.children[1].children[1].leaf == 2)):
            return 10;
        else:
            return ifIdentityLeft(x.children[0]) + ifIdentityLeft(x.children[1])
    elif x.children == []:
        return 0

print p.children[1].children[0].type
print repr(p)
print isSimplified(p,'x')
#print ifIdentityLeft(p)
#print findDepthOfX(p.children[0], 'x')
#print actions(p)
#print findOperations(p)
#print ifXInLeft(p,'x')
#print repr(solveIdentities(p,'^','+','^'))
#print repr(p)
#print repr(inverseIdentity(p,'+','*','^'))
#print repr(p2)
#print repr(squarIt(p))
#print repr(unLogIt(p))
#print isX(p.children[1],'x')
#print isSimplified(p.children[1],'x')

#print h(p)
#print p.children[0].children.leaf == 'x'

#for i in xacts.values():
 #   print result(i[1],i)

#print newActions(currentState)
#print isSimplified(p,'x')
#print actions(p)
#print repr(p)
#print solveThis(p)
#print goal_test(p)
#print repr(inverseIdentity(p,'x','-',3))
#print str(simplify(p,1,'+',2))

#x = findPointer(p,1,'+',1)
#print repr(x)
#print repr((q.children).leaf)
#ptry = makeCopy(p)
#print repr(p)
#print repr((p.children[1]).children)
#print repr(p)
#x = p.children[1]
#inverseIdentity(p, x)
#print repr(p)

#r = simplify(p,1,'+',2)
#print str(r)
#p.children[1] = solveThis(q)
#print str(r)
#print repr(r)
#print repr(p)
#print q.leaf


