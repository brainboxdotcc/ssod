<?php
/* Generate one Seven Spells paragraph illustration.
 * Usage: php generate-image.php <paragraph-id>
 * Reads config.json in the current working directory.
 */

if (PHP_SAPI !== 'cli' || $argc !== 2 || !ctype_digit($argv[1])) {
    fwrite(STDERR, "Usage: php generate-image.php <paragraph-id>\n");
    exit(1);
}

$id = (int) $argv[1];
$config = json_decode(file_get_contents('../config.json'));
if (!$config || empty($config->gptkey) || empty($config->database)) {
    fwrite(STDERR, "Missing or invalid config.json\n");
    exit(1);
}

$db = new mysqli($config->database->host, $config->database->username,
    $config->database->password, $config->database->database);
if ($db->connect_errno) {
    fwrite(STDERR, "Database connection failed: {$db->connect_error}\n");
    exit(1);
}

$stmt = $db->prepare('SELECT data FROM game_locations WHERE id = ? LIMIT 1');
$stmt->bind_param('i', $id);
$stmt->execute();
$record = $stmt->get_result()->fetch_assoc();
if (!$record || trim((string) $record['data']) === '') {
    fwrite(STDERR, "Paragraph $id not found or empty.\n");
    exit(1);
}
$passage = $record['data'];

function postOpenAI(string $endpoint, array $payload, string $key): array {
    $ch = curl_init('https://api.openai.com/v1/' . $endpoint);
    curl_setopt_array($ch, [
        CURLOPT_POST => true,
        CURLOPT_RETURNTRANSFER => true,
        CURLOPT_TIMEOUT => 240,
        CURLOPT_HTTPHEADER => [
            'Authorization: Bearer ' . $key,
            'Content-Type: application/json',
        ],
        CURLOPT_POSTFIELDS => json_encode($payload, JSON_THROW_ON_ERROR),
    ]);
    $body = curl_exec($ch);
    $error = curl_error($ch);
    $status = curl_getinfo($ch, CURLINFO_HTTP_CODE);
    curl_close($ch);
    if ($body === false) {
        throw new RuntimeException("HTTP request failed: $error");
    }
    $decoded = json_decode($body, true);
    if ($status < 200 || $status >= 300 || !is_array($decoded)) {
        throw new RuntimeException("OpenAI HTTP $status: $body");
    }
    return $decoded;
}

try {
    echo "Preparing illustration prompt for paragraph $id...\n";
    $promptResponse = postOpenAI('chat/completions', [
	"model" => "gpt-6.1-terra",
	"reasoning_effort" => "none",
        'temperature' => 0.2,
        'messages' => [
            ['role' => 'system', 'content' => <<<'INSTRUCTIONS'
You are an art director preparing a single illustration prompt for a classic fantasy gamebook called The Seven Spells of Destruction.
Your task is to illustrate ONE specific moment from the passage, as if the player were physically present in the scene.
IMPORTANT RULES:
1. Depict what the player can actually see at their current location. Do not attempt to illustrate the entire paragraph literally.
2. Locations mentioned as destinations, directions, memories, rumours or background information must NOT appear in the image unless they would genuinely be visible from the player's current position.
3. Never combine multiple geographical locations into one composition. Do not create panoramic maps, geographical montages, split scenes, or illustrations showing several destinations simultaneously.
4. If the passage describes a crossroads, depict the crossroads itself. Do not depict the towns, deserts, forests or other destinations to which its roads lead.
5. Select ONE visually interesting subject or moment. Prefer a specific encounter, building, creature, object or environmental detail over a generic landscape.
6. Do not automatically include an adventurer, hooded figure or player character. The illustration should normally represent the player's point of view. Only depict characters explicitly present in the scene.
7. Do not invent landmarks, castles, mountains, rivers, creatures or other significant features merely to make the composition more dramatic.
8. Do not include text of any kind: no titles, captions, labels, maps, readable signs, lettering, watermarks or interface elements.
9. Ignore game mechanics and markup such as <LINK>, <BOOK>, <I>, <mod> and <tempset>. These are instructions to the game engine, not visual content.
10. Conditional branches (<if>, <else>, <endif>) may describe mutually exclusive situations. Never combine them. Select the most visually distinctive branch and illustrate only that version.
11. If the passage contains little visual description, create a restrained, plausible scene based on the immediate setting. Do not invent elaborate scenery to compensate for missing details.

ART DIRECTION:
Detailed, photorealistic fantasy world imagery with believable medieval architecture, natural materials, atmospheric depth and convincing lighting.
The image should feel like an illustration accompanying a passage in a fantasy adventure novel, NOT a map, poster, promotional illustration or visual summary.
Use lighting appropriate to the actual environment. Underground scenes must not contain sunlight unless explicitly described.
Maintain a grounded, slightly gritty fantasy aesthetic. Avoid exaggerated high-fantasy spectacle unless the passage specifically calls for it.

OUTPUT:
Write one concise, visually specific image-generation prompt in English.
Describe the primary subject, immediate surroundings, composition, atmosphere and lighting.

Do not explain your reasoning.
Do not include a preamble.
Do not reproduce game commands.
Output ONLY the image-generation prompt.
INSTRUCTIONS],
            ['role' => 'user', 'content' => "Gamebook paragraph $id:\n\n" . $passage],
        ],
    ], $config->gptkey);

    $prompt = trim($promptResponse['choices'][0]['message']['content'] ?? '');
    if ($prompt === '') {
        throw new RuntimeException('No illustration prompt returned.');
    }
    echo "Prompt: $prompt\n";
    echo "Generating PNG...\n";
    $imageResponse = postOpenAI('images/generations', [
        'model' => 'gpt-image-1.5',
        'prompt' => $prompt . "\n\n"
    . "Create a single immersive fantasy gamebook illustration. "
    . "Do not include any text, labels, captions, maps, diagrams, "
    . "split compositions or geographical montages. "
    . "Depict only the immediate scene described.",
        'size' => '1536x1024',
        'quality' => 'medium',
        'output_format' => 'png',
        'n' => 1,
    ], $config->gptkey);

    $encoded = $imageResponse['data'][0]['b64_json'] ?? null;
    if (!$encoded) {
        throw new RuntimeException('Image API returned no base64 image: ' . json_encode($imageResponse));
    }
    $png = base64_decode($encoded, true);
    if ($png === false || substr($png, 0, 8) !== "\x89PNG\r\n\x1a\n") {
        throw new RuntimeException('Image API returned invalid PNG data.');
    }
    $filename = "../resource/paragraph_pictures/$id.png";
    if (file_put_contents($filename, $png) === false) {
        throw new RuntimeException("Could not write $filename");
    }
    echo "Saved: $filename (" . number_format(strlen($png)) . " bytes)\n";
    shell_exec("tiny " . escapeshellarg($filename));
} catch (Throwable $e) {
    fwrite(STDERR, "Error: {$e->getMessage()}\n");
    exit(1);
}
