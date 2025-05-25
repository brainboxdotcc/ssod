<?php

$config = json_decode(file_get_contents("config.json"));
$apiKey = $config->gptkey;
$inputFile = "lang.json";

if (!file_exists($inputFile)) {
    echo "File not found: $inputFile\n";
    exit(1);
}

$data = json_decode(file_get_contents($inputFile), true);
if (!$data) {
    echo "Invalid JSON.\n";
    exit(1);
}

$allLanguages = [];
foreach ($data as $entry) {
    if (is_array($entry) && count($entry) > 1) {
        $allLanguages = array_merge($allLanguages, array_keys($entry));
    }
}
$allLanguages = array_unique($allLanguages);
sort($allLanguages);

// Step 2: Identify keys with only 'en'
$toTranslate = [];
foreach ($data as $key => $entry) {
    if (is_array($entry) && count($entry) === 1 && isset($entry['en'])) {
        $toTranslate[$key] = $entry['en'];
    }
}

function isUntranslatableToken($text) {
    return preg_match('/<[@#&]?\d+>|:\w+:|https?:\/\/\S+/', $text);
}

function callChatGPT($apiKey, $prompt) {
    $ch = curl_init('https://api.openai.com/v1/chat/completions');
    $payload = json_encode([
        "model" => "gpt-4o",
        "messages" => [["role" => "user", "content" => $prompt]],
        "temperature" => 0.3
    ]);

    curl_setopt_array($ch, [
        CURLOPT_RETURNTRANSFER => true,
        CURLOPT_HTTPHEADER => [
            "Authorization: Bearer $apiKey",
            "Content-Type: application/json"
        ],
        CURLOPT_POSTFIELDS => $payload
    ]);

    $response = curl_exec($ch);
    if (curl_errno($ch)) {
        echo "Curl error: " . curl_error($ch) . "\n";
        return null;
    }

    $data = json_decode($response, true);
    if (empty($data['choices'])) {
        print_r($data);
    }
    return $data['choices'][0]['message']['content'] ?? null;
}

foreach ($toTranslate as $key => $english) {
    echo "Translating: $key\n";
    $prompt = "Translate the following English string into all of these languages: "
            . implode(', ', array_diff($allLanguages, ['en'])) . ".\n"
            . "Preserve placeholders like {0}, and do not translate tokens like <@12345> or :emoji:.\n\n"
            . "String: \"$english\"\n\n"
            . "Respond in JSON format mapping language codes to translations.";

    $response = callChatGPT($apiKey, $prompt);
    if (!$response) {
        echo "Failed to get translation for $key\n";
        continue;
    }

    $json = json_decode($response, true);
    if (!is_array($json)) {
        echo "Invalid response JSON for $key:\n$response\n";
        continue;
    }

    $data[$key] = ['en' => $english] + $json;
}

$outputFile = preg_replace('/\.json$/', '_new.json', $inputFile);
file_put_contents($outputFile, json_encode($data, JSON_PRETTY_PRINT | JSON_UNESCAPED_UNICODE));
echo "Translation complete. Output written to $outputFile\n";
