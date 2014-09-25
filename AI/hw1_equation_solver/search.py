from utils import *
from helperfns import *
import math, random, sys, time, bisect, string
counter = 0
currentstate = 0

#______________________________________________________________________________

class Problem(object):
    """The  class for a formal problem.  You should subclass
    this and implement the methods actions and result, and possibly
    __init__, goal_test, and path_cost. Then you will create instances
    of your subclass and solve them with the various search functions."""

    def __init__(self, initial, goal=None):
        """The constructor specifies the initial state, and possibly a goal
        state, if there is a unique goal.  Your subclass's constructor can add
        other arguments."""
        self.initial = initial; self.goal = goal

    def actions(self, state):
        """Return the actions that can be executed in the given
        state. The result would typically be a list, but if there are
        many actions, consider yielding them one at a time in an
        iterator, rather than building them all at once."""
        

    def result(self, state, action):
        """Return the state that results from executing the given
        action in the given state. The action must be one of
        self.actions(state)."""
        

    def goal_test(self, state):
        """Return True if the state is a goal. The default method compares the
        state to self.goal, as specified in the constructor. Override this
        method if checking against a single self.goal is not enough."""
        return state == self.goal

    def path_cost(self, c, state1, action, state2):
        """Return the cost of a solution path that arrives at state2 from
        state1 via action, assuming cost c to get up to state1. If the problem
        is such that the path doesn't matter, this function will only look at
        state2.  If the path does matter, it will consider c and maybe state1
        and action. The default method costs 1 for every step in the path."""
        return c + 1

    def value(self, state):
        """For optimization problems, each state has a value.  Hill-climbing
        and related algorithms try to maximize this value."""
        
#______________________________________________________________________________

class Node:
    """A node in a search tree. Contains a pointer to the parent (the node
    that this is a successor of) and to the actual state for this node. Note
    that if a state is arrived at by two paths, then there are two nodes with
    the same state.  Also includes the action that got us to this state, and
    the total path_cost (also known as g) to reach the node.  Other functions
    may add an f and h value; see best_first_graph_search and astar_search for
    an explanation of how the f and h values are handled. You will not need to
    subclass this class."""

    def __init__(self, state, parent=None, action=None, path_cost=0):
        "Create a search tree Node, derived from a parent by an action."
        update(self, state=state, parent=parent, action=action,
               path_cost=path_cost, depth=0)
        global currentstate
        currentstate = state
        if parent:
            self.depth = parent.depth + 1

    def __repr__(self):
        return "Produces Output: %s" % (self.state,)

    def expand(self, problem):
        "List the nodes reachable in one step from this node."
        #print problem.actions(self.state).values()
        return [self.child_node(problem, action)
                for action in problem.actions(self.state).values()]


    def child_node(self, problem, action):
        "Fig. 3.10"
        next = problem.result(self.state, action)
        #print repr(next)
        if next != False:
            return Node(next, self, action,
                    problem.path_cost(self.path_cost, self.state, action, next))
        else:
            return None

    def solution(self):
        "Return the sequence of actions to go from the root to this node."
        return [node.action for node in self.path()[1:]]

    def path(self):
        "Return a list of nodes forming the path from the root to this node."
        node, path_back = self, []
        while node:
            path_back.append(node)
            node = node.parent
        return list(reversed(path_back))

    # We want for a queue of nodes in breadth_first_search or
    # astar_search to have no duplicated states, so we treat nodes
    # with the same state as equal. [Problem: this may not be what you
    # want in other contexts.]

    def __eq__(self, other):
        return isinstance(other, Node) and self.state == other.state

    def __hash__(self):
        return hash(self.state)

#______________________________________________________________________________


class equationSolver(Problem):
    """
    Actual Class for this Problem
    """

    def __init__(self, initial, goal=None, variable=None):
        self.initial = initial;
        self.variable = variable;


    def actions(self, x):
        global currentstate
        global counter
        acts = {}
        if x.children != [] and x.leaf not in specials:
            if (x.leaf == '+') and x.children[0].leaf == '^' and x.children[1].leaf == '^' and \
                ((x.children[0].children[0].leaf == "sin" and  x.children[1].children[0].leaf == "cos" and x.children[0].children[1].leaf == x.children[1].children[1].leaf == 2) or\
                (x.children[0].children[0].leaf == "cos" and  x.children[1].children[0].leaf == "sin" and x.children[0].children[1].leaf == x.children[1].children[1].leaf == 2)):
                acts[counter] = [solveIdentities,makeCopy(currentstate),x.children[0].leaf,x.leaf,x.children[1].leaf]
                counter+=1
                acts = dict(acts.items() + self.actions(x.children[0]).items() + self.actions(x.children[1]).items())
                return acts
            if  x.leaf == '=' and x.children[0].leaf == "sqrt" and x.children[0].children.leaf == self.variable :
                acts[counter] = [squarIt,makeCopy(currentstate),self.variable]
                counter+=1
                acts = dict(acts.items() + self.actions(x.children[1]).items())
                return acts
            elif x.leaf == '=' and x.children[0].leaf == "log" and x.children[0].children.leaf == self.variable:
                acts[counter] = [unLogIt,makeCopy(currentstate),self.variable]
                counter+=1
                acts = dict(acts.items() + self.actions(x.children[1]).items())
                return acts
            elif x.leaf == '=' and x.children[0].leaf == "ln" and x.children[0].children.leaf == self.variable:
                acts[counter] = [unLnIt,makeCopy(currentstate),self.variable]
                counter+=1
                acts = dict(acts.items() + self.actions(x.children[1]).items())
                return acts
            elif x.children[0].children != [] and x.children[1].children == [] and (x.leaf == "+" or x.leaf == "-" or x.leaf == "*" or x.leaf == "/"):
                acts[counter] = [inverseIdentity,makeCopy(currentstate),x.children[0].leaf,x.leaf,x.children[1].leaf]
                counter+=1
                if x.leaf == '+' or x.leaf == '*':
                    acts[counter] = [commutative,makeCopy(currentstate),x.children[0].leaf,x.leaf,x.children[1].leaf]
                    counter+=1
                acts = dict(acts.items() + self.actions(x.children[0]).items() + self.actions(x.children[1]).items())
                return acts
            elif x.children[0].children == [] and x.children[1].children != [] and (x.leaf == "+" or x.leaf == "-" or x.leaf == "*" or x.leaf == "/"):
                acts[counter] = [inverseIdentity,makeCopy(currentstate),x.children[0].leaf,x.leaf,x.children[1].leaf]
                counter+=1
                if x.leaf == '+' or x.leaf == '*':
                    acts[counter] = [commutative,makeCopy(currentstate),x.children[0].leaf,x.leaf,x.children[1].leaf]
                    counter+=1
                acts = dict(acts.items() + self.actions(x.children[0]).items() + self.actions(x.children[1]).items())
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
                acts = dict(acts.items() + self.actions(x.children[0]).items() + self.actions(x.children[1]).items())
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
                acts = dict(acts.items() + self.actions(x.children[0]).items() + self.actions(x.children[1]).items())
                return acts
            else:
                acts[counter] = [simplify,makeCopy(currentstate),x.children[0].leaf,x.leaf,x.children[1].leaf]
                counter+=1
                acts[counter] = [commutative,makeCopy(currentstate),x.children[0].leaf,x.leaf,x.children[1].leaf]
                counter+=1
                acts = dict(acts.items() + self.actions(x.children[0]).items() + self.actions(x.children[1]).items())
                return acts
        else:
            return {}

    def result(self, x, act):
        if len(act) > 3 :
            return act[0](act[1],act[2],act[3],act[4])
        else:
            return act[0](act[1],act[2])


    def goal_test(self, state):
        """Return True if the state is a goal. The default method compares the
        state to self.goal, as specified in the constructor. Override this
        method if checking against a single self.goal is not enough."""
        if state.children[0].leaf != self.variable:
            return False
        else:
            return isSimplified(state,self.variable)

    def path_cost(self, c, state1, action, state2):
        """Return the cost of a solution path that arrives at state2 from
        state1 via action, assuming cost c to get up to state1. If the problem
        is such that the path doesn't matter, this function will only look at
        state2.  If the path does matter, it will consider c and maybe state1
        and action. The default method costs 1 for every step in the path."""
        return c + 1

    def value(self, state):
        """For optimization problems, each state has a value.  Hill-climbing
        and related algorithms try to maximize this value."""

    def best_first_graph_search(self, problem, f):
        """Search the nodes with the lowest f scores first.
        You specify the function f(node) that you want to minimize; for example,
        if f is a heuristic estimate to the goal, then we have greedy best
        first search; if f is node.depth then we have breadth-first search.
        There is a subtlety: the line "f = memoize(f, 'f')" means that the f
        values will be cached on the nodes as they are computed. So after doing
        a best first search you can examine the f values of the path returned."""
        global currentstate
        f = memoize(f, 'f')
        node = Node(problem.initial)
        if problem.goal_test(node.state):
            return node
        frontier = PriorityQueue(min, f)
        frontier.append(node)
        explored = set()
        while frontier:
            node = frontier.pop()
            currentstate = node.state
            #print node
            if problem.goal_test(node.state):
                return node
            explored.add(node.state)
            for child in node.expand(problem):
                if child.state not in explored and child not in frontier:
                    #print child
                    frontier.append(child)
                elif child in frontier:
                    incumbent = frontier[child]
                    if f(child) < f(incumbent):
                        del frontier[incumbent]
                        frontier.append(child)
        return None


    def astar_search(self, problem, h=None):
        """A* search is best-first graph search with f(n) = g(n)+h(n).
        You need to specify the h function when you call astar_search, or
        else in your Problem subclass."""
        h = memoize(problem.h, 'h')
        return self.best_first_graph_search(problem, lambda n: n.path_cost + h(n))

    def h(self, n):
        """
        h function latest
        """
        x = n.state
        return 2*findOperations(x) + findDepthOfX(x,self.variable) + ifXInLeft(x,self.variable) + ifXAtLeft(x,self.variable) + ifIdentityLeft(x)

    def hOld(self, n):
        "h function old"
        x = n.state
        hvalue = 0
        temp1 = x.children[0]
        temp2 = x.children[1]
        if isSimplified(x,self.variable) and x.children[0].leaf == self.variable:
            return 0
        elif isSimplified(x,self.variable) and x.children[0].leaf != self.variable:
            hvalue+=5
        if x.children[0].leaf in ["sqrt","log","ln"]:
            hvalue+=50
        if isX(temp2, self.variable):
            hvalue+=100
        else:
            if temp1.children != [] and temp1.leaf not in specials and temp1.children[1] == self.variable:
                hvalue+=10
        while (temp2.children != []):
            if temp2.leaf in specials:
                break
            elif not isSimplifiedSubTree(temp2):
                hvalue+=2
            elif temp2.children[0].type in ops.keys():
                hvalue+=2
            temp2 = temp2.children[1]
        while (temp1.children != []):
            if temp1.leaf in specials:
                break
            elif not isSimplifiedSubTree(temp1):
                hvalue+=5
            elif temp1.children[1].type in ops.keys():
                hvalue+=5
            temp1 = temp1.children[0]
        return hvalue



#______________________________________________________________________________

