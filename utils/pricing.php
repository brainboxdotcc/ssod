<?php

exit(0);

$config = json_decode(file_get_contents("config.json"));
$db = mysqli_connect($config->database->host, $config->database->username, $config->database->password, $config->database->database);

// first, do spells

$p = mysqli_query($db, "SELECT id, data FROM game_locations WHERE data IS NOT NULL AND trim(data) != ''");

$out = [];

while ($loc = mysqli_fetch_object($p)) {
	$data = $loc->data;
	if (preg_match_all('/<(pick) name="(.+?)" [^>]+>/i', $data, $matches, PREG_PATTERN_ORDER) || preg_match_all('/<(pick) (.+?)>/i', $data, $matches, PREG_PATTERN_ORDER)) {
		for ($n = 0; $n < count($matches); ++$n) {
			$cost = 1;
			$name = $matches[2][$n] ?? '';
			$tag = $matches[1][$n] ?? '';
			if (preg_match('/^(gold|silver) [-\d]+$/', $name) || $name == 'scroll') {
				continue;
			}
			$flag = "";
			if (strtolower($matches[1][$n] ?? '') == 'i') {
				preg_match('/value="(\S+)"/i', $matches[0][$n], $flags);
				preg_match('/cost="([A-Z0-9]+)"/i', $matches[0][$n], $costs);
				if (count($costs) > 0) {
					$cost = $costs[1];
				}
				if (count($flags) > 0) {
					$flag = $flags[1];
				}
	
			} else {
				preg_match('/\[(\S+)\]/i', $matches[0][$n] ?? '', $flags);
				if (count($flags) > 0) {
					$flag = $flags[1];
				}
				if (preg_match('/value="(\S+)"/i', $matches[0][$n], $flags)) {
					if (count($flags) > 1) {
						print_r($flags);
						$flag = $flags[1];
						$tag = "pick";
						echo $name . "\n";
					}
				}
			}
			$name = preg_replace('/ \[\S+?\]$/', '', trim($name));
			if (!empty(trim($tag))) {
				$sellable = true;
				if ($flag == 'SPELL') {
					$sellable = false;
				}
				if (preg_match('/(\d+)/', $flag, $m)) {
					if ($cost < $m[1] && $m[1] > 0) {
						// Cost for items without a cost is trhe rating
						$cost = $m[1];
					}
				}
				if (empty($out[$name])) {
					$out[$name] = [
						'name' => $name,
						'flags' => $flag,
						'value' => $cost ? ceil($cost / 2) : 1,
						'sellable' => $sellable,
					];
				} else {
					if ($out[$name]["value"] > $cost && $cost > 0 && !empty($cost)) {
						$out[$name] = [
							'name' => $name,
							'flags' => $flag,
	       						'value' => $cost ? ceil($cost / 2) : 1,
	       						'sellable' => $sellable,
						];						
					}
				}
			}
		}
	}
}
foreach ($out as $item) {
	$existing = mysqli_fetch_object(mysqli_query($db, "SELECT * FROM game_item_descs WHERE name = '".$item["name"]."'"));
	$flags = $item["flags"];
	$value = $item["value"];
	$name = $item["name"];
	$sellable = (int)$item["sellable"];
	if ($existing) {
		mysqli_query($db, "UPDATE game_item_descs SET flags = '$flags' WHERE id = $existing->id");
		print "UPDATE game_item_descs SET flags = '$flags' WHERE id = $existing->id -- $name\n";
	}
}

