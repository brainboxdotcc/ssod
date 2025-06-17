# `<expire>` Tag

The `<expire>` tag is used to manually remove a time-based flag (or "timed flag") from a player's persistent game state. This is part of the timed flags system in *Seven Spells of Destruction*, which enables temporary conditions or effects to be applied and later expire either naturally or through gameplay mechanics.

## Syntax

```html
<expire flag_name>
```

- `flag_name`: The identifier for the timed flag to be removed. This is case-insensitive and should not include the trailing `>` when typed in markup.

### Example

```html
<expire cursed_touch>
```

## Behaviour

When this tag is processed:

- It deletes the specified timed flag from the `timed_flags` database table for the current player.
- Timed flags are generally set by tags like `<effect>` or other scripted elements and represent temporary conditions.
- This tag provides a manual way to ensure a timed effect is cleared (e.g., if removed early due to an in-game action or achievement).

## Use Cases

- Clearing status effects (e.g., "diseased", "frozen", "empowered") when certain conditions are met.
- Managing narrative flags that control dialogue, availability of events, or player choices.
- Triggering expiration logic without waiting for server-side cooldown timers.

## Related Tags

- [`effect`](./effect_tag_documentation.md): Adds a passive effect with duration and cooldown, often setting a corresponding timed flag.
- [`mod`](./mod_tag_documentation.md): Modifies character stats directly.
- Custom scripting logic (server-side) may also interact with these timed flags.