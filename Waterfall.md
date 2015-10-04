# As described by Bob Marinier #

"Suppose I have a blocks world agent that is trying to accomplish "put A on B". Several moves might be required to do this, and the agent doesn't know what they are, so it goes into a subgoal and starts randomly moving blocks around. What we want is for the agent to get a positive reward on the substate's reward link when it succeeds. So we can have a rule that detects that A is on B and creates a reward when that happens. However, when A is on B, the subgoal's supporting operator proposal will retract. Even though this retraction could, in principle, occur in parallel with the reward rule firing, the, waterfall will cause the retraction to occur first, and thus the substate will go away before the reward rule gets to fire, so the agent won't get the reward. In our proposed modification, the reward rule and retraction would occur in parallel, and thus the agent would get the reward."

# Brief description in manual #

Note: it is not referred to by any name (or waterfall). See manual 2.6.5 (last paragraph):

The second change when there are multiple substates is that at each phase, Soar goes through
the substates, from oldest (highest) to newest (lowest), completing any necessary processing
at that level for that phase before doing any processing in the next substate. When firing
productions for the proposal or application phases, Soar processes the firing (and retraction)
of rules, starting from those matching the oldest substate to the newest. Whenever a
production fires or retracts, changes are made to working memory and preference memory,
possibly changing which productions will match at the lower levels (productions firing within
a given level are fired in parallel – simulated). Productions firings at higher levels can resolve
impasses and thus eliminate lower states before the productions at the lower level ever fire.
Thus, whenever a level in the state stack is reached, all production activity is guaranteed to
be consistent with any processing that has occurred at higher levels.

# Definitions #

Minor quiescence: no more i-assertions (or any retractions) ready to fire in the current goal

Consistency check: making sure that the currently selected operator is still legal (e.g., it's still acceptable, it shouldn't be replaced by a better operator or an impasse)

# Available compile flags (in consistency.h) #

```
/* For information on the consistency check routines */
/* #define DEBUG_CONSISTENCY_CHECK */

/* For information on aspects of determining the active level */
/* #define DEBUG_DETERMINE_LEVEL_PHASE */
```

# Available kernel functions #

`highest_active_goal_propose`: Find the highest goal of activity among the current i-assertions and retractions

`highest_active_goal_apply`: Find the highest goal of activity among the current i-assertions, o-assertions and retractions

`active_production_type_at_goal`: Returns IE\_PRODS if i-assertions active, otherwise PE\_PRODS

`initialize_consistency_calculations_for_new_decision`: call before functions below?

`determine_highest_active_production_level_in_stack_apply`: implements waterfall for apply phase (DETERMINE\_LEVEL\_PHASE)

  * calls itself recursively
  * called in do\_one\_top\_level\_phase (APPLY\_PHASE, twice)
  * if the next active goal is lower in the stack than the previous one, but the stack is no longer consistent up to the previous goal, then proceed to output

`determine_highest_active_production_level_in_stack_propose`: implements waterfall for propose phase

  * called in do\_one\_top\_level\_phase (PROPOSE\_PHASE, twice)

`get_next_assertion` (rete.cpp): gets next production/token/wme associated with the current goal (as determined by above)

`do_working_memory_phase`: "commits" the changes at the end of a phase

# Implementation thoughts #

In do\_one\_top\_level\_phase, currently do this:

  1. determine highest active goal
  1. fire rules at that goal
  1. commit changes
  1. proceed to next phase

Could change it to do this:

  1. determine highest active goal
  1. fire rules at that goal, tracking the highest goal with a change
  1. determine highest active goal below highest changed goal
  1. goto 2 until past bottom goal
  1. commit changes
  1. proceed to next phase

# Test Case #

```
# Test case for revised waterfall model
#
# In Soar8/9.0.0 waterfall model, the change*substate rule will never fire. This is because the 
# change*top-state rule will fire first, which will cause the proposal to unmatch and thus the
# substate will retract
#
# In the revised waterfall model, change*substate will fire in parallel with change*top-state, since
# the effects cannot possibly conflict.

learn --off

sp {propose*test
(state  ^superstate nil
          -^result true)
-->
( ^operator  +)
( ^name test)
}

sp {change*top-state
(state  ^superstate )
( ^operator.name test)
-->
( ^result true)
(write (crlf) |Changed top-state|)
}

sp {change*substate
(state  ^superstate )
( ^operator.name test
      ^result true)
-->
( ^substate changed)
(write (crlf) |Changed substate|)
}
```

# Test case in jSoar #

The file `/jsoar/test/org/jsoar/kernel/FunctionalTests_testWaterJugLookAhead.soar` in the jSoar project contains the above code plus a (succeeded) rhs call that works with JUnit so that the test can succeed once the changes to the waterfall model work.

# Notes #

  * preference phase: the inner loop that processes assertions and retractions at the active level and possibly below with new waterfall model
  * matches: assertion/retraction, matches '''AND unmatches''' coming from the rete
  * active\_level: the highest level at which matches are waiting to be processed
  * previous\_active\_level: the active\_level at the start of the previous outer preference loop
  * change\_level: lowest level affected by matches fired during previous iteration of inner preference loop, always equal to or higher than active\_level, matches firing in next iteration cannot change this level or higher.
  * next\_change\_level: lowest level affected by matches fired during this iteration of inner preference loop, becomes change\_level for next iteration
  * high\_match\_change\_level: highest level affected by a match's changes, compares to change\_level
  * low\_match\_change\_level: lowest level affected by a match's changes, sets next\_change\_level

algorithm

This describes one outer preference loop.

  1. Reset '''active\_level'''=0, '''next\_change\_level'''=0.
  1. Set '''active\_level'''.
  1. Set '''previous\_active\_level''' = '''active\_level'''
  1. Inner preference loop start:
    1. If '''active\_level''' is invalid, break out of loop.
    1. Set '''change\_level''' = '''next\_change\_level'''.
    1. For each match at the '''active\_level''':
      1. Determine '''high\_match\_change\_level''' and '''low\_match\_change\_level''' (see execute\_action).
      1. If the '''high\_match\_change\_level''' < '''change\_level''':
        1. Fire the match (and be sure match is removed from match lists).
        1. Set '''next\_change\_level''' = min('''next\_change\_level''', '''low\_match\_change\_level''')
      1. Else if '''high\_match\_change\_level''' >= '''change\_level''':
        1. Do not fire the match (and be sure match is retained in match lists).
    1. Set '''active\_level''' to next lowest level that has activity (matches) below the current '''active\_level'''.
    1. Go to inner preference loop start.
  1. Set '''active\_level''' = '''previous\_active\_level'''
  1. Commit changes (do\_working\_memory\_phase)

implementation notes

  * done in initialize\_consistency\_calculations\_for\_new\_decision
  * done in determine\_highest\_active\_production\_level\_in\_stack\_apply/propose
  * local variable in doApplyPhase, not yet implemented for propose
  * current handling of this (in the apply phase) is to set current\_phase to Phase.OUTPUT. May need to actually check this though because we probably won't be changing current\_phase in 4.4.

find out what level a match will change

possibly out of date

  1. Call create\_instantiation on the assertion
  1. Determine the highest level of any action (as reported by execute\_action) (see solution above)
  1. If any action's level is higher than the safe active level
    1. Don't create the instantiation (see below)
    1. Put the assertion back on the assertions list where we got it from (the pointers should still be in the right places)
  1. For each retraction at this goal, do something similar to above
notes on only firing the assertion:

create\_instantiation is going to go through all of the actions, if we
find midway through that an action is bad, we throw out the entire
instantiation. This means execute\_action needs to return this sort of
failure code so that the effects of get\_next\_assertion on the rete
listener can be undone, which is something like pushing the assertion
back on to the list for the active goal. There are multiple lists of
assertions for each goal, we need to put it back on the correct one.

Alternative way: get the assertion without popping it, when it is
confirmed legitimate, then pop it. This requires some kind of state.

Which is easier? Find out how many people call get\_next\_assertion. If
not called much, might be easier to do the peek before popping.

Possible changes

Stuff we were fooling with that I didn't want to check in.

change highest\_active\_goal\_propose() (and _apply()) to take a starting goal instead of starting at top\_goal. Current functions that call it just pass top\_goal. We could use this to get the highest active goal at or beneath the passed goal._

create root\_active\_goal and root\_active\_level to save the old active\_goal/level because it needs to be restored after the parallel waterfall finishes. It needs to be saved because the consistency stuff checks to see if it went up/down from the previous active goal. "root" may be the wrong name prefix for it, consider changing.

DecisionCycle.java:doApplyPhase():439 is where we were going to insert the changes.
Levels on action wmes are known in RecognitionMemory.java:execute\_action():359 after the id is not null. id.level seems to be correct, if conservative (the actual level may be higher than what is reported here).