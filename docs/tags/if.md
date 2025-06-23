# Conditional Tag Syntax: `<if ...>`

Displays or hides a block of content depending on player state. Conditions can be combined using `and`, `or`, and `not`, and grouped with parentheses. This system supports **recursive expressions**, **stat comparisons**, and **predicate-based checks**.

## Syntax Examples

```
<if EXP eq 5> ... <else> ... <endif>
<if luck gt dice> ... <endif>
<if flag sword_found> ... <endif>
<if !item "healing potion"> ... <endif>
<if (exp gt 10 and flag quest_started)> ... <endif>
<if not mounted> ... <endif>
<if flag has_sword eq "yes"> ... <endif>
<if flag dungeon_code eq "1234"> ... <endif>
```

---

## 🔹 Comparison Expressions

Use these to compare numeric game state values:

- **Syntax**:\
  `<if STAT OP VALUE>`\
  Where:
  - `STAT` is one of:
    - `stm` (Stamina)
    - `skl` (Skill)
    - `exp` (Experience)
    - `arm` (Armour rating)
    - `wpn` (Weapon rating)
    - `luck` (Luck)
    - `spd` (Speed)
    - `day` (Days passed)
    - `scrolls`
    - `level`
    - `mana`
    - `notoriety`
    - `gold`
    - `silver`
    - `rations`
  - `OP` is a comparison operator: `eq`, `ne`, `gt`, `lt`, `gte`, `lte`
  - `VALUE` is a number or the keyword `dice` (from a prior `<dice>` roll)

**Examples:**

```
<if luck gt 3> You are lucky. <endif>
<if mana lt dice> Your spell fizzles. <endif>
```

---

## 🔹 Flag Checks

Check for flags previously set via `<set>`, `<setglobal>`, or other mechanisms.

- **Syntax**:
  - `<if flag some_flag>`
  - `<if !flag some_flag>` (negated)
  - `<if flag some_flag eq "value">` (compare against a stored value)

**Example:**

```
<if flag has_ring> The ring glows. <else> It's dark. <endif>
<if flag dungeon_code eq "1234"> The door unlocks. <endif>
```

- You can compare string-based flags against specific values.
- String values must be enclosed in double quotes if they contain whitespace or special characters.
- Numeric values can be compared directly, or passed to a `<mod>` tag.

---

## 🔹 Item and Inventory Checks

Check whether the player has certain items, spells, or herbs.

- **Syntax**:
  - `<if item "item name">`
  - `<if !item "item name">`

**Example:**

```
<if !item "healing potion"> You should get one. <endif>
```

---

## 🔹 Quantity Checks

Check whether the player has at least a certain number of a specific item.

- **Syntax**: `<if has N item name>`\
  N must be a positive integer.

**Example:**

```
<if has 2 rations> You eat a ration. <endif>
```

---

## 🔹 Race and Profession Checks

Determine the player’s race or profession, matching against supported values.

- **Syntax**:

  - `<if race VALUE>` or `<if raceex VALUE>`
  - `<if prof VALUE>` or `<if profex VALUE>`

- **Valid values**:

  - **Race**: `human`, `orc`, `lesserorc`, `dwarf`, `elf`
  - **RaceEx**: adds `darkelf`, `barbarian`, `goblin`
  - **Profession**: `warrior`, `wizard`, `thief`, `woodsman`
  - **ProfEx**: adds `mercenary`, `assassin`

**Example:**

```
<if race darkelf> You sense the Severance. <endif>
<if prof wizard> You read the spellbook. <endif>
```

---

## 🔹 Unary Predicates (No Arguments)

These check singular player states:

- `premium` — Player has premium status
- `mounted` — Player has a horse
- `water` — Player has water access (via herb/spell/item)

**Syntax:**

```
<if premium> Welcome back, thanks for subscribing! <endif>
<if not water> You are dehydrated. <endif>
```

---

## 🔹 Logical Operators

Combine multiple conditions using:

- `and` — both sides must be true
- `or` — at least one must be true
- `not` — inverts the result
- Parentheses `(` and `)` are supported

**Example:**

```
<if (flag foo and not item "key")> You are locked in. <endif>
<if (luck gt 3 or premium)> You find a shortcut. <endif>
```

---

## 🔹 Else Support

The `<else>` tag may appear once between `<if>` and `<endif>`.

**Example:**

```
<if flag knows_truth> You nod. <else> You look confused. <endif>
```

---

## Additional Examples

```
<if stm lt 5> You have low stamina. <else> You're fine. <endif>
<if (race elf and has 2 arrows)> You fire twice. <endif>
<if flag companion_name eq "Marigold the Bold"> She smiles at you. <endif>
```

---

## EBNF Definition

```
<expression>       ::= <term> { "or" <term> }

<term>             ::= <factor> { "and" <factor> }

<factor>           ::= "not" <factor>
                     | "(" <expression> ")"
                     | <atom>

<atom>             ::= <comparison>
                     | <function_call>

<comparison>       ::= <identifier> <comparison_op> <number_or_dice>
                     | "flag" <identifier> [<comparison_op> <string>]

<comparison_op>    ::= "eq" | "ne" | "lt" | "gt" | "lte" | "gte"

<number_or_dice>   ::= <number> | "dice"

<function_call>    ::= <identifier> { <argument> }

<argument>         ::= <identifier> | <number>

<identifier>       ::= [a-zA-Z_][a-zA-Z0-9_]*

<number>           ::= [0-9]+

<string>           ::= "\".*\""
```

