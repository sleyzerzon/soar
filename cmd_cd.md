# cd #

Change directory.

## Synopsis ##

```
cd [directory]
```

### Default Aliases ###

| `chdir` | `cd` |
|:--------|:-----|

## Options ##

| `directory` | The directory to change to, can be relative or full path. |
|:------------|:----------------------------------------------------------|

## Description ##

Change the current working directory. If run with no arguments, returns to the
directory that the command line interface was started in, often referred to as
the _home_ directory.

## Examples ##

To move to the relative directory named `../home/soar/agents`:

```
cd ../home/soar/agents
```

## See Also ##

[dirs](cmd_dirs.md) [ls](cmd_ls.md) [pushd](cmd_pushd.md) [popd](cmd_popd.md) [source](cmd_source.md) [pwd](cmd_pwd.md)