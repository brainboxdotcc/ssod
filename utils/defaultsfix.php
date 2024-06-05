<?php

$config = json_decode(file_get_contents("config.json"));
$db = mysqli_connect($config->database->host, $config->database->username, $config->database->password, $config->database->database);

// first, do spells

$over = mysqli_query($db, "SELECT user_id, count(flags) as c FROM `game_default_spells` where flags = 'SPELL' GROUP BY flags, user_id having count(flags) > 3 ORDER BY count(flags) DESC");

while ($row = mysqli_fetch_object($over)) {
	$player = mysqli_fetch_object(mysqli_query($db, "SELECT * FROM game_default_users WHERE user_id = $row->user_id"));
	if ($player->profession == 2) { // wizard
		$max = 5;
	} else {
		$max = 2;
	}
	if ($row->c > $max) {
		print "Over spells: $row->user_id\n";
	}
	$spells = mysqli_query($db, "SELECT * FROM game_default_spells WHERE user_id = $row->user_id AND flags = 'SPELL' ORDER BY id");
	$index = 1;
	while ($s = mysqli_fetch_object($spells)) {
		if ($index > $max) {
			mysqli_query($db, "DELETE FROM game_default_spells WHERE id = $s->id");
			print "Deleted spell at id $s->id (count $index beyond $max)\n";
		}
		$index++;
	}

}

$over = mysqli_query($db, "SELECT user_id, count(flags) as c FROM `game_default_spells` where flags = 'HERB' GROUP BY flags, user_id having count(flags) > 3 ORDER BY count(flags) DESC");

while ($row = mysqli_fetch_object($over)) {
        print "Over herbs: $row->user_id\n";
        $spells = mysqli_query($db, "SELECT * FROM game_default_spells WHERE user_id = $row->user_id AND flags = 'HERB' ORDER BY id");
        $index = 1;
        while ($s = mysqli_fetch_object($spells)) {
                if ($index > 3) {
			mysqli_query($db, "DELETE FROM game_default_spells WHERE id = $s->id");
			print "Deleted herb at id $s->id (count $index beyond 3)\n";
                }
                $index++;
        }
}
