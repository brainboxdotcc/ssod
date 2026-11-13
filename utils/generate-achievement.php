
<?php
/*
 * Generate Seven Spells Steam achievement icons and CSV exports.
 *
 * Usage:
 *   php generate-achievement.php <achievement-id>
 *   php generate-achievement.php all
 *
 * Requires:
 *   ../config.json
 *   ./frame.png (256x256)
 *   PHP Imagick extension
 *
 * Outputs:
 *   SLUG_ACHIEVED.png
 *   SLUG_UNACHIEVED.png
 *   achievements-definitions.csv
 *   achievements-localization.csv
 */

if (PHP_SAPI !== 'cli' || $argc !== 2 ||
	($argv[1] !== 'all' && !ctype_digit($argv[1]))) {
	fwrite(STDERR, "Usage: php generate-achievement.php <achievement-id|all>\n");
	exit(1);
}

$config = json_decode(file_get_contents('../config.json'));
if (!$config || empty($config->gptkey) || empty($config->database)) {
	fwrite(STDERR, "Missing or invalid config.json\n");
	exit(1);
}

if (!extension_loaded('imagick')) {
	fwrite(STDERR, "PHP Imagick extension is required.\n");
	exit(1);
}

$framePath = __DIR__ . '/frame.png';
if (!is_file($framePath)) {
	fwrite(STDERR, "Missing frame.png\n");
	exit(1);
}

$frame = new Imagick($framePath);
if ($frame->getImageWidth() !== 256 || $frame->getImageHeight() !== 256) {
	fwrite(STDERR, "frame.png must be 256x256.\n");
	exit(1);
}
$frame->destroy();

$db = new mysqli(
	$config->database->host,
	$config->database->username,
	$config->database->password,
	$config->database->database
);

if ($db->connect_errno) {
	fwrite(STDERR, "Database connection failed: {$db->connect_error}\n");
	exit(1);
}

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

/*
 * Remove magenta pixels, preserving all other colours.
 *
 * Applied to the composited image before saving either version.
 * The tolerance is deliberately small to avoid removing legitimate
 * pink or purple details from the generated artwork.
 */
function removeMagenta(Imagick $image): void {
	$image->setImageAlphaChannel(Imagick::ALPHACHANNEL_ACTIVATE);

	$range = $image->getQuantumRange();
	$quantum = $range['quantumRangeLong'];

	// Approximately 3% tolerance around pure #FF00FF.
	$image->setOption('fuzz', (string) (0.03 * $quantum));

	$image->transparentPaintImage(
		new ImagickPixel('#FF00FF'),
		0.0,
		0.03 * $quantum,
		false
	);
}

/*
 * Generate the unachieved version using Imagick.
 *
 * Saturation is reduced to zero and brightness to 85%.
 * The alpha channel is preserved.
 */
function generateUnachieved(string $achieved, string $unachieved): void {
	$image = new Imagick($achieved);

	$image->modulateImage(85, 0, 100);
	$image->setImageFormat('png');
	$image->stripImage();

	if (!$image->writeImage($unachieved)) {
		throw new RuntimeException("Could not write $unachieved");
	}

	$image->destroy();

	echo "Saved: $unachieved (" . number_format(filesize($unachieved)) . " bytes)\n";
}

function generateAchievement(array $achievement, string $key, string $framePath): void {
	$slug = $achievement['slug'];
	$name = $achievement['name'];
	$description = $achievement['description'] ?? '';

	$achieved = __DIR__ . '/' . $slug . '_ACHIEVED.png';
	$unachieved = __DIR__ . '/' . $slug . '_UNACHIEVED.png';

	/*
	 * Existing images are never regenerated through OpenAI.
	 * Missing greyscale images can be reconstructed locally.
	 */
	if (is_file($achieved)) {
		echo "Skipping artwork for $slug (already exists)\n";

		if (!is_file($unachieved)) {
			generateUnachieved($achieved, $unachieved);
		}

		return;
	}

	echo "\nGenerating $slug: $name\n";

	$promptResponse = postOpenAI('chat/completions', [
		'model' => 'gpt-6.1-terra',
		'reasoning_effort' => 'none',
		'temperature' => 0.2,
		'messages' => [
			['role' => 'system', 'content' => <<<'INSTRUCTIONS'
You are the art director for The Seven Spells of Destruction, a dark medieval fantasy role-playing game.

Create a visually specific image-generation prompt for ONE Steam achievement icon.

You will receive the achievement name and description. Interpret their meaning and select ONE distinctive visual symbol that represents the accomplishment.

ART DIRECTION:

The artwork must belong to a coherent collection of premium dark fantasy achievement icons.

Style:
- Rich antique gold, bronze and black.
- Highly detailed, realistic fantasy illustration.
- Dramatic chiaroscuro lighting.
- Polished metallic highlights and subtle magical illumination.
- Deep black and charcoal backgrounds.
- Ancient medieval fantasy atmosphere.
- A prestigious, collectible, almost jewel-like appearance.
- Strong contrast and a clear central silhouette.

COMPOSITION:

1. The artwork will ultimately be displayed at 256x256 pixels.
2. Feature ONE dominant central object, creature, symbol or tightly grouped composition.
3. Keep the subject large, clear and immediately recognisable.
4. Place important details within the central 75% of the image.
5. Keep the outer edges dark and visually quiet.
6. Use a square composition.
7. Avoid elaborate backgrounds or distant scenery.
8. Do not create an ornamental border or frame. A separate gold frame will be composited over the finished artwork.
9. Do not include any text, letters, numbers, labels, typography, logos or watermarks.
10. Do not depict a generic adventurer unless the achievement specifically warrants one.
11. Avoid generic fantasy symbols when the achievement suggests something more distinctive.
12. Interpret humorous achievements visually, whilst preserving the serious fantasy art style.
13. The entire background must be fully opaque, black or near-black. Never generate transparency.

CONSISTENCY:

Every icon must appear to belong to the same collection.

Use the same gold-and-black palette, realistic metallic rendering, dramatic lighting and dark background treatment.

The subject must be distinctive, but the overall visual treatment must remain consistent.

OUTPUT:

Write ONE concise, visually specific image-generation prompt in English.

Describe the central subject, materials, composition, lighting and atmosphere.

Do not explain your reasoning.
Do not include a preamble.
Output ONLY the image-generation prompt.
INSTRUCTIONS],
			['role' => 'user', 'content' =>
				"Achievement: $name\nDescription: $description"
			],
		],
	], $key);

	$prompt = trim($promptResponse['choices'][0]['message']['content'] ?? '');

	if ($prompt === '') {
		throw new RuntimeException("No illustration prompt returned for $slug.");
	}

	echo "Prompt: $prompt\n";
	echo "Generating artwork...\n";

	$imageResponse = postOpenAI('images/generations', [
		'model' => 'gpt-image-1.5',
		'prompt' => $prompt . "\n\n"
			. "Create a single square dark fantasy achievement icon. "
			. "Use a predominantly black background with richly illuminated antique gold and bronze subjects. "
			. "The central subject must be large and recognisable at small sizes. "
			. "Keep the outer edges dark and uncluttered. "
			. "The background must be completely opaque, with no transparency. "
			. "Do not draw any frame, border, text, letters or numbers.",
		'size' => '1024x1024',
		'quality' => 'medium',
		'background' => 'opaque',
		'output_format' => 'png',
		'moderation' => 'low',
		'n' => 1,
	], $key);

	$encoded = $imageResponse['data'][0]['b64_json'] ?? null;

	if (!$encoded) {
		throw new RuntimeException("Image API returned no image for $slug.");
	}

	$png = base64_decode($encoded, true);

	if ($png === false || substr($png, 0, 8) !== "\x89PNG\r\n\x1a\n") {
		throw new RuntimeException("Invalid PNG returned for $slug.");
	}

	// Resize generated artwork.
	$image = new Imagick();
	$image->readImageBlob($png);
	$image->setImageFormat('png');
	$image->resizeImage(256, 256, Imagick::FILTER_LANCZOS, 1);

	// Overlay the gold frame.
	$frame = new Imagick($framePath);
	$frame->setImageFormat('png');

	$image->compositeImage($frame, Imagick::COMPOSITE_OVER, 0, 0);

	// Remove magenta from the completed composition.
	removeMagenta($image);

	// Save the achieved version.
	$image->stripImage();
	$image->setImageFormat('png');

	if (!$image->writeImage($achieved)) {
		throw new RuntimeException("Could not write $achieved");
	}

	echo "Saved: $achieved (" . number_format(filesize($achieved)) . " bytes)\n";

	$image->destroy();
	$frame->destroy();

	// Create the greyscale version from the completed artwork.
	generateUnachieved($achieved, $unachieved);
}

/*
 * Export the actual Steamworks bulk import CSV format.
 *
 * Definitions and localisation are separate CSV files.
 * Only enabled achievements with both PNGs are included.
 *
 * Both files are rebuilt after every run, including single-ID runs.
 */
function exportSteamCSV(mysqli $db): void {
	$result = $db->query(
		'SELECT id, slug, name, description, secret
		FROM achievements
		WHERE enabled = 1
		ORDER BY id'
	);

	$definitionsPath = __DIR__ . '/achievements-definitions.csv';
	$localizationPath = __DIR__ . '/achievements-localization.csv';

	$definitions = fopen($definitionsPath, 'wb');
	$localization = fopen($localizationPath, 'wb');

	if (!$definitions || !$localization) {
		if ($definitions) {
			fclose($definitions);
		}
		if ($localization) {
			fclose($localization);
		}
		throw new RuntimeException('Could not create Steam CSV files.');
	}

	/*
	 * Steamworks definitions:
	 * api_name, setid, permission, spoiler, archived,
	 * progress_stat_name, progress_stat_min, progress_stat_max
	 */
	fputcsv($definitions, [
		'api_name',
		'setid',
		'permission',
		'spoiler',
		'archived',
		'progress_stat_name',
		'progress_stat_min',
		'progress_stat_max',
	]);

	/*
	 * Steamworks localisation:
	 * One row for the name, one for the description.
	 */
	fputcsv($localization, [
		'api_name',
		'field',
		'english',
	]);

	$count = 0;

	while ($achievement = $result->fetch_assoc()) {
		$slug = $achievement['slug'];

		$achieved = __DIR__ . '/' . $slug . '_ACHIEVED.png';
		$unachieved = __DIR__ . '/' . $slug . '_UNACHIEVED.png';

		if (!is_file($achieved) || !is_file($unachieved)) {
			continue;
		}

		fputcsv($definitions, [
			$slug,
			'',                         // Base achievement set
			'OfficialGameServer',       // Server-controlled unlocks
			$achievement['secret'] ? 'true' : 'false',
			'false',                    // Not archived
			'',                         // No progress stat
			'',
			'',
		]);

		fputcsv($localization, [
			$slug,
			'name',
			$achievement['name'],
		]);

		fputcsv($localization, [
			$slug,
			'description',
			$achievement['description'] ?? '',
		]);

		$count++;
	}

	fclose($definitions);
	fclose($localization);

	echo "\nSteamworks CSV export complete.\n";
	echo "Achievements exported: $count\n";
	echo "Definitions: $definitionsPath\n";
	echo "Localization: $localizationPath\n";
}

try {
	if ($argv[1] === 'all') {
		$result = $db->query(
			'SELECT id, slug, name, description FROM achievements WHERE enabled = 1 ORDER BY id'
		);

		$achievements = $result->fetch_all(MYSQLI_ASSOC);
	} else {
		$id = (int) $argv[1];

		$stmt = $db->prepare(
			'SELECT id, slug, name, description FROM achievements WHERE id = ? LIMIT 1'
		);
		$stmt->bind_param('i', $id);
		$stmt->execute();

		$achievement = $stmt->get_result()->fetch_assoc();

		if (!$achievement) {
			throw new RuntimeException("Achievement $id not found.");
		}

		$achievements = [$achievement];
	}

	echo "Processing " . count($achievements) . " achievement(s).\n";

	$failed = 0;

	foreach ($achievements as $achievement) {
		try {
			generateAchievement($achievement, $config->gptkey, $framePath);
		} catch (Throwable $e) {
			fwrite(STDERR, "Error [{$achievement['slug']}]: {$e->getMessage()}\n");
			$failed++;
		}
	}

	// Always rebuild CSVs from successfully generated artwork.
	exportSteamCSV($db);

	echo "\nFinished. Failures: $failed\n";
	exit($failed > 0 ? 1 : 0);

} catch (Throwable $e) {
	fwrite(STDERR, "Error: {$e->getMessage()}\n");
	exit(1);
}
