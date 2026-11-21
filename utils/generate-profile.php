<?php
/*
 * Generate Seven Spells of Destruction character portraits.
 *
 * Usage:
 *   php generate-profile.php 8 2 f       # one portrait
 *   php generate-profile.php 8           # all 12 dark elf portraits
 *   php generate-profile.php all         # all 60 non-greenskin portraits
 *   php generate-profile.php 8 2 f --force  # replace an existing portrait
 *   php generate-profile.php all --dry-run  # print prompts, no API calls
 *
 * Requires ../config.json with gptkey, PHP cURL and Imagick.
 * Outputs <race>_<profession>_<f|m>.png in this script's directory.
 * Races 3 (Orc), 5 (Lesser Orc), 7 (Goblin) are intentionally excluded.
 */

if (PHP_SAPI !== 'cli' || !extension_loaded('curl') || !extension_loaded('imagick')) {
	fwrite(STDERR, "CLI PHP with cURL and Imagick is required.\n");
	exit(1);
}

$args = array_slice($argv, 1);
$force = in_array('--force', $args, true);
$dryRun = in_array('--dry-run', $args, true);
$args = array_values(array_filter($args, static fn($arg) => $arg !== '--force' && $arg !== '--dry-run'));

if (count($args) < 1 || count($args) > 3 ||
	($args[0] !== 'all' && !ctype_digit($args[0])) ||
	(count($args) >= 2 && !ctype_digit($args[1])) ||
	(count($args) === 3 && !in_array($args[2], ['f', 'm'], true)) ||
	($args[0] === 'all' && count($args) !== 1)) {
	fwrite(STDERR, "Usage: php generate-profile.php <all|race [profession [f|m]]> [--force] [--dry-run]\n");
	exit(1);
}

$races = [
	1 => ['Human', 'A human from a lived-in medieval society, with ordinary human proportions and a distinctive, unidealised face. Architecture and clothing are practical and modest; no modern details.'],
	2 => ['Elf', 'A woodland elf with naturally pointed ears, graceful but credible anatomy, and an individual, lived-in face. Native to established forest communities; organic craftsmanship and restrained decoration.'],
	4 => ['Dwarf', 'A sturdy, broad-featured dwarf with a compact build, strong hands and a practical, work-worn appearance. Dwarven craft traditions favour durable metal, leather, wool and stone.'],
	6 => ['Barbarian', 'A human barbarian from a rugged tribal culture, physically capable and weathered by outdoor life. Practical furs, woven cloth, leather and repaired equipment; no primitive caricatures.'],
	8 => ['Dark Elf', 'A Cryptillian dark elf of the Kin: a woodland people who live in secluded forest settlements and distrust outsiders. Pointed ears, individual facial structure and restrained woodland craftsmanship. NOT an underground dweller, D&D drow or spider worshipper. No subterranean architecture, mushroom groves or spider motifs.'],
];

/*
 * Every race/profession combination has its own scene and narrative action.
 * Each is combined with the race identity and gender to produce 60 prompts.
 */
$scenes = [
	1 => [
		1 => 'A guard-trained fighter tightening the straps of battered armour before setting out from a modest town gate; a plain sword and shield, evidence of previous hard service.',
		2 => 'A self-taught spellcaster examining a handwritten spellbook at a cluttered wooden desk, a small controlled light hovering over the page; ink-stained fingers.',
		3 => 'A streetwise thief discreetly examining a small stolen purse beside a busy market alley, blending into ordinary clothing rather than dressing as a theatrical rogue.',
		4 => 'A practical ranger studying animal tracks beside a stream at the edge of farmland and forest, bow unstrung or lowered; a weather-beaten cloak.',
		5 => 'An inconspicuous assassin checking a slim blade in a quiet courtyard before a difficult assignment; plain travelling clothes and a guarded expression.',
		6 => 'A veteran sellsword repairing a dented piece of armour beside a caravan camp, an old campaign sword within reach; visibly used equipment.',
	],
	2 => [
		1 => 'A woodland warrior inspecting the edge of a well-used elven shortsword at a forest watchpost; practical light armour and restrained craftsmanship.',
		2 => 'A forest scholar testing a small magical effect above an open manuscript in a timber-built study overlooking trees; controlled, scholarly magic.',
		3 => 'A deft thief quietly opening a merchant satchel beneath the awning of a woodland trading post; an alert, unremarkable appearance.',
		4 => 'A seasoned forest tracker watching a distant deer trail through layered branches, longbow held low; worn boots and travel-stained clothes.',
		5 => 'A disciplined assassin concealed among the ordinary activity of a forest settlement, checking a compact blade with a calm, watchful expression.',
		6 => 'An elven mercenary pausing to adjust battered travel armour at a roadside camp, evidence of service far beyond the home forests.',
	],
	4 => [
		1 => 'A dwarven fighter sharpening a battered hand axe in a stone-built guardroom, heavy practical armour and the marks of years of training.',
		2 => 'A dwarven wizard poring over a rune-inscribed rod and weathered notes in a lamplit workshop, restrained magical sparks among ordinary tools.',
		3 => 'A resourceful dwarven thief inspecting a complicated lock in a merchant storeroom, with a small toolkit and an expression of concentration.',
		4 => 'A dwarf woodsman reading tracks in a rocky pine woodland, a compact hunting bow and heavy weatherproof travelling clothes.',
		5 => 'A dwarven assassin quietly testing the balance of a short blade in a deserted passage, practical unremarkable clothing and a measured gaze.',
		6 => 'A scarred dwarven mercenary sitting beside a campfire while repairing a dented shield, sturdy boots and well-maintained but ageing equipment.',
	],
	6 => [
		1 => 'A hardened tribal warrior preparing a stone-headed axe before a difficult journey, standing beside a rough timber shelter in a windswept landscape.',
		2 => 'A tribal spellcaster carefully arranging carved tokens and an old staff beside a modest fire, focused on a small, believable ritual.',
		3 => 'A cunning traveller discreetly retrieving a hidden purse from beneath a rough cloak at a crowded trading camp, sharp eyes and worn clothing.',
		4 => 'A wilderness hunter examining fresh tracks in muddy ground, carrying a simple javelin and practical fur-lined travelling gear.',
		5 => 'A patient, experienced killer quietly checking a serrated hunting blade behind a temporary camp, no theatrical costume or exaggerated menace.',
		6 => 'A weather-beaten tribal mercenary binding a damaged forearm guard at a frontier encampment, a heavy war club beside them.',
	],
	8 => [
		1 => 'An elf warrior maintaining a serrated sabre at a guarded woodland settlement, alert to unfamiliar visitors; dignified practical armour, not villainous theatrics.',
		2 => 'An elf wizard studying an old manuscript and ivory sceptre in a quiet timber-built woodland study, contemplating forbidden knowledge; subtle magic, scholarly restraint.',
		3 => 'An elf thief quietly inspecting a small concealed blade and a merchant purse beside a forest-border trading path, watchful and opportunistic.',
		4 => 'An elf woodsman examining disturbed leaves and tracks along a secluded forest trail, yew longbow held low, familiar with every path through the trees.',
		5 => 'An elf assassin testing the edge of a narrow blade in a discreet woodland courtyard, composed and unsettling through restraint rather than theatrical darkness.',
		6 => 'A widely travelled elf mercenary repairing hardened leather armour at a mixed-company roadside camp, weary but capable, with signs of many former employers.',
	],
];

$professions = [1 => 'Warrior', 2 => 'Wizard', 3 => 'Thief', 4 => 'Woodsman', 5 => 'Assassin', 6 => 'Mercenary'];
$genderDescriptions = [
	'f' => 'An adult female character. Practical, appropriate clothing and armour, no sexualised styling, no glamour make-up.',
	'm' => 'An adult male character. Individual facial structure, practical grooming, no fashion-model styling.',
];

function makePrompt(int $race, int $profession, string $gender, array $races, array $scenes, array $professions, array $genderDescriptions): string {
	$identity = $races[$race];
	return implode("\n\n", [
		"SEVEN SPELLS OF DESTRUCTION — $identity[0] $professions[$profession], " . ($gender === 'f' ? 'female' : 'male') . '. One character, one scene.',
		$identity[1],
		$genderDescriptions[$gender],
		$scenes[$race][$profession],
		'ART DIRECTION: Grounded dark medieval fantasy photorealstic image with convincing physical materials. Character appears to inhabit a real working world. Natural asymmetry, distinctive nose, jaw and eye shapes, visible pores, fine lines, scars or weathering where appropriate. Different ages and facial structures across the collection. Believable hands, exactly five fingers on each visible hand, correct weapon grips and sensible anatomy. No Instagram face, beauty filters, waxy skin, airbrushed complexion, influencer posing, glamour photography, perfect symmetry, elaborate cosmetics or immaculate hair. No cosplay-photo aesthetic. No gratuitous cleavage or impractical armour.',
		'COMPOSITION: Cinematic landscape character composition, one adult character shown from approximately the waist or mid-thigh upwards, with hands and profession-specific action visible where possible. Three-quarter or candid angle, not a direct-to-camera headshot. Distinctive but restrained environmental storytelling. Subject and hands within the central 80% of the frame. Realistic soft directional lighting, varied natural colour, legible silhouette. No borders, frames, captions, text, logos or watermarks.',
		'FORMAT: Landscape image. Final delivery is 1456 by 816 pixels (approximately 16:9). Generation will be cropped vertically from a 3:2 canvas: keep the character face, hands, weapons and important narrative details away from the top and bottom 12% of the generated frame. Do not place important details at the edges.',
	]);
}

function postOpenAI(array $payload, string $key): array {
	$ch = curl_init('https://api.openai.com/v1/images/generations');
	curl_setopt_array($ch, [
		CURLOPT_POST => true,
		CURLOPT_RETURNTRANSFER => true,
		CURLOPT_TIMEOUT => 300,
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

$targets = [];
$selectedRaces = $args[0] === 'all' ? array_keys($races) : [(int) $args[0]];
foreach ($selectedRaces as $race) {
	if (!isset($races[$race])) {
		fwrite(STDERR, "Race $race is excluded or invalid. Valid races: 1, 2, 4, 6, 8.\n");
		exit(1);
	}
	$selectedProfessions = isset($args[1]) ? [(int) $args[1]] : array_keys($professions);
	foreach ($selectedProfessions as $profession) {
		if (!isset($professions[$profession])) {
			fwrite(STDERR, "Invalid profession $profession. Valid professions: 1-6.\n");
			exit(1);
		}
		foreach (isset($args[2]) ? [$args[2]] : ['f', 'm'] as $gender) {
			$targets[] = [$race, $profession, $gender];
		}
	}
}

$key = '';
if (!$dryRun) {
	$configPath = __DIR__ . '/../config.json';
	$config = is_file($configPath) ? json_decode(file_get_contents($configPath)) : null;
	$key = $config->gptkey ?? '';
	if ($key === '') {
		fwrite(STDERR, "Missing gptkey in ../config.json\n");
		exit(1);
	}
}

$failed = 0;
foreach ($targets as [$race, $profession, $gender]) {
	$filename = $race . "_" . $profession . "_" . $gender . ".png";
	$path = __DIR__ . '/' . $filename;
	if (!$dryRun && is_file($path) && !$force) {
		echo "Skipping $filename (exists)\n";
		continue;
	}
	$prompt = makePrompt($race, $profession, $gender, $races, $scenes, $professions, $genderDescriptions);
	if ($dryRun) {
		echo "\n===== $filename =====\n$prompt\n";
		continue;
	}
	try {
		echo "Generating $filename...\n";
		$response = postOpenAI([
			'model' => 'gpt-image-1.5',
			'prompt' => $prompt,
			'size' => '1536x1024',
			'quality' => 'medium',
			'background' => 'opaque',
			'output_format' => 'png',
			'moderation' => 'low',
			'n' => 1,
		], $key);
		$encoded = $response['data'][0]['b64_json'] ?? null;
		$png = is_string($encoded) ? base64_decode($encoded, true) : false;
		if ($png === false || substr($png, 0, 8) !== "\x89PNG\r\n\x1a\n") {
			throw new RuntimeException('Image API returned no valid PNG');
		}
		$image = new Imagick();
		$image->readImageBlob($png);
		/* Crop to exact target aspect ratio, then resize to original resolution. */
		$w = $image->getImageWidth();
		$h = $image->getImageHeight();
		$targetRatio = 1456 / 816;
		if ($w / $h > $targetRatio) {
			$cropW = (int) round($h * $targetRatio);
			$image->cropImage($cropW, $h, (int) floor(($w - $cropW) / 2), 0);
		} else {
			$cropH = (int) round($w / $targetRatio);
			$image->cropImage($w, $cropH, 0, (int) floor(($h - $cropH) / 2));
		}
		$image->setImagePage(0, 0, 0, 0);
		$image->resizeImage(1456, 816, Imagick::FILTER_LANCZOS, 1);
		$image->setImageFormat('png');
		$image->stripImage();
		/* Write to a temporary file first so failed writes cannot damage originals. */
		$tmp = tempnam(__DIR__, '.profile-');
		if ($tmp === false) {
			throw new RuntimeException('Could not create temporary file');
		}
		try {
			if (!$image->writeImage($tmp)) {
				throw new RuntimeException('Could not write PNG');
			}
			if (!rename($tmp, $path)) {
				throw new RuntimeException("Could not save $filename");
			}
		} finally {
			if (is_file($tmp)) {
				unlink($tmp);
			}
		}
		$image->clear();
		$image->destroy();
		echo "Saved $filename (1456x816, " . number_format(filesize($path)) . " bytes)\n";
	} catch (Throwable $e) {
		fwrite(STDERR, "Error [$filename]: {$e->getMessage()}\n");
		$failed++;
	}
}

echo "Finished. Portraits selected: " . count($targets) . ". Failures: $failed\n";
exit($failed ? 1 : 0);
