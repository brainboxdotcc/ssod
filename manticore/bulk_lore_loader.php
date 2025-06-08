<?php

$host = '127.0.0.1';
$port = 9306; // Manticore default SQL port
$db = new mysqli($host, '', '', '', $port);

if ($db->connect_error) {
    die("Connection failed: " . $db->connect_error);
}

function loadMarkdownFiles($dir): array {
    $files = [];
    $iterator = new RecursiveIteratorIterator(new RecursiveDirectoryIterator($dir));
    foreach ($iterator as $file) {
        if ($file->isFile() && strtolower($file->getExtension()) === 'md') {
            $files[] = $file->getPathname();
        }
    }
    return $files;
}

function insertLore($db, $id, $content, $source) {
    // Escape single quotes for Manticore
    $content = $db->real_escape_string($content);
    $source = $db->real_escape_string($source);

    $sql = "REPLACE INTO lore (id, content, source) VALUES ($id, '$content', '$source')";
    if (!$db->query($sql)) {
        echo "Failed to insert $source: " . $db->error . "\n";
    }
}

$files = loadMarkdownFiles(__DIR__ . '/../resource');
$idCounter = 1;

foreach ($files as $filepath) {
    $markdown = file_get_contents($filepath);
    $chunks = preg_split('/\n\n+/', $markdown); // Split by paragraphs (loose)
    foreach ($chunks as $chunk) {
        $cleaned = trim($chunk);
        if (strlen($cleaned) >= 40) { // Skip tiny or meaningless entries
            insertLore($db, $idCounter++, $cleaned, basename($filepath));
        }
    }
}

echo "Import complete: $idCounter entries loaded.\n";

