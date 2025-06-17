# `<mod>` Tag

The `<mod>` tag allows a paragraph to apply a stat change to the player. This can represent things like healing, skill boosts, receiving gold, or losing stamina. The effect only happens once — unless the `repeatable` option is used.

## Syntax

```text
<mod [repeatable] stat value>
```

- `repeatable` *(optional)* — Allows the effect to be applied more than once.
- `stat` — The player stat to modify. See list below.
- `value` — The amount to change the stat by (can be negative).

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

## Behaviour

- The change **won’t apply again** if the player has already visited this paragraph — unless you add `repeatable`.
- Negative values reduce the stat.
- Most stat changes are marked as unsafe (the location will not be treated as a rest spot).
- If the player levels up as a result of the change, a level-up notification is triggered.

## Example

```text
<mod stm 5>   — Gives the player 5 stamina.
<mod exp 20>  — Gives the player 20 XP.
<mod repeatable gold -10> — Takes 10 gold every time they visit.
```

Use this tag to reward or penalise the player based on their journey.