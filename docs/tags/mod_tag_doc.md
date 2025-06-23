
# `<mod>` Tag

The `<mod>` tag allows a paragraph to apply a stat change to the player. This can represent things like healing, skill boosts, receiving gold, or losing stamina. The effect only happens once — unless the `repeatable` option is used.

## Syntax

```text
<mod [repeatable] stat value>
<mod [repeatable] stat value flag flagname>
```

- `repeatable` *(optional)* — Allows the effect to be applied more than once.
- `stat` — The player stat to modify. See list below.
- `value` — The amount to change the stat by (can be negative).
- `flag flagname` *(optional)* — Ensures the effect is only applied once by setting and checking an internal MOD flag with the given `flagname`.

## Supported Stats

- `stm` – Stamina
- `skl` – Skill
- `luck` or `lck` – Luck
- `exp` – XP
- `arm` – Armour rating
- `wpn` – Weapon rating
- `spd` – Speed
- `mana` – Mana
- `rations` – Rations
- `notoriety` – Notoriety
- `gold` – Gold
- `silver` – Silver

Also:

- `reg0` to `reg31` – General-purpose registers for advanced scripting use

## Behaviour

- The change **won’t apply again** if the player has already visited this paragraph — unless you add `repeatable`.
- If you use the `flag` syntax, the effect will only occur once **per flag**, independent of repeatability.
- Negative values reduce the stat.
- Most stat changes are marked as unsafe (the location will not be treated as a rest spot).
- If the player levels up as a result of the change, a level-up notification is triggered.
- You may assign a stat directly from another stat: `<mod stm skl>` sets stamina equal to skill.
- You may assign a stat directly from a stored value using `flag flagname`: `<mod stm flag temp_stm>` will load a numeric value from saved flag `temp_stm`.

## Examples

```text
<mod stm 5>                     — Gives the player 5 stamina.
<mod exp 20>                    — Gives the player 20 XP.
<mod repeatable gold -10>      — Takes 10 gold every time they visit.
<mod exp 10 flag saw_intro>    — Adds 10 XP only if the player hasn’t seen this flag before.
<mod skl reg1>                 — Sets skill equal to register 1.
<mod stm flag old_stm>         — Sets stamina to value stored in flag 'old_stm' (if numeric).
```

Use this tag to reward or penalise the player based on their journey while preventing exploits or repeated triggering.
