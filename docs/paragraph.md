# Paragraph Language Guide

*Paragraph* is a custom language used to define content, locations and encounters in The Seven Spells Of Destruction. It's syntax is somewhat HTML like, except it is stricter in some ways about positioning of elements.

Paragraph tags are never considered to be a DOM. Only certain tags have range and can contain other content, such as `<if>` and `<endif>`. Tags which do this cannot be nested, e.g. you **cannot have two levels of `<if>`.

* Spacing between tags is essential
* You should  only ever have one space between attributes in a tag.
* You MUST have at least one space or newline between the leading or terminating < > and any text outside the tag, e.g. `Go to <LINK=5>` and not `Go to<LINK=5>`
* Ordering of items within a tag is important and must follow the examples in this documentation (e.g. with the `<I>` tag). This is for simplicity of the parser, and speed of parsing.

<table>
  <tbody>
    <tr>
      <td>Tag</td>
      <td>Description</td>
      <td>Example</td>
    </tr>
    <tr>
      <td>&lt;LINK=ID&gt;</td>
      <td>Link to another location</td>
      <td>&lt;LINK=111&gt;</td>
    </tr>
    <tr>
      <td>&lt;AUTOLINK=ID&gt;</td>
      <td>Link to another location. These are used with score tests, you must use two of them after each &lt;TEST&gt; tag. The tag for the failing condition will be grayed.</td>
      <td>&lt;TEST LUCK&gt; are you lucky &lt;AUTOLINK=1&gt; or unlucky &lt;AUTOLINK=2&gt;</td>
    </tr>
    <tr>
      <td>&lt;paylink=X,ID&gt;</td>
      <td>Link to another location. Upon clicking the link, X gold is subtracted from the purse of the player. If the player does not have enough gold the link is grayed.</td>
      <td>&lt;paylink=10,150&gt;</td>
    </tr>
    <tr>
      <td>&lt;pickup ITEM [STAT+-N]&gt;</td>
      <td>
        Automatically add an item to a players possessions list. The item name ITEM is shown in the backpack, STAT is the score it modifies (optional) followed by a +N or -N value for that score. See the example for information.
        <p>Special: &lt;pickup gold X&gt; to pick up gold, &lt;pickup silver X&gt; to pick up silver. Negative figures drop gold or silver.</p>
        <p>Spells are special items where STAT+-N == "SPELL".</p>
        <p>Herbs are special items where STAT+-N == "HERB"</p>
      </td>
      <td>
        &lt;pickup Special item that adds stm [ST+6]&gt;
        <p>&lt;pickup Rating 5 Sword [W5]&gt;</p>
        <p>&lt;pickup gold 6&gt;</p>
      </td>
    </tr>
    <tr>
      <td>&lt;drop ITEM&gt;</td>
      <td>Drops ITEM from the backpack of the player, case insensitive. If the item does not exist nothing happens.</td>
      <td>&lt;drop sword&gt;</td>
    </tr>
    <tr>
      <td>&lt;mod STAT +-N&gt;</td>
      <td>Modify a player score. Valid values for STAT are: stm - stamina; skl - skill; wpn - weapon; arm - armour; exp - experience; luck - luck; spd - speed</td>
      <td>&lt;mod stm -5&gt;</td>
    </tr>
    <tr>
      <td>
        &lt;if STAT eq|gt|lt|ne N&gt; ... [&lt;else&gt;] ... &lt;endif&gt;
        <p>&lt;if flag FLAG_ID&gt; ... [&lt;else&gt;] .. &lt;endif&gt;</p>
        <p>&lt;if dice eq|gt|lt|ne N&gt; ... [&lt;else&gt;] ... &lt;endif&gt;</p>
        <p>&lt;if STAT eq|gt|lt|ne dice&gt; ... [&lt;else&gt;] ... &lt;endif&gt;</p>
        <p>&lt;if race orc&gt;, &lt;if race human&gt;....</p>
        <p>&lt;if prof wizard&gt;, &lt;if prof warrior&gt;</p>
      </td>
      <td>
        Display next part of the paragraph if STAT is equal (eq), greater than (gt), less than (lt) or not equal (ne) to the value you provide. Valid values for STAT are: stm - stamina; skl - skill; wpn - weapon; arm - armour; exp - experience; luck - luck; spd - speed
        <p>When checking with the &lt;if flag&gt; tag, you may check against a local flag (set with &lt;set&gt;) or a global flag (set with &lt;setglobal&gt;).</p>
        <p>With the &lt;if dice&gt; tag, you must have used the &lt;dice&gt; tag previously to have a result to check against. N must be in the range 1-6.</p>
        <p>When using the &lt;if prof&gt; or &lt;if race&gt; tags, these compare against the current players race or profession, only use the ones valid in the game, in lower case only.</p>
      </td>
      <td>
        &lt;if stm lt 5&gt; you have less than 5 stamina &lt;else&gt; you have 5 or more stamina &lt;endif&gt;
        <p>&lt;if stm gt 20&gt; you're a hard bastard &lt;endif&gt;</p>
        <p>&lt;if dice eq 1&gt; You rolled a 1 &lt;endif&gt;</p>
      </td>
    </tr>
    <tr>
      <td>
        &lt;I NAME="Item Name" COST="X"&gt;
        <p>&lt;I NAME="Item Name" VALUE="STAT+-N" COST="X"&gt;</p>
      </td>
      <td>
        A buyable item, e.g. from a shop. Item Name will be displayed in backpack if bought. The cost value X is the cost in gold of the item. If a VALUE parameter is included it indicates the special STAT value of the item. Valid values for STAT are: ST - stamina; SK - skill; W - weapon; A - armour; EX - experience; LK - luck; SD - speed.
        <p>Spells are special items where STAT+-N == "SPELL".</p>
        <p>Herbs are special items where STAT+-N == "HERB"</p>
      </td>
      <td>
        &lt;I NAME="Stamina restorer +6" VALUE="ST+6" COST="10"&gt;
        <p>&lt;I NAME="Pointless junk" COST="100"&gt;</p>
        <p>&lt;I NAME="fly" VALUE="SPELL" COST="1"&gt;</p>
      </td>
    </tr>
    <tr>
      <td>&lt;TEST STAT&gt;</td>
      <td>Used to test a stat. See AUTOLINK. Valid values for STAT are SKILL, STAMINA, ARMOUR, WEAPON, LUCK</td>
      <td>&lt;TEST LUCK&gt;</td>
    </tr>
    <tr>
      <td>&lt;SET FLAG_ID&gt;</td>
      <td>Sets a flag which you can later check against with the &lt;if&gt; tag. Flags are saved to the player's entry in the database and so are permanent until removed.</td>
      <td>&lt;set killed_some_boss&gt;</td>
    </tr>
    <tr>
      <td>&lt;COMBAT NAME="monster" SKILL=X STAMINA=Y ARMOUR=Z WEAPON=T&gt;</td>
      <td>Creates a monster to fight, with the stats X, Y, Z and T. The name "monster" is shown to the player on their screen. Progress through the location will not continue until after the monster is defeated. Multiple combats may be placed one after the other. It is important to preserve the order of attributes.</td>
      <td>&lt;COMBAT NAME="1ST ORC" SKILL=5 STAMINA=8 ARMOUR=6 WEAPON=6&gt;</td>
    </tr>
    <tr>
      <td>&lt;PICK NAME="item name" VALUE="STAT+-N"&gt;</td>
      <td>
        A pickable item, for a "you may take only one item" event. Item Name will be displayed in backpack if bought. The VALUE parameter must be included and it indicates the special STAT value of the item. Valid values for STAT are: ST - stamina; SK - skill; W - weapon; A - armour; EX - experience; LK - luck; SD - speed.
        <p>Spells are special items where STAT+-N == "SPELL".</p>
        <p>Herbs are special items where STAT+-N == "HERB"</p>
      </td>
      <td>
        You may take one item only:
        <p>&lt;pick name="Item 1" VALUE="ST+5"&gt;</p>
        <p>&lt;pick name="Item 2" VALUE="SK+5"&gt;</p>
      </td>
    </tr>
    <tr>
      <td>&lt;EAT&gt;</td>
      <td>Eats a ration from a players inventory, or subtracts two stamina if there is not enough food for the player to eat.</td>
      <td>&lt;eat&gt;</td>
    </tr>
    <tr>
      <td>&lt;D12&gt;</td>
      <td>Roll a 12 sided dice, store score in engine for later.</td>
      <td>&lt;d12&gt;</td>
    </tr>
    <tr>
      <td>&lt;2D6&gt;</td>
      <td>Roll two dice, store total sum score in engine for later.</td>
      <td>&lt;2d6&gt;</td>
    </tr>
    <tr>
      <td>&lt;SETGLOBAL FLAG_ID&gt;</td>
      <td>This tag sets a global flag. A global flag is stored in a mysql table which all users can access, therefore if a global flag is set for one user it is set for them all. Useful for game persistence, e.g. if one user opens a door and it cannot be closed again, you may want to use &lt;setglobal&gt; to mark the door as open to all other users that encounter it. You may check against a global flag with the &lt;if&gt; tag, in the same way you can check against a flag which is specific to a user, placed with &lt;set&gt;. Note that global flags, like user specific flags, cannot contain spaces, and global and local flags may not conflict (have the same names) otherwise a condition will occur if either of the two flags is set)</td>
      <td>&lt;setglobal blew_up_the_door&gt;</td>
    </tr>
    <tr>
      <td>&lt;UNSETGLOBAL FLAG_ID&gt;</td>
      <td>Use this tag to unset a global flag that has been set with setglobal.</td>
      <td>&lt;unsetglobal blew_up_the_door&gt;</td>
    </tr>
    <tr>
      <td>&lt;INPUT PROMPT="prompt line" LOCATION="ID" VALUE="correct_answer"&gt;</td>
      <td>This tag asks the user for an answer to some form of text riddle or question, e.g. "how many fingers am i holding up". If the user enters the correct answer (which cannot be more then one word) then the location referred to by ID is jumped to, same as if they'd clicked a link. See the example. Note that the answer is CaSe SeNsiTiVe!</td>
      <td>&lt;input prompt="What creature beginning with A eats ants?" location="666" value="anteater"&gt;</td>
    </tr>
    <tr>
      <td>&lt;DICE&gt;</td>
      <td>Roll a dice, store score in engine for later.</td>
      <td>&lt;dice&gt;</td>
    </tr>
  </tbody>
</table>
<p>&nbsp;</p>
