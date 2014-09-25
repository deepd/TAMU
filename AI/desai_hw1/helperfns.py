__author__ = 'deep'

# Function to do arithmetic operations for more than one levels
import eqparser
from eqparser import Node as eqNode
import operator
import sys

ops = { "+": operator.add, "-": operator.sub, "*": operator.mul, "/":operator.div, "^":operator.pow, "=":operator.eq }
specials = ["sin", "cos", "tan", "log", "ln", "sqrt"]
arithtypes = ["INT", "FLOAT"]
UNDEFINED = 100000

# Function to solve it

def solveThis(x):
    c1 = x.children[0]
    c2 = x.children[1]
    if c1.children != []:
        if c1.children[0].leaf in specials:
            return c1.children[0]
        if c1.children[1].leaf in specials:
            return c1.children[1]
        c1 = solveThis(c1)
    if c2.children != []:
        if c2.children[0].leaf in specials:
            return c2.children[0]
        if c2.children[1].leaf in specials:
            return c2.children[1]
        c2 = solveThis(c2)
    opr = x.leaf
    res = ops[opr](c1.leaf,c2.leaf)
    return eqNode(c1.type,children=None,leaf=res)

# Function to simplify one arithmetic

def simplify(temp,c1,l,c2):
    try:
        v1 = int(c1) and int(c2)
        x = findPointer(temp,c1,l,c2)
        if x == None:
            return temp
        c1 = x.children[0]
        c2 = x.children[1]
        if c1.leaf in specials or c2.leaf in specials:
            return temp
        try:
            if l == '/':
                if c1.leaf % c2.leaf == 0:
                    x.leaf=ops[x.leaf](c1.leaf,c2.leaf)
                else:
                    x.leaf=ops[x.leaf](float(c1.leaf),float(c2.leaf))
            else:
                x.leaf=ops[x.leaf](c1.leaf,c2.leaf)
        except ZeroDivisionError:
            sys.exit("x = undefined")
        x.children=[]
        x.type=c1.type
        return temp
    except ValueError:
        return temp

# Function to do apply an inverse identity on given tree with child1 leaf and child2

def inverseIdentity(temp, c1, l, c2):
    temp2 = temp
    x = findPointer(temp, c1, l, c2)
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
        if temp.leaf not in specials and x in temp.children:
            if x == temp.children[0]:
                temp.children[0] = x.children[0]
                x.children[0] = temp2.children[1]
                temp2.children[1] = x
                return temp2
        elif temp.leaf in specials:
            return temp2
        temp = temp.children[0]
    temp = temp2
    while (temp.children != []):
        if temp.leaf not in specials and x in temp.children:
            if x == temp.children[1]:
                temp.children[1] = x.children[0]
                x.children[0] = temp2.children[0]
                temp2.children[0] = x
                return temp2
            elif temp.leaf in specials:
                return temp2
        temp = temp.children[1]
    return temp2

# Function for commutative property

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
        copyNode = eqNode(x.type, children=x.children,leaf=x.leaf)
    elif x.leaf in ops.keys():
        copyNode = eqNode(x.type, children=[makeCopy(x.children[0]),makeCopy(x.children[1])],leaf=x.leaf)
    else:
        copyNode = eqNode(x.type, children=None, leaf=x.leaf)
    return copyNode

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

# Function to find if it is simplified enough

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
    
# Function to find out if simplified just for leaf+child1+child2

def isSimplifiedSubTree(x):
    if x.leaf in specials:
        return True
    elif x.leaf in ops.keys():
        if x.children[0].type in arithtypes and x.children[1].type in specials:
            return True
        elif x.children[0].type in arithtypes and x.children[1].type in arithtypes:
            return False
        elif x.children[0].type in arithtypes and x.children[1].type in ["VARIABLENAME", "SYMBOL"]:
            return True
        elif x.children[0].type in specials and x.children[1].type == arithtypes:
            return True
        elif x.children[0].type in specials and x.children[1].type in ["VARIABLENAME", "SYMBOL"]:
            return True
        elif x.children[0].type in specials and x.children[1].type in specials:
            return True
        elif x.children[0].type in ["VARIABLENAME", "SYMBOL"] and x.children[1].type in ["VARIABLENAME", "SYMBOL"]:
            return True
        elif x.children[0].type in ["VARIABLENAME", "SYMBOL"] and x.children[1].type in arithtypes:
            return True
        elif x.children[0].type in ["VARIABLENAME", "SYMBOL"] and x.children[1].type in specials:
            return True
    else:
        return False

# Function to see if x is there in this subtree

def isX(x, var):
    if x.leaf in specials:
        return False
    elif x.leaf == var:
        return True
    elif x.children == [] and x.leaf != var:
        return False
    elif x.children != []:
        return (isX(x.children[0], var) or isX(x.children[1],var))

# Function to square the whole equation

def squarIt(x, v):
    temp = x
    temp2 = eqNode("BINARYOP",children=[temp.children[1],eqNode("INT",children=None,leaf=2)],leaf='^')
    temp.children[0] = eqNode("VARIABLENAME", children=None,leaf=v)
    temp.children[1] = temp2
    return temp

# Function to unLog the whole equation

def unLogIt(x, v):
    temp = x
    temp2 = eqNode("BINARYOP",children=[eqNode("INT",children=None,leaf=10),temp.children[1]],leaf='^')
    temp.children[0] = eqNode("VARIABLENAME", children=None,leaf=v)
    temp.children[1] = temp2
    return temp

# Function to unLn the whole equation

def unLnIt(x, v):
    temp = x
    temp2 = eqNode("BINARYOP",children=[eqNode("SYMBOL",children=None,leaf='e'),temp.children[1]],leaf='^')
    temp.children[0] = eqNode("VARIABLENAME", children=None,leaf=v)
    temp.children[1] = temp2
    return temp

# Function to find out number of operations left in equation

def findOperations(x):
    if x.children != [] and x.leaf in ops.keys():
        if x.leaf == '=' and x.leaf not in specials:
            return findOperations(x.children[0]) + findOperations(x.children[1])
        else:
            return 1 + findOperations(x.children[0]) + findOperations(x.children[1])
    else:
        return 0

# Function to find depth of variable v

def findDepthOfX(x, v):
    if x.leaf == v:
        return 0
    elif x.children != [] and x.leaf not in specials:
        return max(1 + findDepthOfX(x.children[0], v), 1 + findDepthOfX(x.children[1], v))
    elif x.leaf in specials and x.children.leaf == v:
        return 2
    else:
        return 0

# Function to check if v in left subtree

def ifXInLeft(x,v):
    if (x.children == [] and x.leaf != v):
        return 10
    elif x.leaf in specials and x.children.leaf == v:
        return 2
    elif x.leaf in specials:
        return 10
    elif x.leaf == v:
        return 1
    else:
        return ifXInLeft(x.children[0],v)

# Function to check if v is left child of root

def ifXAtLeft(x,v):
    if x.children[0].leaf == v:
        return 1
    if x.children[0].leaf in specials and x.children[0].children.leaf == v:
        return 2
    else:
        return 10

# Function to solve sin^2 + cos^2 or cos^2 + sin^2

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

# Function to check if identities left in the tree

def ifIdentityLeft(x):
    if x.children != [] and x.leaf not in specials:
        if (x.leaf == '+') and x.children[0].leaf == '^' and x.children[1].leaf == '^' and \
            ((x.children[0].children[0].leaf == "sin" and  x.children[1].children[0].leaf == "cos" and x.children[0].children[1].leaf == x.children[1].children[1].leaf == 2) or\
            (x.children[0].children[0].leaf == "cos" and  x.children[1].children[0].leaf == "sin" and x.children[0].children[1].leaf == x.children[1].children[1].leaf == 2)):
            return 10;
        else:
            return ifIdentityLeft(x.children[0]) + ifIdentityLeft(x.children[1])
    else:
        return 0