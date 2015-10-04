# dirs #

List the directory stack.

## Synopsis ##

```
dirs
```

## Description ##

This command lists the directory stack. Agents can move through a directory
structure by pushing and popping directory names. The dirs command returns the
stack.

The command [pushd](cmd_pushd.md) places a new "agent current
directory" on top of the directory stack and changes to it. The command
[popd](cmd_popd.md) removes the directory at the top of the
directory stack and changes to the previous directory which now appears at the
top of the stack.

## See Also ##

[cd](cmd_cd.md)
[ls](cmd_ls.md) [pushd](cmd_pushd.md)
[popd](cmd_popd.md) [source](cmd_source.md)
[pwd](cmd_pwd.md)
