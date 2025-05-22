<?php
$config = json_decode(file_get_contents("../config.json"));
$db = mysqli_connect($config->database->host, $config->database->username, $config->database->password, $config->database->database);
$books = json_decode(file_get_contents($argv[1]));
foreach ($books as &$book) {
    foreach ($book as $key => &$field) {
        if ($key === "tags") {
            $field = mysqli_real_escape_string($db, json_encode($field));
        } elseif (!is_array($field)) {
            $field = mysqli_real_escape_string($db, $field);
        } else {
            foreach ($field as &$element) {
                $element = mysqli_real_escape_string($db, $element);
            }
        }
    }
    echo $book->title . "\n";
}
mysqli_query($db, "START TRANSACTION");
unset($book);
foreach ($books as $book) {
    if (empty($book->title) || empty($book->tags)) {
        throw new Exception("Missing book title or tags!");
    }
    mysqli_query($db, "INSERT INTO books (title, author, tags) VALUES('" . $book->title . "', '" . ($book->author ?? 'Unknown') . "', '" . $book->tags . "')");
    $bookId = mysqli_insert_id($db);
    if (empty($book->pages)) {
        throw new Exception("Missing book pages for book id $bookId!");
    }
    foreach ($book->pages as $index => $page) {
        mysqli_query($db, "INSERT INTO book_pages (book_id, page_index, content) VALUES('" . $bookId . "', " . $index . ", '" . $page . "')");
    }
}
mysqli_query($db, "COMMIT");
