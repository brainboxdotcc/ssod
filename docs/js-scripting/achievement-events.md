# 🏆 Achievement Event Types

Achievements in *Seven Spells of Destruction* are reactive scripts triggered by specific events that occur during gameplay. These events range from combat outcomes to inventory management, player choices, and state changes.

Each event type may pass zero or more **variables** into the achievement script, depending on the context.

> ⚠️ You cannot call `achievement_check()` manually. These events are fired automatically by the game engine at appropriate times.

---

## 🎯 Player Progress

- **LEVEL_UP** — Triggered when the player levels up
  - `level` (number): The player's new level

---

## ⚔️ Combat-Related Events

- **COMBAT** — A full combat event begins
  - `enemy` (object): `{ name, stamina, skill, armour, weapon }`

- **COMBAT_WIN** — The player wins a combat
  *(No variables)*

- **COMBAT_HIT** — The player is hit in combat
  - `stamina_damage` (number)
  - `skill_damage` (number)

- **CRITICAL** — A critical hit occurs (by or against the player)
  - `stamina_damage` (number)
  - `skill_damage` (number)

- **COMBAT_PLAYER_DEAD** — The player dies during combat
  - `enemy` (string): Name of the enemy that dealt the final blow

- **SNEAKTEST** — A sneak test is attempted
  - `success` (bool): Whether the player succeeded
  - `enemy` (string): Name of the enemy

- **TEST_LUCK**, **TEST_STAMINA**, **TEST_SKILL**, **TEST_SPEED**, **TEST_XP** — A test of the named stat
  - `success` (bool)

- **MOD** — A stat is modified
  - `modifier` (number): Amount of change
  - `stat` (string): Abbreviation of the stat

---

## 🧍 PvP Events

- **PVP_TIMEOUT**, **PVP_LOSE**, **PVP_WIN** — PvP combat outcome
  *(No variables)*

- **PVP_HIT** — A PvP hit lands
  - `stamina_damage`, `skill_damage` (numbers)

- **PVP_ACCEPT**, **CHALLENGE_PVP** — PvP challenge accepted or issued
  - `opponent` or `other_user` (string): Discord user ID

---

## 📦 Inventory and Items

- **PICKUP_FLOOR_ITEM** — Picked up an item from the floor
  - `name` (string): Item name
  - `flags` (string): Metadata flags

- **EQUIP_ITEM** — Equipped a weapon or armour
  - `name` (string): Item name
  - `rating` (number): Stat rating

- **DROP_ITEM** — Dropped an item
  - `item` (string): Item name
  - `value` (number): Gold value

- **DISCARD** — Discarded an item
  - `name` (string): Item name

- **USE_ITEM**, **USE_CONSUMABLE** — Used an item or consumable
  - `name` or `item` (string): Item name
  - `flags` (string)

- **EAT_FOOD** — Ate a food item
  - `name` (string): Food name
  - `food` (object): Full food record

- **SELL_ITEM** — Sold an item
  - `name` (string)
  - `value` (number)

- **CAST_SPELL** — Cast a spell
  - `spell` (string): Spell name

- **SCROLL** — Collected a major scroll
  - `scrolls` (number): Total scrolls now held (1–7)

- **CHOOSE_ITEM** — Chose an item from a list
  - `name` (string): Item name
  - `flags` (string)

---

## 🏕️ Camp and Survival

- **ENTER_CAMPFIRE**, **ENTER_INVENTORY**, **ENTER_GRIMOIRE**, **ENTER_BANK** — Entered specific interfaces
  *(No variables)*

- **COOK_RATIONS**, **COOK_MEAL** — Cooked rations or a meal
  - `name` (string): Recipe name

- **HUNT_SUCCESS** — Successful hunt
  - `name` (string): Ingredient
  - `animal` (string): Animal name
  - `localised_name`, `localised_animal` (strings)

- **HUNT_FAILURE** — Failed hunt
  *(No variables)*

- **FORCED_EAT** — Forced to eat
  *(No variables)*

---

## 🪙 Banking and Economy

- **BANK_DEPOSIT_GOLD**, **BANK_WITHDRAW_GOLD**
  - `amount` (string)

- **BANK_DEPOSIT_ITEM**, **BANK_WITHDRAW_ITEM**
  - `item` (string)
  - `flags` (string)

- **SHOP_BUY** — Bought something from a shop
  - `name` (string)
  - `cost` (number)

- **COLLECT** — Collected loot from a paragraph
  - `name` (string): gold, silver, scroll, or item
  - If `name` is:
    - **scroll** → No variables
    - **gold/silver** → `amount` (number)
    - *(other)* → `flags` (string)

---

## 📜 State and World Events

- **STATE**, **GLOBAL_STATE**, **TEMP_STATE**
  - `flag` (string): State flag set

- **TIME** — Time has passed (usually after a rest)
  *(No variables)*

- **VIEW_LOCATION** — Player viewed a specific paragraph
  - `loc_id` (number): Location ID

- **RESPAWN**, **RESURRECT**, **DEATH** — Death-related transitions
  *(No variables)*

---

## 🧼 Cosmetic or Miscellaneous

- **NAKED** — All armour unequipped
- **BARE_FISTED** — All weapons unequipped
  *(No variables)*

- **VOTE** — Player voted
  - `time` (number): Unix timestamp

- **SEND_CHAT** — Sent a chat message
  - `message` (string)

- **CURED** — Cured of disease
  - `disease` (string): e.g. `CUREBLOOD`, `CURERASP`

- **PLAGUE** — Contracted plague
  - `loss` (number): Stamina lost

---

> ✨ Use these events to trigger creative, humorous, or unexpected achievements. Just don’t rely on undocumented or engine-only variables, and always assume values may change in future versions.

