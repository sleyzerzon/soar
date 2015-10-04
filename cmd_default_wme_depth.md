# default-wme-depth #

Set the level of detail used to print WMEs.

## Synopsis ##

```
default-wme-depth [depth]
```

### Default Aliases ###

| `set-default-depth` | `default-wme-depth` |
|:--------------------|:--------------------|

## Options ##

| `depth` | A non-negative integer. |
|:--------|:------------------------|

## Description ##

The default-wme-depth command reflects the default depth used when working
memory elements are printed (using the [print](cmd_print.md)). The
default value is 1. When the command is issued with no arguments,
default-wme-depth returns the current value of the default depth. When followed
by an integer value, default-wme-depth sets the default depth to the specified
value. This default depth can be overridden on any particular call to the
[print](cmd_print.md) command by explicitly using the `--depth`
flag, e.g. `print --depth 10 args`.

By default, the [print](cmd_print.md) command prints _objects_ in
working memory, not just the individual working memory element. To limit the
output to individual working memory elements, the `--internal` flag must also
be specified in the [print](cmd_print.md) command. Thus when the
print depth is `0`, by default Soar prints the entire object, which is the same
behavior as when the print depth is `1`. But if `--internal` is also specified,
then a depth of `0` prints just the individual WME, while a depth of `1` prints
all WMEs which share that same identifier. This is true when printing timetags,
identifiers or WME patterns.

When the depth is greater than `1`, the identifier links from the specified
WME's will be followed, so that additional substructure is printed. For
example, a depth of `2` means that the object specified by the identifier,
wme-pattern, or timetag will be printed, along with all other objects whose
identifiers appear as values of the first object. This may result in multiple
copies of the same object being printed out. If `--internal` is also specified,
then individuals WMEs and their timetags will be printed instead of the full
objects.

## See Also ##

[print](cmd_print.md)
