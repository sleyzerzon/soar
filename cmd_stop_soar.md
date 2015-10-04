# stop-soar #

Pause Soar.

## Synopsis ##

```
stop-soar [-s] [reason string]
```

### Default Aliases ###

| `interrupt` | `stop-soar` |
|:------------|:------------|
| `ss`        | `stop-soar` |
| `stop`      | `stop-soar` |

## Options ##

| `-s, --self` | Stop only the soar agent where the command is issued. All other agents continue running as previously specified. |
|:-------------|:-----------------------------------------------------------------------------------------------------------------|
| `reason_string` | An optional string which will be printed when Soar is stopped, to indicate why it was stopped. If left blank, no message will be printed when Soar is stopped. |

## Description ##

The stop-soar command stops any running Soar agents. It sets a flag in the Soar
kernel so that Soar will stop running at a "safe" point and return control to
the user.  This command is usually not issued at the command line prompt - a
more common use of this command would be, for instance, as a side-effect of
pressing a button on a Graphical User Interface (GUI).

## See Also ##

[run](cmd_run.md)

## Warnings ##

If the graphical interface doesn't periodically do an "update" of flush the
pending I/O, then it may not be possible to interrupt a Soar agent from the
command line.
