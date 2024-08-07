<?php
require 'vendor/autoload.php';
$config = json_decode(file_get_contents("config.json"));

$files = glob("/tmp/.sentry_envelope_*.json");
sort($files);

$dsn = $config->sentry_dsn;
$environment = $config->environment;
$sampleRate = $config->sentry_sample_rate;
$dsn = parse_url($dsn);

foreach ($files as $envelope) {
    $content = file_get_contents($envelope);
    $postUrl = sprintf("%s://%s/api%s/envelope/?sentry_key=%s&sentry_version=7&sentry_client=sentry.native/7.77.0", $dsn["scheme"], $dsn["host"], $dsn["path"], $dsn["user"]);

    $curlHandle = curl_init($postUrl);
    curl_setopt($curlHandle, CURLOPT_POSTFIELDS, $content);
    curl_setopt($curlHandle, CURLOPT_RETURNTRANSFER, true);
    curl_setopt($curlHandle, CURLOPT_HTTPHEADER, ['Content-Type: application/json']);
    $curlResponse = curl_exec($curlHandle);
    curl_close($curlHandle);
    unlink($envelope);
}