# todo: for the grammar
#       S is start symbol
#       A <- B
#       B <- A
# we get an infinite loop, i.e. a series that never stops, how can we find that?

import re   # regular expression tools

# note that non-terminal S must be start symbol with only one rule
example = 3
if example == 0:
    startsymbol = 'E'
    lhs = ['E', 'E', 'E']       # non-terminal symbols
    rhs = ['E+E', 'E*E', 'iEE']      # patterns
    prec = [1, 2, 0]              # precedences
    left = [True, True, True]  # left-associative yes or no
elif example == 1:
    startsymbol = 'E'
    lhs = ['E', 'E', 'T', 'T', 'F', 'F']       # non-terminal symbols
    rhs = ['E+T', 'T', 'T*F', 'F', '(E)', 'i']      # patterns
    prec = [0,0,0,0,0,0]              # precedences
    left = [True, True, True, True, True, True]  # left-associative yes or no
elif example == 2:
    startsymbol = 'E'
    lhs = ['E', 'E']       # non-terminal symbols
    rhs = ['EE', 'i']      # patterns
    prec = [0,0]              # precedences
    left = [True, True]  # left-associative yes or no
elif example == 3:
    startsymbol = 'E'
    lhs = ['A', 'E', 'E']         # non-terminal symbols
    rhs = ['E', 'E+E', 'iEAA']    # patterns
    prec = [2, 1, 0]              # precedences
    left = [True, False, False]   # left-associative yes or no
else:
    raise 'Unknown example.'

def unique(l):  # return nl
    "Copies l with no element twice."
    nl = []   # list with unique elements
    for e in l:
        if e not in nl:
            nl += [e]
    return nl   # end of unique(l)

keywords = ''
for r in rhs:
    keywords += r
keywords = unique(keywords)

# note that:
#    S -> EE
#    T -> EE
# is not allowed,
# all rules must be different
if rhs != unique(rhs):
    raise 'Lhs of grammar must be unique.'

def pfs(str): # return l
    "Generates a list of prefixes of a string."
    l = ['$']   # the resulting list
    for s in str:
        l += [ l[-1] + s ]
    return l # end of pfs(str)

def pfs_tree(rhs): # return (tree, prefix, rule)
    "Creates a prefix tree."
    tree = [[]]     # contains lists of children, initially only root
    prefix = ['$']  # contains corresponding prefixes
    rule = [[]]     # contains rule-indices matching the prefix
    # first add an artificial prefix for the start symbol
    tree[0] += [1]   # new child for the root
    tree    += [[]]  # add an empty node
    prefix  += ['$'+startsymbol] # artificial prefix
    rule    += [[]] # artificial rule number
    for r in rhs:
        for p in pfs(r):
            if p in prefix:
                i = prefix.index(p) # keep current tree pos
                rule[i] += [rhs.index(r)]  # add rule-index
            else:
                j = len(prefix)  # index of new node
                tree[i] += [j]   # insert new prefixes into tree
                tree += [[]]     # add empty node
                prefix += [p]    # add prefix
                rule += [[rhs.index(r)]] # claim this prefix
                i = j            # keep current tree pos
    return (tree, prefix, rule) # end of pfs_tree(rhs)


def show_tree(tree, prefix, rule): # no return
    "Shows prefix tree by depth-first path."
    visited = [False for p in prefix]
    nodes = [0]   # stack, that keeps next node to visit
    children = [0] # stack, that keeps next child to visit
    while len(nodes) > 0:
        node  = nodes[-1]       # last element is current node
        child = children[-1]    # last element is current child
        if child == 0:
            if visited[node]:
                raise 'Given structure is not a tree.'
            # first time visit of node
            visited[node] = True
            print ' '*len(nodes), node, prefix[node]
        if child < len(tree[node]):
            # still subtrees to follow
            children[-1] += 1;       # next time the next child of node
            nodes.append(tree[node][child])  # push child onto stack
            children.append(0)       # push first grandchild onto stack
        else:
            # we are done with this node, thus backtrack
            nodes.pop()
            children.pop()
    # end of show_tree(tree, prefix):

def cpl_tree(tree, prefix, rule): # return (tree, prefix, rule)
    "Complete tree to graph for syntax analysis."
    # for a syntax analysis table there are some types of connections missing:
    # (i)  for each rule A->alpha with alpha not beginning with A
    #      all prefixes pointing at A should link to alpha, continue this for all
    #      non-terminals at the beginning of alpha
    # (ii) for each rule A->alpha
    #      all prefixes that contain maximally beginnings of alpha
    #      should link to the remaining part of alpha
    
    # (i)
    # first create tables for each non-terminal
    kids = {}
    for nt in unique(lhs):  # create empty dicts for each non-terminal
        kids[nt] = []       # to keep their children
    # initially fill the kids only according to rules
    for child in tree[0]:
        for j in rule[child]:
            if lhs[j] != rhs[j][0]:
                kids[lhs[j]].append(child)
    # next close the kids sets under substitution
    changed = True
    while changed:
        changed = False
        for j in range(len(lhs)):
            old = len(kids[lhs[j]])
            key = rhs[j][0]
            if key in unique(lhs):
                kids[lhs[j]] = unique(kids[lhs[j]] + kids[key])
                if old < len(kids[lhs[j]]):
                    changed = True
    # search for terminal symbols and add the possible follow states (kids)
    for p in prefix:
        if p[-1] in unique(lhs):
            i = prefix.index(p[:-1])  # index of node fulfilling (i)
            tree[i] = unique(tree[i] + kids[p[-1]])
    # (ii)
    for p in prefix[1:]:  # instead over all rules we loop over all prefixes
        for q in prefix:  # find q that should point to p
            if p[1:-1] == q[(-len(p)+2):]:  # does q match p
                i = prefix.index(q)   # candidate index for (ii)
                next = [prefix[j][-1] for j in tree[i]]  # next symbols for i
                # does a child continue with the same symbol as p
                if p[-1] not in next:
                    j = prefix.index(p)
                    if j not in tree[i]:
                        tree[i].append(j)
    return (tree, prefix, rule) # end of cpl_tree(tree, prefix, rule)


def sat(rhs, tree, prefix, rule): # return (action, jump)
    "Creates the syntax analysis table (action- and jump table)."
    action = []   # will be list of dictionaries indexed by symbols
    jump   = []   # similar
    for i in range(len(tree)):   # loop through all nodes in the graph
        action += [{}]        # new dictionary for each node
        jump   += [{}]        # new dictionary for each node

    # step 0: add reduce actions
    for k in range(len(rhs)):
        i = prefix.index('$'+rhs[k])  # a state that can reduce
        for key in keywords+['$']:
            action[i][key] = 'reduce'
            jump[i][key] = k  # NOTE: the index of the reduced rule
    
    # step 1: add shift actions/overwrite some reduces
    for i in range(len(tree)):   # loop through all nodes in the graph
        keys = []
        for child in tree[i]:
            key = prefix[child][-1]
            action[i][key] = 'shift'  # goto child
            jump[i][key] = child
            keys += [key]  # collect for sanity check
        # sanity check
        if keys != unique(keys):
            raise 'Tree has serious problem.'

    # step 2: add accept action
    #i = prefix.index('$'+startsymbol)
    #action[i]['$'] = 'accept'
    #jump[i]['$'] = 0  # NOTE: just artificial
        
    # step 3: resolve shift/reduce conflicts
    for k in range(len(rhs)):
        i = prefix.index('$'+rhs[k])  # a state that can reduce
        for child in tree[i]:
            key = prefix[child][-1]  # next letter
            # case 0: r is a prefix of some longer rule
            #         policy: always follow the longer rule
            #         this solves the 'dangling else' problem
            #   -> do nothing
            if prefix[i] != prefix[child][:-1]:
                if k in rule[child]:
                    # case 1: we are jumping back into the same rule
                    # use associativity to decide
                    if left[k]:
                        # change from shift to reduce
                        action[i][key] = 'reduce'
                        jump[i][key] = k  # NOTE: the index of the reduced rule
                else:
                    # case 2: we are jumping somewhere else, check precedences
                    precs = [prec[j] for j in rule[child]]
                    if max(precs) < prec[k]:
                        # case 2.1: all precs are smaller than prec[k]
                        # change action from shift to reduce
                        action[i][key] = 'reduce'
                        jump[i][key] = k  # NOTE: the index of the reduced rule
                    elif min(precs) > prec[k]:
                        # case 2.2: all precs are larger than prec[k]
                        # keep the shift for key -> do nothing
                        i = i  # do nothing, how can you do nothing in python?
                    else:
                        # case 2.3: precedences are not comparable by preference
                        # thus unresolvable shift/reduce conflict
                        msg = 'Warning: shift/reduce conflict can not be resolved.\n'
                        msg += 'reduce: prec %5.2f  ' % prec[k] + prefix[i] + '\n'
                        for j in rule[child]:
                            msg += 'shift:  prec %5.2f  ' % prec[j] + prefix[child] + '\n'
                        msg += 'Resolving to shift.\n'
                        print msg
    return (action, jump) # end of sat(rhs, tree, prefix, rule)


(tree, prefix, rule) = pfs_tree(rhs)
(tree, prefix, rule) = cpl_tree(tree, prefix, rule)
(action, jump) = sat(rhs, tree, prefix, rule)

def parse(inputstr):
    "Parses a string according to the sat."
    word = inputstr + '$'
    stack = [0]
    a = ''
    while True:
        line = str(stack) + ' '*(60-len(str(stack)))
        line += ' %20s' % word
        s = stack[-1]     # state
        w = word[0]       # next letter
        # are we done yet?
        if w=='$' and len(stack)==3 and s == prefix.index('$'+startsymbol):
            line += ' accept'
            print line
            break
        # else follow the state machine
        if w not in action[s].keys():
            raise 'Parse error: stack=' + str(stack) + '  word=' + word
        a = action[s][w]
        if a == 'shift':
            word = word[1:]
            stack.append(w)
            stack.append(jump[s][w])
            line += ' shift ' + w
        elif a == 'reduce':
            i = jump[s][w]
            terminal = lhs[i]
            stack = stack[:-2*len(rhs[i])]
            nextstate = jump[stack[-1]][terminal]
            stack.append(terminal)
            stack.append(nextstate)
            line += ' reduce ' + lhs[i] + ' -> ' + rhs[i]
        print line
