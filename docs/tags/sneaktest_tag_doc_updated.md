
# `<sneaktest>` Tag

The `<sneaktest>` tag is used to simulate a sneak-based skill test against an opponent or rival. This is especially useful for scenarios where stealth, pickpocketing, or evasion are involved.

## How It Works

- The player’s **Sneak** value plus a **1d6 dice roll** is compared against an opponent's **Sneak** value plus their **1d6 dice roll**.
- If the player’s result is **equal or higher**, the test **passes**.
- If it is **lower**, the test **fails**.

This is a PvP-style mechanic — the sneak comparison is not against a fixed number but assumes a roll for the opposing party too.

## Syntax

```
<sneaktest name="Guard" sneak=5>
```

- `name`: Displayed name of the opponent.
- `sneak`: Sneak skill level of the opponent.

## Example

```text
You try to sneak past the castle guard.
<sneaktest name="Castle Guard" sneak=4>
```

Will result in something like:

```
**Castle Guard** *Sneak 4*, **PASS!**
```
or
```
**Castle Guard** *Sneak 4*, **FAIL!**
```

## Behaviour

- The result is automatically shown to the player as **PASS** or **FAIL**.
- The test result is stored in `p.auto_test` which may be used by following `<if>` blocks to branch the narrative.
- The location is marked as **unsafe** due to the potential consequences of being detected.
- An achievement check is triggered with details of success and the opponent name.

## Notes

- This tag is best used for stealth-based encounters.
- It introduces a light random element due to the dice involved on both sides.
- Consider pairing with `<if>` logic to branch outcomes based on success or failure.


### Usage Note

The `<sneaktest>` tag should always be followed by **two `<AUTOLINK>` tags**:
- The **first `<AUTOLINK>`** will be shown **only if the sneak test succeeds**.
- The **second `<AUTOLINK>`** will be shown **only if the sneak test fails**.

Only **one** of these links will be enabled based on the test result. This enables clear branching based on the player's stealth success.
