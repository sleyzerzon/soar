# wma #

Control the behavior of working memory activation.

## Synopsis ##

```
wma
wma -g|--get <parameter>
wma -s|--set <parameter> <value>
wma -S|--stats [<statistic>]
wma -t|--timers [<timer>]
wma -h|--history <timetag>
```

## Options ##

| `-g, --get` | Print current parameter setting |
|:------------|:--------------------------------|
| `-s, --set` | Set parameter value             |
| `-S, --stats` | Print statistic summary or specific statistic |
| `-t, --timers` | Print timer summary or specific timer |
| `-h, --history` | Print reference history of a WME |

## Description ##

The `wma` command changes the behavior of and displays information about working memory activation.

To get the activation of individual WMEs, use `print -i`.
To get the reference history of an individual WME, use `wma -h|--history `<`timetag`>. For example:

```
print --internal s1
(4000016: S1 ^ct 1000000 [3.6])
(4: S1 ^epmem E1 [1])
(11: S1 ^io I1 [1])
(20: S1 ^max 1000000 [3.4])
(18: S1 ^name ct [3.4])
(4000018: S1 ^operator O1000001 [1] +)
(4000019: S1 ^operator O1000001 [1])
(3: S1 ^reward-link R1 [1])
(8: S1 ^smem S2 [1])
(2: S1 ^superstate nil [1])
(14: S1 ^top-state S1 [1])
(1: S1 ^type state [1])
```

The bracketed values are activation. To get the history of an individual element:

```
wma --history 18
history (60/5999999, first @ d1):
 6 @ d1000000 (-1)
 6 @ d999999 (-2)
 6 @ d999998 (-3)
 6 @ d999997 (-4)
 6 @ d999996 (-5)
 6 @ d999995 (-6)
 6 @ d999994 (-7)
 6 @ d999993 (-8)
 6 @ d999992 (-9)
 6 @ d999991 (-10)

considering WME for decay @ d1019615
```

This shows the last 60 references (of 5999999 in total, where the first occurred at decision cycle 1). For each reference, it says how many references occurred in the cycle (such as 6 at decision 1000000, which was one cycle ago at the time of executing this command). Note that references during the current cycle will not be reflected in this command (or computed activation value) until the end of output phase. If `forgetting` is `on`, this command will also display the cycle during which the WME will be considered for decay. Even if the WME is not referenced until then, this is not necessarily the cycle at which the WME will be forgotten. However, it is guaranteed that the WME will not be forgotten before this cycle.

### Parameters ###

The `wma` command uses the `--get|--set <parameter> <value>` convention rather than individual switches for each parameter.
Running `wma` without any switches displays a summary of the parameter settings.

| **Parameter** | **Description** | **Possible values** | **Default** |
|:--------------|:----------------|:--------------------|:------------|
| `activation`  | Enable working memory activation | `on`, `off`         | `off`       |
| `decay-rate`  | WME decay factor | `[`0, 1`]`          | 0.5         |
| `decay-thresh` | Forgetting threshold | `(`0, inf`)`        | 2.0         |
| `forgetting`  | Enable removal of WMEs with low activation values | `on`, `off`         | `off`       |
| `forget-wme`  | If `lti` only remove WMEs with a long-term id | `all`, `lti`        | `all`       |
| `max-pow-cache` | Maximum size, in MB, for the internal `pow` cache | 1, 2, ...           | 10          |
| `petrov-approx` | Enables the (Petrov 2006) long-tail approximation | `on`, `off`         | `off`       |
| `timers`      | Timer granularity | `off`, `one`        | `off`       |

The `decay-rate` and `decay-thresh` parameters are entered as positive decimals, but are internally converted to, and printed out as, negative.

The `petrov-approx` may provide additional validity to the activation value, but comes at a significant computational cost, as the model includes unbounded positive exponential computations, which cannot be reasonably cached.

When `activation` is enabled, the system produces a cache of results of calls to the `pow` function, as these can be expensive during runtime. The size of the cache is based upon three run-time parameters (`decay-rate`, `decay-thresh`, and `max-pow-cache`), and one compile time parameter, `WMA_REFERENCES_PER_DECISION` (default value of 50), which estimates the maximum number of times a WME will be referenced during a decision. The cache is composed of `double` variables (i.e. 64-bits, currently) and the number of cache items is computed as follows:

e^((decay\_thresh - ln(max\_refs)) / decay\_rate)

With the current default parameter values, this will incur about 1.04MB of memory. Holding the `decay-rate` constant, reasonable changes to `decay-thresh` (i.e. +/- 5) does not greatly change this value. However, small changes to `decay-rate` will dramatically change this profile. For instance, keeping everything else constant, a `decay-thresh` of 0.3 requires ~2.7GB and 0.2 requires ~50TB. Thus, the `max-pow-cache` parameter serves to allow you to control the space vs. time tradeoff by capping the maximum amount of memory used by this cache. If `max-pow-cache` is much smaller than the result of the equation above, you may experience somewhat degraded performance due to relatively frequent system calls to `pow`.

If `forget-wme` is `lti` and `forgetting` is `on`, only those WMEs whose id is a long-term identifier **at the decision of forgetting** will be removed from working memory. If, for instance, the id is stored to semantic memory after the decision of forgetting, the WME will not be removed till some time after the next WME reference (such as testing/creation by a rule).

### Statistics ###

Working memory activation tracks statistics over the lifetime of the agent.
These can be accessed using `wma --stats <statistic>`.  Running
`wma --stats` without a statistic will list the values of all
statistics.  Unlike timers, statistics will always be updated.
Available statistics are:

| **Name** | **Label** | **Description** |
|:---------|:----------|:----------------|
| `forgotten-wmes` | Forgotten WMEs | Number of WMEs removed from working memory due to forgetting |

### Timers ###

Working memory activation also has a set of internal timers that record the
durations of certain operations.  Because fine-grained timing can
incur runtime costs, working memory activation timers are off by default.
Timers of different levels of detail can be turned on by issuing
`wma --set timers <level>`,
where the levels can be `off` or `one`, `one` being
most detailed and resulting in all timers being turned on.  Note that
none of the working memory activation statistics nor timing information is
reported by the `stats` command.

All timer values are reported in seconds.

Level one

| `wma_forgetting` | Time to process forgetting operations each cycle |
|:-----------------|:-------------------------------------------------|
| `wma_history`    | Time to consolidate reference histories each cycle |

## See Also ##

[print](cmd_print.md)