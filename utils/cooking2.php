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
$list = array_values($list);
$missing = [];
$foodNames = [];
$foodIngr = [];
$foods = json_decode(file_get_contents("food.json"));
echo "Number of dishes: " . count($foods) . "\n";
mysqli_query($db, "DELETE FROM ingredients");
mysqli_query($db, "DELETE FROM food");

foreach ($foods as $food) {
	if (in_array($food->name, $foodNames)) {
		echo "Dupe food name: $food->name\n";
	}
	asort($food->ingredients);
	$m = strtoupper(json_encode(array_values($food->ingredients)));
	if (in_array($m, $foodIngr)) {
		echo "Dupe ingredients: $food->name $m in " . $foodIngr[$m] . "\n";
	}
	$foodIngr[$m] = $food->name;
	$foodNames[] = $food->name;
}
foreach ($foods as $food) {
	$food->name = mysqli_escape_string($db, $food->name);
	//echo $food->name . "...\n";
	$food->description = mysqli_escape_string($db, $food->description);
	$value = (int)($food->stamina_change ?? 0) + (int)($food->skill_change ?? 0) + (int)($food->luck_change ?? 0) + (int)($food->speed_change ?? 0);
	foreach ($food->ingredients as $item) {
		if (in_array($item, $foodNames)) {
			$value *= 4;
		}
	}
	mysqli_query($db, "INSERT INTO food (name, description, stamina_change, skill_change, luck_change, speed_change, value) VALUES('$food->name','$food->description','".($food->stamina_change ?? 0)."','".($food->skill_change ?? 0)."','".($food->luck_change ?? 0)."','".($food->speed_change ?? 0)."', $value)");
	$id = mysqli_insert_id($db);
	foreach ($food->ingredients as $item) {
		if (!in_array($item, $list) && !in_array($item, $foodNames)) {
			$missing[$item] = true;
		}
		$item = mysqli_escape_string($db, $item);
		mysqli_query($db, "INSERT INTO ingredients (food_id, ingredient_name) VALUES($id, '$item')");
	}
}
$missing = array_keys($missing);
print_r($missing);
