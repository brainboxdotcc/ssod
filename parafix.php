<?php
require 'vendor/autoload.php';

$config = json_decode(file_get_contents("config.json"));
$client = OpenAI::client($config->gptkey);
$db = mysqli_connect($config->database->host, $config->database->username, $config->database->password, $config->database->database);

while (true) {
	$paragraph = readline("Paragraph ID: ");
	echo "================================================================================\n";
	$data = mysqli_fetch_object(mysqli_query($db, "SELECT * FROM game_locations WHERE id = " . (int)$paragraph));
	if ($data) {
		echo $data->data . "\n\n";
		$r = $client->chat()->create([
			'model' => 'gpt-3.5-turbo',
			'messages' => [
				[
					"role" => "system",
					"content" => "Task: The text contains <LINK> tags. Do the following:
					Summarize the context of the sentence before a link tag into short words.
					Insert the summarized text into the link tag. Example: TEXT <LINK=1> is converted to TEXT <LINK=1, CONTEXT> where CONTEXT is summarized context of the sentence before LINK tag.
					The context refers to an action a player can take.
					Do not edit anything outside the LINK tag.
					There should be NO punctuation in the CONTEXT.
					Do not edit any other types of tag than LINK.
					Each LINK should occur only ONCE in the output, determined by the LINK number.
					CONTEXT should be as short as possible. Not more than 2-4 words.
					Output ONLY the replaced <LINK, CONTEXT> tags each on a separate line. NOTHING else should be output."
				],
				[
					"role" => "user",
					"content" => $data->data,
				],
			],
		]);
		$corrections = explode("\n", $r->choices[0]->message->content);
		print_r($corrections);
		foreach ($corrections as $correction) {
			$find = substr($correction, 0, strpos($correction, ',')) . '>';
			$replace = $correction;
			$data->data = preg_replace('/' . $find . '/', $replace, $data->data);
		}
		echo "================================================================================\n";
		echo $data->data . "\n\n";
		$yn = readline("IS THIS OK [YN]? ");
		if (preg_match('/y/', $yn)) {
			$data->data = mysqli_escape_string($db, $data->data);
			mysqli_query($db, "UPDATE game_locations SET data = '" . $data->data . "' WHERE id = " . (int)$paragraph);
		}
	}
}
