# popd #

Pop the current working directory off the stack and change to the next
directory on the stack. Can be relative pathname or fully specified path.

## Synopsis ##

```
popd
```

## Description ##

This command pops a directory off of the directory stack and
[changes to it](cmd_cd.md). See the [cmd\_dirs
dirs] command for an explanation of the directory stack.

## See Also ##

[cd](cmd_cd.md) [dirs](cmd_dirs.md)
[ls](cmd_ls.md)
[pushd](cmd_pushd.md) [source](cmd_source.md)
[pwd](cmd_pwd.md)
