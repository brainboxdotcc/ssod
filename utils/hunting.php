<?php
require 'vendor/autoload.php';

$config = json_decode(file_get_contents("config.json"));
$client = OpenAI::client($config->gptkey);
$db = mysqli_connect($config->database->host, $config->database->username, $config->database->password, $config->database->database);

$q = mysqli_query($db, "SELECT * FROM game_locations WHERE hunting_json IS NULL AND trim(data) != '' AND data IS NOT NULL");
while ($paragraphData = mysqli_fetch_object($q)) {
	echo "================================================================================\n";
	$data = $paragraphData;
	if ($data) {
		$r = $client->chat()->create([
			'model' => 'gpt-3.5-turbo',
			'messages' => [
				[
					"role" => "system",
					"content" => "You are a helpful assistant that does as the user asks."
				],
				[
					"role" => "user",
					"content" => "Task: The text is a description of a location in a text based high fantasy medieval adventure game.
Given the content of the text, determine the probability between 0 and 1 of a player successfully hunting for wild game and other animals at that location.
Indoor locations and locations with combat shall always have probability 0. You should also list the potential animals found at that location for hunting,
and what loot items they drop. loot items should be harvestable realistic parts the player could craft/cook into recipies etc.
The result should be given only ever as json. Here is an example schema:
{\"probability\":0.64,\"reason\":\"Reason that animals are found here (or not)\",\"animals\":{\"fox\":[\"pelt\",\"meat\"],\"water fowl\":[\"bird leg\",\"yellow feathers\"]}}
DO NOT OUTPUT COMMA SEPARATED LISTS for the result.
The reason given in json should be in first person in the form to be displayed to the player.
No pre-prepared items such as stews, steaks.
Probabilities should be no higher than 0.1 if the area seems to be close to human habitat. the more wilderness the area the higher the chance of encountering an animal.
Keep animal names short, and loot names short. Single word or two word loot names are preferable.
No duplicate animal names, but duplicate loot names are fine.
Prefix body parts with name of animal at all times, e.g. \"squirrel leg\" not just \"leg\".
No more than eight animals should be returned and no more than four loot items per animal.
The animal with highest probability of finding should be listed first, and the lowest probability last in descending order.
Loot to be found on the animal should be listed in order of commonality.
Display only the json.
Do not prefix or suffix the result. The output is to be fed into a computer program and must be only json.

Text is:
```
$paragraphData->data
```",
				],
			],
		]);
		$json = $r->choices[0]->message->content;
		$json = json_encode(json_decode($json), true);
		if ($json !== 'null') {
			print $paragraphData->id . ": " . $json . "\n";
			mysqli_query($db, "UPDATE game_locations SET hunting_json = '" . mysqli_real_escape_string($db, $json) . "' WHERE id = " . (int)$paragraphData->id);
		}
	}
}
