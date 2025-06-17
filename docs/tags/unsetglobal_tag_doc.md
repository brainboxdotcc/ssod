# `<unsetglobal>` — Remove Global Flag

The `<unsetglobal>` tag is used to **remove a globally set game flag** that affects **all players**. This tag should be used **with extreme caution**, as global flags represent world-level state — not personal state. Once removed, the global state is gone for everyone, not just the current player.

## Syntax:
```
<unsetglobal FLAGNAME>
```

- `FLAGNAME` should be a lowercase-compatible identifier (underscores allowed).
- There should be **no trailing `>`** in the flag name inside the tag; it will be stripped automatically by the engine.

## Behaviour:
- Deletes the flag from the `game_global_flags` database table.
- There is **no visual feedback or confirmation** by default when the flag is removed.
- The change is **global** — it affects all current and future players.

## Example:
```
<unsetglobal DARKNESSCURSE>
```

This removes the global flag `darknesscurse`, potentially undoing a previous global event — such as ending an extended eclipse or curse affecting all towns.

## Warnings and Best Practices:
- Do **not** use `<unsetglobal>` unless you're **absolutely certain** undoing a global state is appropriate.
- Avoid placing this tag in publicly accessible or general-use locations.
- Most global flags set with `<setglobal>` are intended to be **permanent narrative/world state changes**. Removing them may cause narrative inconsistencies or affect logic across multiple quests or players.

Use with **extreme discretion**.