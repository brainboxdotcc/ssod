# Achievement Event Types

### LEVEL_UP
* level - numeric, new level
### PVP_TIMEOUT
No variables
### PVP_LOSE
No variables
### PVP_WIN
No variables
### PVP_HIT
* stamina_damage - numeric, stamina damage
* skill_damage - numeric, skill damage
### COMBAT_WIN
No variables
### COMBAT_HIT
* stamina_damage - numeric, stamina damage
* skill_damage - numeric, skill damage
### COMBAT_PLAYER_DEAD
* enemy - string, enemy name
### BANK_DEPOSIT_GOLD
* amount - string, gold amount
### ANSWER_RIDDLE_CORRECT
No variables
### ANSWER_RIDDLE_INCORRECT
No variables
### SEND_CHAT
* message - string, chat message
### BANK_WITHDRAW_GOLD
* amount - string, gold amount
### BANK_WITHDRAW_ITEM
* item - string, item name
* flags - string, item flags
### BANK_DEPOSIT_ITEM
* item - string, item name
* flags - string, item flags
### NAKED
Player de-equipped all armour

No variables
### BARE_FISTED
Player de-equipped all weapons

No variables
### DROP_ITEM
* item - string, item name
* value - numeric, item gold value
### CAST_SPELL
* spell - string, spell name
### COOK_RATIONS
* name - string, ration recipe name
### COOK_MEAL
* name - string, meal name
### USE_ITEM
* item - string, item name
* flags - string, item flags
### USE_CONSUMABLE
* name - string, item name
### EAT_FOOD
* name - string, food name
* food - object, food row from database
### EQUIP_ITEM
* name - string, weapon/armour name
* rating - numeric, weapon/armour rating
### SELL_ITEM
* name - string, item name
* value - numeric, item gold value
### CHALLENGE_PVP
* other_user - string, other user snowflake ID
### SHOP_BUY
* name - string, item name
* cost - numeric, gold value
### COMBAT
* enemy - object
  * name - string, enemy name
  * stamina - numeric, enemy stamina
  * skill - numeric, enemy skill
  * armour - numeric, enemy armour rating
  * weapon - numeric, enemy weapon rating
### ENTER_BANK
No variables
### CHOOSE_ITEM
* name - string, item name
* flags - string, item flags
### RESPAWN
No variables
### RESURRECT
No variables
### ENTER_INVENTORY
No variables
### ENTER_GRIMOIRE
No variables
### ENTER_CAMPFIRE
No variables
### HUNT_SUCCESS
* name - string, found ingredient name
* animal - string, killed animal name
* localised_name - string, localised ingredient name
* localised_animal - string, localised animal name
### HUNT_FAILURE
No variables
### PICKUP_FLOOR_ITEM
* name - string, item name
* flags - string, item flags
### PVP_ACCEPT
* opponent - string, opponent snowflake ID
### PVP_ACCEPT
* opponent - string, opponent snowflake ID
### VIEW_LOCATION
* loc_id - numeric, paragraph id
### TEST_SNEAK
* success - boolean, success or failure of test
* enemy - string, enemy name
### TIME
No variables
### COLLECT
* name - string, item name

If name is 'scroll' there are no further variables.
If the name is 'gold' or 'silver':
* amount - numeric, gold or silver amount

Otherwise:
* flags - string, item flags
### TEST_LUCK
* success - boolean, success or failure of test
### TEST_STAMINA
* success - boolean, success or failure of test
### TEST_SKILL
* success - boolean, success or failure of test
### TEST_SPEED
* success - boolean, success or failure of test
### TEST_XP
* success - boolean, success or failure of test
### MOD
* modifier - numeric, stat modifier amount
* stat - string, short stat name
### TEMP_STATE
* flag - string, flag name
### GLOBAL_STATE
* flag - string, flag name
### DISCARD
* name - string, item name
### STATE
* flag - string, flag name
### FORCED_EAT
No variables
