# pbreak #

Toggles the :interrupt flag on a rule at run-time, which stops the Soar decision cycle when the rule fires.

## Synopsis ##

```
rl -c|--clear <production-name>
rl -p|--print
rl -s|--set <production-name>
rl <production-name>
```

## Options ##

| **Parameter** | **Argument** | **Description** |
|:--------------|:-------------|:----------------|
| `-c, --clear` | 

&lt;production-name&gt;

 | Clear :interrupt flag from a production. |
| `-p, --print` | (none)       | Print which production rules have had their :interrupt flags set. |
|  (none)       | (none)       | Print which production rules have had their :interrupt flags set. |
| `-s, --set`   | 

&lt;production-name&gt;

 | Set :interrupt flag on a production rule. |
| (none)        | 

&lt;production-name&gt;

 | Set flag :interrupt on a production rule. |

## Description ##

The `pbreak` command can be used to toggle the :interrupt flag on production rules which did not have it set in the original source file, which stops the Soar decision cycle when the rule fires. This is intended to be used for debugging purposes.

## See Also ##

[matches](cmd_matches.md) [pwatch](cmd_pwatch.md) [watch](cmd_watch.md)