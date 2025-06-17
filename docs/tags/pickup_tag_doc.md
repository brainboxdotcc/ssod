# `<pickup>` Tag

The `<pickup>` tag is used to automatically award the player an item, scroll, or currency upon visiting a location. Unlike `<pick>`, which presents the player with a *choice*, `<pickup>` is unconditional and automatic, but **not repeatable** — it tracks whether the player has already visited this paragraph.

## Basic Syntax

```
<pickup scroll>
<pickup gold 5>
<pickup silver 10>
<pickup sword [none]>
<pickup Fireball [SPELL]>
<pickup Healing Root [HERB]>
```

## Supported Pickups

- **scroll**: Awards 1 scroll and logs achievement. This ignores inventory limits.
- **gold**/**silver**: Awards the specified amount, once. Adds a flag so it can't be re-obtained.
- **item**: Standard possessions, herbs, or spells. Inventory checks apply.

### Flags
The flag (e.g. `[SPELL]`, `[HERB]`, `[none]`) determines what type of item is granted:
- `[SPELL]` adds to the player’s known spells list.
- `[HERB]` adds to the herbs collection.
- `[none]` or no flag is treated as a normal item.

## Behaviour

- **Not repeatable**: Once a pickup has been claimed, the player cannot claim it again unless their state resets (e.g. on death).
- **Inventory Limit**: Standard items cannot be picked up if the inventory is full, but scrolls, spells, and herbs bypass this restriction.
- **Scroll Recognition**: Grants a scroll automatically with achievements triggered.
- **Achievement Triggers**: For scrolls, gold, silver, and items, the corresponding collection achievements are checked and logged.
- **Safety Flag**: The presence of `<pickup>` marks a location as *unsafe*, so stamina is consumed when travelling there.

## Example

```
<pickup gold 12>
<pickup Bronze Ring [none]>
<pickup Cure Wounds [SPELL]>
<pickup Bitterroot [HERB]>
```

Each line ensures that the item is awarded when the player arrives at the paragraph, as long as they haven't already received it. You don't need to write logic to prevent multiple pickups — this is handled automatically.

---

*Use `<pickup>` when you want to reward the player for finding a location — without giving them a choice.*  
*Use `<pick>` when you want to give them a mutually exclusive selection between several items.*
