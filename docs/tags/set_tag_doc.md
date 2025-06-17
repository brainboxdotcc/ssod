# `<set>` Tag

The `<set>` tag is used to define persistent game state flags for the player. These flags are remembered per paragraph and are primarily used to track progress, choices, or major quest milestones that should not be repeatable.

## Purpose

- Store that a player has made a choice or triggered an event.
- Prevent content from repeating by checking with `<if>` tags.
- Can also be used to trigger achievements.

## Syntax

```
<set myflag>
```

This sets a flag called `myflag`. You can later check for it using:

```
<if flag myflag>
    (content shown only if flag is set)
<endif>
```

## Special Cases

Some flags trigger secondary logic internally:

- If the flag is `steam_copter`, it also sets a general-purpose `steamcopter` flag.
- All flags of this type can trigger a `"STATE"` achievement if defined.

## Example

```
You pull the lever.
<set vault_opened>
```

Later in another paragraph:

```
<if flag vault_opened>
The vault door is already open.
<else>
The vault door is shut tight.
<endif>
```

## Notes

- Flags are case-insensitive and stored lowercase.
- Flags are not automatically reset unless the player dies
- You cannot unset a flag with `<set>`. Use `<unset>` if you need to remove it.

