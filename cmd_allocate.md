# allocate #

Allocate additional 32 kilobyte blocks of memory for a specified memory pool without running Soar.

## Synopsis ##

```
allocate [pool blocks]
```

## Description ##

Soar allocates blocks of memory for its memory pools as it is needed during a
run (or during other actions like loading productions). Unfortunately, this
behavior translates to an increased run time for the first run of a
memory-intensive agent. To mitigate this, blocks can be allocated before a run
by using this command.

Issuing the command with no parameters lists current pool usage, exactly like
[stats](cmd_stats.md) command's memory flag.

Issuing the command with part of a pool's name and a positive integer will
allocate that many additional blocks for the specified pool. Only the first few
letters of the pool's name are necessary. If more than one pool starts with the
given letters, which pool will be chosen is unspecified.

Memory pool block size in this context is approximately 32 kilobytes, the exact
size determined during agent initialization.

## See Also ##

[stats](cmd_stats.md)
