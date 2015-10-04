﻿#summary Soar Kernel notes



# List of Terms #

(jzxu)

Here's a list of terms and abbreviations used in the source code of the Soar
kernel whose meanings might not be obvious.


---

**slot**

From John Laird:

> We used to select more than just the operator (state, problem space, and goal)
> all together this was the context. Slots were for things that can be selected,
> so there was a slot for each of those. Now there is just a slot for the
> operator, although some of that language might have bled over to selection of
> values for non-operator attributes. In general they are an out of date
> terminology.

Slots are contained by identifiers, and hold all the preferences associated
with the identifier, including acceptable wme preferences. Each identifier can
have multiple slots, which can be accessed via the `prev` and `next` fields
in the slot structure. Operator preferences are held in `context` slots,
which are identified by the isa\_context\_slot flag.


---

**tc**

transitive closure


---

**match goal**

The lowest goal (biggest number after the "S") that an instantiation of the LHS
of a production matches on. Part of the instantiation structure.


---

**potential** (backtracing)

condition whose id is instantiated on a symbol linked from both the current
goal and a higher goal, and is tested for in the production's LHS using a path
from the current goal.

For example, if the following wmes are in working memory

```
(S1 ^foo F1)
(S2 ^bar F1)
(F1 ^baz B1)
(S2 ^superstate S1)
```

and this production was backtraced through

```
sp {example
   (<s> ^bar <b>)
   (<b> ^baz <z>)
-->
   ...}
```

then `(<b> ^baz <z>)` would be a potential condition.


---

**potential** (life)

abstract invented concept that actually has no real meaning


---

**tm**

temporary memory. I believe that any preference that is currently valid in Soar
(either they are o-supported or the instantiation that generated them still
matches) is in temporary memory. Once a preference is no longer valid, it is
taken out of temporary memory (which involves setting the `in_tm` flag to
false, and taking them off the `preferences` array and `all_preferences`
lists on the slot they're on).


---

**clone** (preference)

a copy of a preference that is the result of a subgoal. While the inst pointer
of the original preference points to the instantiation of the rule that fired
in the subgoal to create the result, the inst pointer of the clone points to
the newly created chunk or justification. Therefore, the preference and its
clone exist on different match goals, and hence different match goal levels.


---

**instantiation**

a particular match of a production in working memory, and the preferences
generated


---

**DEBUG\_MEMORY**

if this flag is defined, the contents of freed memory locations in the memory
pools are memset to 0xBB


---

# Old Soar Kernel Code Guide #

No, really, this page is under development. --Karen 12:25, 13 Feb 2007 (EST)

## "But where can I start?" ##

In a nutshell:

The Soar Kernel is a very object-oriented, structured set of code.  If you
don't understand the basic Soar execution cycle and its phases, the source code
won't help you.  You should start by reading the introductory material in the
Soar Tutorials that are bundled with the releases (in the Documents directory).

Then read the first four chapters of the Soar Manual,   "Introduction" thru
"Learning"

## Basic code execution ##

## Data Structures ##

All of the structures are well-commented in the header files.  In earlier
versions of Soar (up thru 8.3.5), the single header file "soarkernel.h" defined
all the common structures used throughout the code.  I still find it the
easiest place to search for information eventhough it's a very large file as
code goes.  In 8.6.**and later, the header file was separated by function into
many smaller files which can be found in "Core/SoarKernel/include"**

The agent\_struct defined in agent.h includes all the system parameters,
counters, variables, and pointers to all the other structures used by an agent
(wmes, productions, hash tables, memory\_pools, formats, etc etc).  It's a BIG
structure, tough to read, and includes a lot of detailed comments.  But if it
isn't defined or allocated here, the agent doesn't know about it.

Chances are you will never modify any structures in the rete, lexer, hash
tables, or backtracing facilities, but you should know that they exist.  If you
do start to muck with these structures, you better know what you are doing.
Changes here can greatly impact performance and even whether or not Soar will
run properly.  Several structures require their members to be defined in
specific order and are commented appropriately.

Structures that you should familiarize yourself with are symbols (a typedef'd
union!!), wmes, productions, instantiations, preferences, and (eventually)
memory pools.

  * `symbol_struct` is in symtab.h, _everything_ in soar boils down to some kind of symbol. See [[Development/Symbol]] for details.
  * `wme_struct` is in wmem.h, defines the working memory elements of an agent
  * `production_struct` is in production.h, these are the productions as loaded into Soar and stored in the rete.
  * `instantiations` are in instantiation.h, these store the bindings of productions as they are matched in the rete.  Instantiations whose conditions all match are fired during specific phases -- meaning their actions are executed:  preferences are asserted to create wmes, and/or RHS actions are executed.
  * `preference_struct` is defined in gdatastructs.h store a pointer to the instantiation that created them, and when the instantiation no longer matches, the preference is retracted.

## I want to add a new link! ##

Existing links include input/output and reward links.  Instructions on how to
make your own are here: [[Development/Links]]

# Things to add #

(I can tell that Bob found this page...)

A lot of topics are in the Soar FAQ

  * Basic structure of how critical code works (e.g. decision procedure is a big switch statement, how printing/XML generation works)
  * Locations of critical code (e.g. decision procedure, preference procedure, scheduler)
  * Union basics (most people don't know what unions are)
    * see Kernigan and Ritchie
    * Unions are a data structure that can have multiple data-dependent ways of storing and accessing  information, much like (but better than) overloading methods in C++.
  * Explain how `sysparams` work (e.g. how they are set/used, how to add a new one)
    * sysparams are just an array of agent parameters that store settings for runtime switches.  Most of the sysparams are either True/False, or can take on some enum value.  Setting a sysparam is easy -->> see init\_soar.c for initializing and setting values.  Search the code for "set\_sysparams" to see examples.
    * To add a sysparam, see gsysparams.h (although that file MUST be renamed or folded into another header when gSKI removed).  The code depends on looping over HIGHEST\_SYSPARAM, so make sure it's always equal to the last one in the set of definitions.
    * When is it a sysparam, and when is it part of the agent struct?   Depends what you are using it for, and whether it needs to be exposed for the user interface.  If its a user-controlled setting, it should definitely be a sysparam.
  * What is a `slot`

From John: "We used to select more than just the operator (state, problem
space, and goal) and all together this was the context. Slots were for things
that can be selected, so there was a slot for each of those. Now there is just
a slot for the operator and although some of that language might have bled over
to selection of values for non-operator attributes. In general they are an out
of date terminology."

  * Basics of how wme changes are done in parallel (i.e. explain do\_buffered\_wm\_and\_ownership\_changes)
  * Difference between wme and input\_wme (and any other wmes there might be)
  * Where/how to add new links (e.g. ep-mem link, RL link, etc)
  * Explain `memory pool basics`

  * Basics of bit manipulations that are used (unless this is rete-only, in which case don't bother)
    * I think the rete is the only place bit manipulations occur.  Bit manipulations are extremely fast.  If you can guarantee your raw data structure, you can shift registers instead of calling complex instructions to go very fast.  Compilers hide this from you, but don't always know when they can optimize.
  * Explain transitive closure and tc\_num
  * What all the Soar kernel STL-like data structures are (e.g. lists, hash tables, growable strings, others?) and how to use them.
  * Ref counting (link to [[Development/Memory Leaks|Tracking down memory leaks]])


# Old Kernel TODO/BUGBUG audit #

## TODOs ##

```
agent.cpp:405:     TODO: Not clear why callbacks need to take the agent pointer essentially twice.

init_soar.cpp:1650:    // TODO: Make sure that this ifdef is neccessary for the timeing code

rhsfun.cpp:647:   /* TODO: We need a serious reference counting audit of the kernel But I think
```

## BUGBUGs ##

```
agent.h:59:/* RBD BUGBUG more comments here, or should this stuff be here at all? */
   some #define's for enums -- might want to fix this...

backtrace.h:46: /* RBD BUGBUG more comments here */ 
   changed to "NOTE"

chunk.cpp:57:   "next_result" field of the preference structures.  (BUGBUG: to save space 
   Old comment to use a cons to store result.  Changed "BUGBUG" to "NOTE"

chunk.h:23:/* RBD BUGBUG more comments here */ 
   regarding #defs for CHUNK_COND_TABLE sizes.  could add comments, or move def's

decide.cpp:791:  BUGBUG There is a problem here:  since the require/acceptable priority <br>
decide.cpp:1765:   potentially break some code, but it avoids the BUGBUG condition
   First comment anticipates the case where a WME has multiple prefs where REQUIRE is
   lower in the goalstack than an ACCEPTABLE, the stack gets popped, but the pref is
   referencing the defunct REQUIRE instead of the ACCEPTABLE.  The second comment refers
   to a code change in fake instantiations, changing the pref from REQUIRE to ACCEPTABLE,
   so as not to create the potential bug situation.   This is a tricky part of the code,
   so I left the BUGBUGs as is.

decide.cpp:3514:       BUGBUGBUG 
   This occurs in probabilistically_select routine, which was implemented for 
   NUMERIC_INDIFFERENCE mode.  Refers to need for error checking.

explain.cpp:272:  /* BUGBUG *** shouldn't have user interface stuff in kernel!! */ 
   yeah, right.  This is noting a place where info is printed to the user.  changed to bug noted:

explain.h:77:/* BUGBUG ms changes only really need tok (the tok from the p-node) 
   The ms_change_struct has pointers to both 'tok' and 'wme' and the comment says
   don't need both, but would have to change firer code.  We can live with as is.

lexer.cpp:52: *  BUGBUG There are still problems with Soar not being very friendly 
   Not a specific bug, just a comment that more support for users could be coded;  
   eg: balanced parens, bugs in productions, etc.  Maybe the clients can help...

lexer.cpp:369:/* --- BUGBUG: these routines are here because the ANSI routine strtod() isn't <br>
lexer.cpp:376:  /* BUGBUG without ANSI's strtod(), there's no way to check for floating
   These are some compiler-def'd checks for string support for strtod and strtoul

lexer.cpp:760:      /* BUGBUG if reading from top level, don't want to signal EOF */  <br>
lexer.cpp:789:      /* BUGBUG if reading from top level, don't want to signal EOF */ 
  in lex_vbar and lex_quote, this is a comment that we might incorrectly signal an EOF
  when we shouldn't.  This is a VERY old comment and doesn't seem to have been a problem.

lexer.cpp:1167:  BUGBUG: if the input line contains any tabs, the pointer comes out in 
  this is in print_location_of_most_recent_lexeme, which aids in debugging productions
  that can't be processed due to some error.  Not a critical bug...

osupport.cpp:256:   BUGBUG the code does a check of whether the lhs tests the match state via 
  pertains to longtime code for runtime o-support calculations

osupport.cpp:350:	   BUGBUG this check only looks at positive conditions.  we
  not an actual known bug, just anticipating that possibly ignoring negations could
  yield in this calculation, although it is assumed that any such production would get
  i-support

osupport.cpp:573:	   BUGBUG: this is not fully general; it does not rule out assiging
  SBH (Scott Huffman comment?) in section where calculating runtime o-support for
  operators, operator acceptable prefs are explicitly ruled out, but BUGBUG notes that
  it is not done in fully general way.  An example is provided.
  -- This is old code, so leave until future explorations of o-support

osupport.cpp:674:    /* BUGBUG With Doug's scheme, o_support_tc no longer needs to be a 
  a global agent variable o_support_tc could possibly be declared only locally.

osupport.cpp:777:   BUGBUG should follow ^object links up the goal stack if possible
  in find_known_goals.  routine finds variables that are certain to be bound to goals, 
  but doesn't follow any ^objects up the goal stack.  old code.

prefmem.cpp:15: *  BUGBUG  need some comments here
  changed to NOTE:    Problem is no header or file comments included...

prefmem.cpp:110:  /* BUGBUG check to make sure the pref doesn't have
  in make_preference, the comment specifies that should check value or referent
  to make sure doesn't have .isa_goal or .isa_impasse.  hmmm, ok thus far...

prefmem.cpp:335:  pref->slot = NIL;      /* BUGBUG use pref->slot in place of pref->in_tm? *?
  in remove_preference_from_tm.  need to investigate before changing.  not a problem?
  removed bugbug, left comment

print.cpp:173:    /* BUGBUG doesn't handle tabs correctly */ 
  in print_string, only if #ifdef __SC__, mishandles /t when setting printer_output_column

print.cpp:514:      /* BUGBUG if in context where id's could occur, should check possible id_flag
  in symbol_to_string when converting SYM_CONSTANT_SYMBOL_TYPE.  old issue, not sure if problem

print.h:200:extern void print_list_of_conditions(agent* thisAgent, condition *cond); /* BUGBUG comments */ 
  note to programmer to add comments.  deleted it.

production.h:50:      OPERAND_which_assert_list: (BUGBUG need info from REW or RCHONG) 
  variable in a struct which has no associated comments.  removed BUGBUG, left note.

recmem.cpp:198:   (BUGBUG I'm not sure this is really needed.)
  instantiate_rhs_value might have an extraneous parameter in arg list.  removed BUGBUG
  but left comment

recmem.h:47:/* RBD BUGBUG more comments here */
  somehow, I don't think Bob Doorenbos intends to add more comments at this point. removed.

reorder.cpp:16: *  BUGBUG comments here
  in head of file, no comments or description.  removed BUGBUG, left note.

reorder.cpp:850:      /* BUGBUG I'm not sure whether this can ever happen. */
  in reorder_simplified_conditions (code I've NEVER had occasion to look at!) this is
  a test to see if some LHS conditions are not connected, which may not be possible.  old code.

reorder.cpp:1059:    /* BUGBUG most people aren't going to understand this error message */
  in reorder_lhs.  I've never heard of people even SEEING this error message.  old code. left alone

reorder.h:8:   BUGBUG comments here
  in head of file, no comments or description.  removed BUGBUG, left note.

rete.cpp:3509:   BUGBUG should we check for duplicate justifications?
  pertains to add_production_to_rete.  valid comment, left alone.
 
rete.cpp:4104:/* BUGBUG clean this procedure up somehow? */
  pertains to rete_node_to_conditions, which is a lot if nested if-then stmts.
  old code.  left alone.  (LOTS of other code could be cleaned up first...)

rete.cpp:5601:  BUGBUG i haven't tested this with a production that has more than
  found in p_node_left_addition.  Comment left from RChong code for Operand, but not
  sure if still applies to Operand 2 (Soar 8), but I'll bet it does.  Left alone for now.
  Might ask Bob Wray to review it.  Or Ron.  applies when determining whether a match is IE or PE

rete.cpp:5675:			BUGBUG this check only looks at positive conditions.  we
  same section of code, determining support for a match.  here, we're looking at prods
  that aren't operator proposals to see how to classify them

rete.cpp:5851:/* BUGBUG shouldn't need to pass in both tok and w -- should have the
  in p_node_left_removal, could possibly pass one less arg.  left alone.

rhsfun_math.cpp:16: *  BUGBUG more comments here.  Nothing in soarkernel.h either.
  in head of file, no comments or description.  removed BUGBUG, left note.
```


---


# Thoughts on a potential rewrite #

Description From Bob Marinier 2003-10-23 22:50:51 (-) [reply](reply.md)

This is some of my thoughts on a potential rewrite of Soar. This bug is NOT
for discussing whether we should do a rewrite, it is for thinking about what
kinds of implementation details we may need to consider if we do decide to
rewrite Soar.

1. Written in C++
1.1 Object-oriented language will allow us to more easily create intuitive
structures (i.e. classes) which will make the code easier to understand, more
intuitive, more maintainable, and more extensible. The language will also
enforce certain constraints for us (i.e. private members, etc) which will help
limit instances of bad programming. We will also be able to use the STL,
which will make certain parts of the code much easier to write (and
potentially faster). The STL gives us much more power than Soar's current
home-grown containers (growable strings, doubly linked lists and hash tables).

1.2 While other tools (i.e. VisualSoar, the upcoming debugger) are written in
Java, I think that Java is a bad idea for this project. The first reason is
performance. While we should be prepared to accept a certain amount of
performance loss (we may have to trade some performance for maintainability),
we should still strive for high performance as much as we reasonably can.
While Java has made significant improvements in the last few years, it still
isn't as fast as well-written C++. But the biggest reason to go with C++ over
Java is for ease of hooking up to external environments like
UnrealTournament. While it's possible to call C/C++ code from Java via JNI,
it's not clear to me that it's easy to go the other way. Furthermore, even if
it's possible, there is probably a severe performance hit from doing so.
While we may be able to accept severe performance hits while debugging (like
we do now with Haunt and MOUT), we do not want the connection to Soar to be
inherently slow.

2. Use the newer .NET C++ for Windows development
2.1 The newer .NET IDE is significantly better than the old IDE for Windows
developers.
2.2 The newer .NET C++ is more standards compliant which will make writing
portable code easier.
2.3 Do NOT use the managed C++ stuff (i.e. C++ with garbage collection).
While this may become part of the C++ standard in the future, it isn't now so
we can't sacrifice the portability.

3. Use the STLPort version of the STL
3.1 STLPort is a freely available version of the STL based on SGI's
implementation. It is available at http://www.stlport.org/
3.2 STLPort is significantly faster than Microsoft's implementation and much
more portable than SGI's implementation.
3.3 STLPort has a "debugging" mode which can check array bounds, etc at run
time. Read about it here: http://www.stlport.org/doc/debug_mode.html
3.4 STLPort (like SGI's implementation), besides being fully compliant with
the ANSI standard, has important extensions to the current standard, including
ropes and hash tables. These extensions will very likely be made part of the
ANSI standard in the future.

4. Object Pooling
4.1 The current version of Soar uses object pools to make memory allocation
more efficient. However, it also makes many memory leaks impossible to detect
with standard tools like Purify (since the leak is to the pool and not the OS).
4.2 On the one hand it seems reasonable to not use object pooling unless
profiling reveals that it is necessary. However, by the time that we get to a
stage where we can test that, it may be very difficult to implement object
pooling. Perhaps we could implement object pooling using a check-out/check-in
model. In this model, the pool does not maintain pointers to the objects once
they are checked out. Instead, it relinquishes the pointer to whatever is
requesting the object. When the object is checked back in, then the pool
takes the pointer back. If the object is not checked in later when it is no
longer being used, then the leak will be to the OS and thus detectable. If
the object is checked in but some external reference is maintained and later
used, it should wreak havoc on the system and thus be easy to track down (as
opposed to the silent leaks we know we are getting now but can't track down).
Finally, because we are using the pools, we can still track object statistics
like memory usage (although some statistics, like number of objects created
since start, should be tracked in static class variables, not by the pools).

5. Use an autodocumenting tool
5.1 We should agree on and use some autodocumenting tool like Doxygen
(http://www.doxygen.org). These tools require the comments for classes,
methods, etc to be in a standard format using tags to identify various things
like parameters, etc. If we pick a tool to use, it will make documenting what
we do and understanding the code during the process and after it is done much
easier.

6. Some performance loss is acceptable
6.1 When Soar was rewritten in C from Lisp, it was 80 times faster. We may
not be able to maintain this high level of performance. Of course, we should
try, but if it comes down to maintainability versus speed, I vote for
maintainability. There are many places in the current version of Soar where
things are intertwined at too low of a level. For example, doing certain
kinds of calculations directly in the rete may be fast, but it makes
understanding (and modifying) that code very difficult. Of course, we should
have limits on how much slow down we are willing to accept. Off the top of my
head I think a 2x slow down is acceptable but a 10x is not. Profiling at
various stages during development should be done and will give us insight into
what parts of the developing code need work (or even a redesign) before we get
so far along that we can't do anything about it. Other large projects (for
example, Mozilla) actually do automatic profiling every day so that
performance loss can be tracked to particular check-ins, etc. I don't think
we need to be that vigorous about it, but some level of regular testing will
help a lot in minimizing performance loss (and who knows, it may even be
faster).

------- Comment #1 From Andrew Nuxoll 2003-10-24 12:46:23 (-) [reply](reply.md) -------

Summary: In terms of man hours, the most overwhelming part of
rewriting Soar will be verifying what we have written. This outweighs
(by at least an order of magnitude) considerations like what language
we are writing in or what development tools we use.

As was stated in the meeting, a rewritten Soar must have near 100% of
the functionality of the current Soar or it will not be accepted. One
has only to look at how many users are still using Soar7 to understand
the power of inertia.

This has two hidden, but overwhelming ramifications:
1. We must examine the existing source code completely to make sure
we are fully aware of its capabilities. A lot of functionality
has been added to the Soar kernel since it was written in C. Can
anyone, even John Laird, claim to know all of it. Clearly not.
2. We must be able to **PROVE** to ourselves and to the Soar community
that what we have written is virtually, functionally identical to
the Soar we are replacing.

To accomplish this, I recommend the following steps:

1. (estimated 6 man months) Examine Soar and document its
functionality entirely using a complete specification document.

2. (estimated 6 man months) Write a design document for a new Soar.

3. (estimated 1 man year) Build unit tests that link to the Soar
kernel library and test specific units of functionality as
documented in step 1.

4. (estimated 6 man months) Create a multitude of small Soar programs
that test high level functionality (e.g. chunking, sub-goaling,
stack unwind, GDS, i-support & o-support, etc.). These programs
should also act as unit tests (i.e., they output a success or fail
code depending upon whether they run successfully or not.)

5. (estimated 8 man months) Collect multiple (~10-20) existing Soar7
and Soar8 programs and projects of signficant size. Ideally this
collection would include Tac-Air Soar. Run them in a
deterministic manner (which will require tweaking them so that
Soar settles indifferent preferences and gensym calls
deterministically) in verbose mode. Record these log files.

6. (estimated 4 man years) Reimplement Soar. As each set of
functionality is implemented run the unit test to verify each
functional unit as it is completed.

7. (estimated 1 man year) Run all of the small soar programs created
in step 4 and verify them. Fix bugs.

8. (estimated 1 man year) Run all of the large soar programs from
step 5 and verify them. Fix bugs.

Total Verification/Documentation effort: ~7 man years
Total Development effort: ~2 man years
Total effort required: ~9 man years

No I am not exaggerating. I have a fair amount of confidence in this
estimate. I also believe that if we skip any of these steps we will
make the project **longer** and much more painful. Now there are
multiple competent developers in the Soar research group now. I think
that if we all stopped doing research and focused on this project, we
could complete it in about 3 years. Given that we're all doing
research, I think it's reasonable to estimate it will take more than
double that time.

This is not an argument for or against rewriting Soar. I agree that a
rewritten, object-oriented Soar would be a nice boon. In fact, I
predict the following benefits:

1. The new Soar will run **faster** (not slower) than the old Soar due
to the efficiency of tighter rebuild and despite the overhead of
using C++.
2. A better documented Soar kernel. All of the following would
probably exist in better form and more up to date:
- general source code documentation
- API documentation (see Bob's doxygen paragraph)
- better developer/advanced soar user documentation
3. Tight, clean integration with new tools (e.g., SoarDoc,
VisualSoar, a Soar Debugger and gSki)
4. A better organized development environment for future
development. We've been slowly moving this way for a while. I
think a redevelopment effort would help us cement that. (also see
Bob's comments)

I also predict the following costs:
1. 6 years of real time required to produce a stable, functionally
equivalent rewrite (see above)
2. Loss of a significant number of Soar users due to stagnant
development with the existing Soar.

:AMN:

------- Comment #2 From Bob Marinier 2003-10-24 14:24:45 (-) [reply](reply.md) -------

I agree that Andy's effort estimates are very realistic. I have the following
additional comments:

Even if we ultimately don't do the rewrite (i.e. lack of funding, John gets
hit by a bus, etc), it is still very valuable to complete steps 1-5. So, if
we get part way through the steps Andy has laid out but still fail to complete
the project, it won't be a complete waste by any means.

The one thing I'm not sure I agree with is that we would lose a significant
number of users. This presumes two things:

1) People want new releases of Soar fairly frequently. Given that a
significant number of people still use Soar7, I don't think this is true.

2) Development on the old version will stop during development of the new
version. Given that several people have research which depends on the
modifying the current kernel, I think that development on the old system will
continue. This gives us a moving target which makes things harder, but with
careful management I think we can still do this. We will have to require that
people who make permanent changes to the kernel (i.e. changes which will
become part of the official release) work with us to document their code at
the same level that we will have documented the preexisting code. We will
also require that they submit test programs which can be used to verify their
code. Given that they will already be doing this for their own purposes, it
should not be a big deal to make the process a little more formal.

What I'm thinking is that, just because we are doing a new version does not
mean that the old version has to be completely stagnant. It may be slowed
down significantly, but if we want to take a large step forward we may have to
risk taking a small step back.

Also, I think the idea of test suites which output a pass/fail for various
components is an excellent idea. This sort of thing can be automated; see,
for example:

http://porting.openoffice.org/irix/testing/OO641.html

I think if we start with the right tools it will make our lives a lot easier.

Finally, is there a standard way to document the specs for a software project
(i.e. for step 1)? I was just wondering what the most effective method for
documenting and understanding the current functionality was without resorting
to "the code is the documentation."

------- Comment #3 From Bob Marinier 2003-10-24 14:25:44 (-) [reply](reply.md) -------

Added Jon Voigt to CC list.

------- Comment #4 From Bob Marinier 2004-04-01 09:08:56 (-) [reply](reply.md) -------

Sorry, I just need to get some of this out of my system :)

Here's some pointers to coding techniques that may be useful:

Object pooling:
http://www.brent.worden.org/articles/1999/objectPools.html
Overrides new and delete to provide the pool. Also provides some simple
empirical results.

auto\_ptr:
http://www.gotw.ca/publications/using_auto_ptr_effectively.htm
The standard auto\_ptr seems pretty cool. It can't be used with the STL, and
there can only be one auto\_ptr per pointer, but it still seems useful.
Combined with the object pooling, it seems like it will automatically return
objects to the pool. This page gives some do's and don't of auto\_ptrs.

Memory leak detection:
http://www.flipcode.com/tutorials/tut_memleak.shtml
This page describes overridden versions of new and delete. It should be
combinable with the object pool versions of new and delete.

http://www.cfxweb.net/modules.php?name=News&file=article&sid=1920
This page also contains info on auto\_ptr's, but also on a different kind of
smart pointer that does reference counting and also different versions of new
and delete that keeps track of memory allocations and deallocations.

------- Comment #5 From Bob Marinier 2008-04-14 20:55:41 (-) [reply](reply.md) -------

We might consider running each agent in a multiagent system in its own thread.
This would be a major parallelization win for multiagent systems. Also, there
was some discussion a while ago about making the kernel single agent. This is
probably a good idea, to keep things simple, but we'd still need an extra layer
that can generate events like AFTER\_ALL\_GENERATED\_OUTPUT (maybe an "agent" will
be what we call a kernel now, and a "kernel" will be a group of "agents").


---


# Thoughts on parallelizing Soar #

Currently, while SML uses some threads to handle message passing, Soar itself
is single threaded. This means it won't scale well on multicore/multiprocessor
systems, which are becoming increasingly common.

There are many possible ways to introduce parallelism to Soar. Earlier work
has looked at parallelizing the rete itself, for example. This kind of very
low level parallelism is probably more appropriate for a complete rewrite of
Soar. Another possibility is for some of the new modules we're working on to
be designed to run in parallel with Soar or internally. This might apply
particularly well to episodic and semantic memory. At a higher level, we can
simply run multiple agents in parallel (e.g., the scheduler would run each
agent simultaneously in its own thread). I suspect that the agent level would
be the easiest place to start (although it will only benefit multiagent
systems).

At the agent level, the primary issue is that there are 20-30 global variables
(most data is agent-specific, and thus not subject to conflicts). Many of these
globals are read-only after initialization; for the rest, we can use locks to
avoid conflicts, or move them into the agent structure.

Another possible issue is callbacks. If multiple agents are calling the same
callback at the same time, and those callbacks reference global/static data,
that will likely cause problems. I don't know if anything in SML falls into
this category, but it at least implies restrictions on client code. It would
be interesting to hear from anyone who thinks this might be a problem for them.
We could serialize callbacks, but that's likely to negate much of the benefit
we get from threading in the first place (but maybe we can make it an option).

Anyway, I'm hardly a threading expert, so please share any thoughts or gotchas
that I'm oblivious to.

------- Comment #1 From Dave Ray 2008-05-29 11:53:06 (-) [reply](reply.md) -------

Another possible approach that I think would be a little less work than running
agents in separate threads would be to remove the one-kernel-per-process
restriction that currently exists in SML. Then you could create as many
kernels as you like, hopefully running on different cores, while still allowing
multiple agents to run on a single core.

I think the main hurdle is global variables in both SML (I think the lock is
global) and the globals you mention in the kernel. I think I would trust an
approach that eliminates the globals more than one that tries to protect them
with locks.

Also, since callbacks are specific to a kernel you wouldn't have to worry about
the synchronization problems you'd get with callbacks being invoked by
different agents from different threads.

Not sure how this impacts remote debugging. An initial, naive approach would be
to just have each kernel accessible on a separate port.

------- Comment #2 From Bob Marinier 2008-05-29 11:58:39 (-) [reply](reply.md) -------

This actually brings up related design issue that comes up from time to time --
namely that each kernel can have multiple agents. Certain aspects of the code
might be simplified if we enforced one agent per kernel. But it may be that
this just pushes the complexity of dealing with multiple agents into a
different layer (e.g., into SML or client code).

------- Comment #3 From Dave Ray 2008-05-29 12:04:42 (-) [reply](reply.md) -------

Well, if you have one agent per kernel, then you don't really have a kernel
anymore do you :) Is there even really a kernel now, or is it just a list of
agents and some globals for rete tokens and stuff.

I'm not necessarily opposed to this idea, but allowing multiple kernels in a
process seems like a smaller conceptual (and API) change with a clearer payoff.

------- Comment #4 From Bob Marinier 2008-05-29 12:41:25 (-) [reply](reply.md) -------

I realize that I wasn't clear before. When we talk about one agent per kernel,
we mean at the Soar level. And as you say, there already isn't really a kernel
at that level. It looks like the gSKI guys added a kernel struct, but all it
did was keep a list of agents and (two!) agent counts. Since no one actually
used the kernel struct, we just removed it. So completing the transition to
"no kernel" at the Soar level probably just means eliminating globals and
whatever else might need to happen such that agents can be used without
conflicts.

At the SML level, a kernel object is definitely useful. We often want to
register for events that involve multiple agents (e.g.,
after\_all\_generated\_output) or that are about agents (e.g.,
after\_agent\_destroyed). But since the scheduler lives in SML now, I don't
think that changes we make at the Soar level will require much change at the
SML level (perhaps none at all).

So anyway, I think you're right in that we're essentially single agent now.

------- Comment #5 From Douglas Pearson 2008-05-29 16:43:37 (-) [reply](reply.md) -------

I think if we try to parallelize at the agent level (or the kernel level) it
shouldn't be too bad. I don't think we'll need to serialize the callbacks.
It's just up to the environment/client to make sure their code is thread-safe
-- so you don't have two threads modifying the same object within the
environment.

(BTW, if we ever decide we want multiple KernelSML objects for some
reason--like Bob I'm not sure we would--the only global I'm aware of is the
KernelSML singleton itself. If you pulled that out and passed it around
instead you could have multiple kernels).