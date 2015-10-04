# smem #

Control the behavior of semantic memory.

## Synopsis ##

```
smem
smem -e|--enable|--on
smem -d|--disable|--off
smem -i|--init
smem -g|--get <parameter>
smem -s|--set <parameter> <value>
smem -S|--stats [<statistic>]
smem -t|--timers [<timer>]
smem -a|--add <concept>
smem -p|--print [<lti>] [<depth>]
smem -v|--viz [<lti>] [<depth>]
smem -b|--backup <file name>
```

## Options ##

| **Option** | **Description** |
|:-----------|:----------------|
| `-e, --enable, --on` | Enable semantic memory. |
| `-d, --disable, --off` | Disable semantic memory. |
| `-g, --get` | Print current parameter setting |
| `-s, --set` | Set parameter value |
| `-i, --init` | Reinitialize ALL memories |
| `-S, --stats` | Print statistic summary or specific statistic |
| `-t, --timers` | Print timer summary or specific statistic |
| `-a, --add` | Add concepts to semantic memory |
| `-r, --remove`| Remove concepts from semantic memory|
| `-p, --print` | Print semantic store in user-readable format |
| `-q, --query`| Print concepts in semantic store matching some cue|
| `-h, --history`| Print activation history for some LTI|
| `-v, --viz` | Print semantic store in graphviz format |
| `-b, --backup` | Creates a backup of the semantic database on disk |

## Description ##

The `smem` command changes the behavior of and displays information
about semantic memory.  The command `watch --smem` displays additional
trace information for semantic memory not controlled by this command.

### Parameters ###

Due to the large number of parameters, the `smem` command uses the
`--get|--set <parameter> <value>` convention rather than individual
switches for each parameter.  Running `smem` without any switches
displays a summary of the parameter settings.

#### Main Parameters ####
| **Parameter** | **Description** | **Possible values** | **Default** |
|:--------------|:----------------|:--------------------|:------------|
| `append`      | Controls whether database is overwritten or appended when opening or re-initializing | `on`, `off`         | `off`       |
| `database`    | Database storage method | `file`, `memory`    | `memory`    |
| `learning`    | Semantic memory enabled | `on`, `off`         | `off`       |
| `merge`       | Controls how retrievals interact with long-term identifiers in working memory | `none`, `add`       | `add`       |
| `mirroring`   | Controls automatic encoding of working-memory changes | `on`, `off`         | `off`       |
| `path`        |  Location of database file | _empty_, _some path_ | _empty_     |

The `learning` parameter turns the episodic memory module on or off.
When `learning` is set to `off`, no new episodes are encoded and no
commands put on the epmem link are processed.  This is the same as
using the enable and disable commands.

The `path` parameter specifies the file system path the database is stored
in.  When `path` is set to a valid file system path and database mode is set to
file, then the SQLite database is written to that path.

The append parameter will determine
whether all existing facts stored in a database on disk will be erased
when semantic memory loads. Note that this affects semantic memory re-initialization
also, i.e. if the append setting is off, all semantic facts
stored to disk will be lost when an init-soar is performed. For semantic memory,
append mode is by default on.

Note that changes to database, path and append will not have
an effect until the database is used after an initialization.  This
happens either shortly after launch (on first use) or after a database initialization
command is issued.  To switch databases or database storage types while
running, set your new parameters and then perform an `smem --init` command.

The `smem --backup` command can be used to make
a copy of the current state of the database, whether in memory or on
disk. This command will commit all outstanding changes before
initiating the copy.

For semantic memory to be reinitialized, all
references to long-term identifiers in all of Soar's memories must be
removed.  Consequently,
`smem --init`
will reinitialize episodic, semantic, procedural, and working
memories.  It is equivalent to wiping the semantic store and executing
these commands:

```
epmem --close
excise --all
init-soar
```

The `merge` parameter controls how the augmentations of retrieved long-term identifiers (LTIs) interact with an existing LTI in working memory. If the LTI is not in working memory or has no augmentations in working memory, this parameter has no effect. If the augmentation is in working memory and has augmentations, by default (`add`), semantic memory will add any augmentations that augmented the LTI in a retrieved LTI are added to working memory. If the parameter is set to `none` then semantic memory will not augment the LTI. Note this is opposite of the value of the same parameter in episodic memory.

The `mirroring` parameter controls a useful form of automatic encoding. If enabled (`on`), all changes to long-term identifiers (LTIs) in working memory are "mirrored" to semantic memory (assuming the LTI in working memory has at least one augmentation - i.e. no accidental clearing). The `mirrors` statistic is incremented for each LTI that is updated in this way.

#### Activation Parameters ####
| **Parameter** | **Description** | **Possible values** | **Default** |
|:--------------|:----------------|:--------------------|:------------|
| `activation-mode` | Sets the ordering bias for retrievals that match more than one memory | `recency`, `frequency`, `base-level` | `recency`   |
| `activate-on-query` | Determines if the results of queries should be activated | `on`, `off`         | `on`        |
| `base-decay`  | Sets the decay parameter for base-level activation computation | `>` 0               | 0.5         |
| `base-update-policy` | Sets the policy for re-computing base-level activation | `stable`, `naive`, `incremental` | `stable`    |
| `base-incremental-threshes` | Sets time deltas after which base-level activation is re-computed for old memories | 1, 2, 3, ...        | 10          |
| `thresh`      | Threshold for activation locality | 0, 1, ...           | 100         |

If `activation-mode` is `base-level`, three parameters control bias values. The `base-decay` parameter sets the free decay parameter in the base-level model. Note that we do implement the (Petrov, 2006) approximation, with a history size set as a compile-time parameter (default=10). The `base-update-policy` sets the frequency with which activation is recomputed. The default, `stable`, only recomputes activation when a memory is referenced (through storage or retrieval). The `naive` setting will update the entire candidate set of memories (defined as those that match the most constraining cue WME) during a retrieval, which has severe performance detriment and should be used for experimentation or those agents that require high-fidelity retrievals. The `incremental` policy updates a constant number of memories, those with last-access ages defined by the `base-incremental-threshes` set.

#### Performance Parameters ####
| **Parameter** | **Description** | **Possible values** | **Default** |
|:--------------|:----------------|:--------------------|:------------|
| `cache-size`  | Number of memory pages used in the SQLite cache | 1, 2, ...           | 10000       |
| `lazy-commit` | Delay writing semantic store changes to file until agent exits | `on`, `off`         | `on`        |
| `optimization` |  Policy for committing data to disk | `safety`, `performance` | `performance` |
| `page-size`   | Size of each memory page used in the SQLite cache | 1k, 2k, 4k, 8k, 16k, 32k, 64k | 8k          |
| `timers`      | Timer granularity | `off`, `one`, `two`, `three` | `off`       |

When the database is stored to disk, the
`lazy-commit` and `optimization` parameters control how
often cached database changes are written to disk.  These parameters
trade off safety in the case of a program crash with database
performance.  When `optimization` is set to `performance`, the agent
will have an exclusive lock on the database, meaning it cannot be
opened concurrently by another SQLite process such as SQLiteMan.  The
lock can be relinquished by setting the database to memory or another
database and issuing init-soar/`smem --init` or by shutting down
the Soar kernel.

### Statistics ###

Semantic memory tracks statistics over the lifetime of the agent.
These can be accessed using `smem --stats <statistic>`.  Running
`smem --stats` without a statistic will list the values of all
statistics.  Unlike timers, statistics will always be updated.
Available statistics are:

| **Name** | **Label** | **Description** |
|:---------|:----------|:----------------|
| `act_updates` | Activation Updates | Number of times memory activation has been calculated |
| `db-lib-version` | SQLite Version | SQLite library version |
| `edges`  | Edges     | Number of edges in the semantic store |
| `mem-usage` | Memory Usage | Current SQLite memory usage in bytes |
| `mem-high` | Memory Highwater | High SQLite memory usage watermark in bytes |
| `mirrors` | Mirrors   | Number of LTIs that have been automatically encoded via `mirroring` |
| `nodes`  | Nodes     | Number of nodes in the semantic store |
| `queries` | Queries   | Number of times the **query** command has been issued |
| `retrieves` | Retrieves | Number of times the **retrieve** command has been issued |
| `stores` | Stores    | Number of times the **store** command has been issued |

### Timers ###

Semantic memory also has a set of internal timers that record the
durations of certain operations.  Because fine-grained timing can
incur runtime costs, semantic memory timers are off by default.
Timers of different levels of detail can be turned on by issuing
`smem --set timers <level>`,
where the levels can be `off`, `one`, `two`, or `three`, `three` being
most detailed and resulting in all timers being turned on.  Note that
none of the semantic memory statistics nor timing information is
reported by the `stats` command.

All timer values are reported in seconds.

Level one

| **Timer** | **Description** |
|:----------|:----------------|
| `_total`  | Total smem operations |

Level two

| **Timer** | **Description** |
|:----------|:----------------|
| `smem_api` | Agent command validation |
| `smem_hash` | Hashing symbols |
| `smem_init` | Semantic store initialization |
| `smem_ncb_retrieval` | Adding concepts (and children) to working memory |
| `smem_query` | Cue-based queries |
| `smem_storage` | Concept storage |

Level three

| **Timer** | **Description** |
|:----------|:----------------|
| three\_activation | Recency information maintenance |

### Manual Storage ###

Concepts can be manually added to the semantic store using the
`smem --add <concept>`
command.  The format for specifying the concept is similar to that of
adding WMEs to working memory on the RHS of productions.  For example:

```
smem --add {
   (<arithmetic> ^add10-facts <a01> <a02> <a03>)
   (<a01> ^digit1 1 ^digit-10 11)
   (<a02> ^digit1 2 ^digit-10 12)
   (<a03> ^digit1 3 ^digit-10 13)
}
```

Although not shown here, the common "dot-notation" format used in
writing productions can also be used for this command.  Unlike agent
storage, manual storage is automatically recursive.  Thus, the above
example will add a new concept (represented by the temporary
"arithmetic" variable) with three children.  Each child will be its
own concept with two constant attribute/value pairs.

### Manual Removal ###

Part or all of the information in the semantic store of some LTI can be manually removed from the semantic store using the

```
smem --remove <concept>
```

command.  The format for specifying what to remove is similar to that of
adding WMEs to working memory on the RHS of productions.  For example:
```
smem --remove {
   (@W34 ^good-attribute |gibberish value|)
}
```
If `good-attribute` is multi-valued, then all values will remain in the store except `|gibberish value|`.  If `|gibberish value|` is the only value, then `good-attribute` will also be removed.  It is not possible to use the common "dot-notation" for this command.  Manual removal is not recursive.

Another example highlights the ability to remove all of the values for an attribute:
```
smem --remove {
   (@W34 ^bad-attribute)
}
```
When a value is not given, all of the values for the given attribute are removed from the LTI in the semantic store.

Also, it is possible to remove all augmentations of some LTI from the semantic store:
```
smem --remove {
   (@W34)
}
```
This would remove all attributes and values of `@W34` from the semantic store. The LTI will remain in the store, but will lack augmentations.

(Use the following at your own risk.)  Optionally, the user can force removal even in the event of an error:
```
smem -r {(@W34 ^bad-attribute ^bad-attribute-2)} force
```
Suppose that LTI `@W34` did not contain `bad-attribute`.  The above example would remove `bad-attribute-2` even though it would indicate an error (having not found `bad-attribute`).

### Manual Query ###

Queries for LTIs in the semantic store that match some cue can be initialized external to an agent using the
`smem --query <cue> [<num>]`
command.  The format for specifying the cue is similar to that of adding a new identifier to working memory in the RHS of a rule:
```
smem --query {
    (<cue> ^attribute <wildcard> ^attribute-2 |constant|)
}
```
Note that the root of the cue structure must be a variable and should be unused in the rest of the cue structure.  This command is for testing and the full range of queries accessible to the agent are not yet available for the command.  For example, math queries are not supported.

The additional option of `<num>` will trigger the display of the top `<num>` most activated LTIs that matched the cue.

The result of a manual query is either to print that no LTIs could be found or to print the information associated with LTIs that were found in the `smem --print <lti>` format.

### History ###

When the activation-mode of a semantic store is set to base-level, some history of activation events is stored for each LTI.  This history of when some LTI was activated can be displayed:
```
        smem --history @W34
```
In the event that semantic memory is not using base-level activation, `history` will mimic `print`.

### Visualization ###

When debugging agents using semantic memory it is often useful to
inspect the contents of the semantic store.  Running
`smem --viz [<ltid>] [<depth>]`
will output the concept rooted at `<ltid>` to depth `<depth>` in
graphviz format, including long-term identifier activation levels.  If `<ltid>` is omitted, the entire contents of the semantic store are outputted.  For more information on
this format and visualization tools, see http://www.graphviz.org. The `smem --print` option has the same syntax, but outputs text that is similar to using the `print` command to get the substructure of an identifier in working memory, which is possibly more useful for interactive debugging.

## See Also ##

[watch](cmd_watch.md)