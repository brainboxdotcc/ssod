<?php
exit;
$config = json_decode(file_get_contents("../config.json"));
$db = mysqli_connect($config->database->host, $config->database->username, $config->database->password, $config->database->database);
$books = json_decode(file_get_contents("books/combined_books_enriched_tags.json"));
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
    echo $book->id . ": " . $book->title . "\n";
}
print_r($books);
mysqli_query($db, "START TRANSACTION");
unset($book);
foreach ($books as $book) {
    mysqli_query($db, "INSERT INTO books (id, title, author, tags) VALUES('" . $book->id . "', '" . $book->title . "', '" . $book->author . "', '" . $book->tags . "')");
    $bookId = mysqli_insert_id($db);
    foreach ($book->pages as $index => $page) {
        mysqli_query($db, "INSERT INTO book_pages (book_id, page_index, content) VALUES('" . $book->id . "', " . $index . ", '" . $page . "')");
    }
}
mysqli_query($db, "COMMIT");
