<?php
require 'vendor/autoload.php';

$config = json_decode(file_get_contents("config.json"));
$client = OpenAI::client($config->gptkey);
$db = mysqli_connect($config->database->host, $config->database->username, $config->database->password, $config->database->database);

$q = mysqli_query($db, "SELECT json_extract(json_extract(hunting_json, '$.animals.*'), '$[*][*]') as loot FROM `game_locations` where json_extract(hunting_json, '\$.probability') > 0 ORDER BY `game_locations`.`id` ASC");
$list = [];
while ($loot = mysqli_fetch_object($q)) {
	$l = json_decode($loot->loot);
	foreach ($l ?? [] as $item) {
		$list[] = $item;
	}
}
$list = array_unique($list);
shuffle($list);
$list = json_encode(array_values($list));

$r = $client->chat()->create([
	'model' => 'gpt-3.5-turbo',
	'messages' => [
		[
			"role" => "system",
			"content" => "You are a helpful assistant that does as the user asks."
		],
		[
			"role" => "user",
			"content" => "Task: Given below is a json array of various high fantasy ingredient items from hunting in a high fantasy game. For the list, suggest many different combinations of these items, to produce cooked food at a campfire in a similar vein to how this works in Legend of Zelda Tears Of the Kingdom. For each, suggest how it would affect the core stats of the player: Stamina (health), Skill (combat effectiveness), Luck, Speed
Return the output one dish per line, as many lines as possible, with each dish requiring 2 to a maximum of six items to create. 
the output of each dish should be in json as follows:
{\"name\":\"food name\", \"description\":\"short description of food\",\"ingredients\":[\"ingredient\",\"...\"], \"stamina_change\":1,\"skill_change\":1,\"luck_change\":1,\"speed_change\":1}
The output should be an ARRAY of these objects.
You should endevour to return as many of the recipies as possible within the allowances of the API response.
recipies predominantly from one animal should imbue the mythical characteristics of that animal, e.g. rabbits = luck, snakes = sneakiness, lions = strength etc.
the numbers for the changes should vary accoridngly. if there is no change applied, the change value may be entirely omitted along with its key.
DO NOT OUTPUT COMMA SEPARATED LISTS for the result.
DO NOT ADD ANY INGREDIENTS THAT ARE NOT PART OF THE ORIGINAL LIST BELOW. NO VEGETABLES, BREAD PRODUCTS, DAIRY OR CEREALS
No raw or inedible items but strange items are acceptable.
No duplicate food names.
STAT CHANGES should be in the range 2-8 depending on how many items are in the recipie.
If the majority of ingredients come from a certain animal, prefix the name of the dish with the animals name e.g. \"boar stew\".
Food with more ingredients will give better buffs.
Display only the json.
Do not prefix or suffix the result. The output is to be fed into a computer program and must be only json.

Text is:
```
$list
```",
		],
	],
]);
$json = $r->choices[0]->message->content;
echo $json;

