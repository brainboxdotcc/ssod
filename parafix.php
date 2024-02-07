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
					"content" => "You are a helpful assistant that does as the user asks."
				],
				[
					"role" => "user",
					"content" => "Task: The text contains <LINK> tags. Do the following:
Summarize the context of the sentence before a link tag into short words.
Insert the summarized text into the link tag. Example: TEXT <LINK=1> is converted to TEXT <LINK=1, CONTEXT> where CONTEXT is summarized context of the sentence before LINK tag.
The context refers to an action a player can take.
Do not edit anything outside the LINK tag.
There should be NO punctuation in the CONTEXT.
Do not edit any other types of tag than LINK.
Prefer text closest to the LINK tag for building CONTEXT.
Do not rearrange words in CONTEXT, it should still be understandable english.
Each LINK should occur only ONCE in the output, determined by the LINK number.
CONTEXT should be as short as possible. Not more than 3-4 words.
Do not enclose CONTEXT in quotes.
Display ONLY the <LINK, CONTEXT>, one per line. NO OTHER PART of the text should be replied with
Do not prefix your reply with anything. The output is to be fed into a computer program.
					
Text is:
```
$data->data
```",
				],
			],
		]);
		$corrections = explode("\n", $r->choices[0]->message->content);
		print_r($corrections);
		foreach ($corrections as $correction) {
			$correction = preg_replace("/(<.+?>)/", "$1", $correction);
			$find = trim(substr($correction, 0, strpos($correction, ',')) . '>');
			$replace = trim($correction);
			if (preg_match('/^<link=/i', $replace))  {
				$data->data = str_ireplace($find, $replace, $data->data);
			}
		}
		echo "================================================================================\n";
		echo $data->data . "\n\n";
		$yn = readline("IS THIS OK [YN]? ");
		if (preg_match('/y/i', $yn)) {
			$data->data = mysqli_escape_string($db, $data->data);
			mysqli_query($db, "UPDATE game_locations SET data = '" . $data->data . "' WHERE id = " . (int)$paragraph);
		}
	}
}
