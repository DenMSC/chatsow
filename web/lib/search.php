<?php

function search_redirect($index, $pager, $value) {
    global $hierarchy;
    if ($_POST['submit']) {
        $hierarchy[$pager->getIndex()] = '1';
        header('Location: http://' . $_SERVER['HTTP_HOST'] . url($value, $index, false));
        exit;
    }
}

function search_get($index) {
    global $db, $hierarchy;
    return $db->real_escape_string($hierarchy[$index]);
}

function get_order($index, $default = null) {
    global $db, $hierarchy;
    $order = $db->real_escape_string($hierarchy[$index]);
    if (isset($default)) {
        if ($order == '')
            $order = 'name';
        $hierarchy[$index] = $order;
    }
    $descending = substr($order, 0, 1) == '-';
    if ($descending)
        $order = substr($order, 1);
    return array($order, $descending);
}

function invert_search($level, $link) {
    global $hierarchy;
    $current = $hierarchy[$level];
    if ($current == $link)
        return '-' . $link;
    return $link;
}

function search_prefix($level, $link) {
    global $hierarchy;
    $current = $hierarchy[$level];
    if ($current == $link)
        return '+';
    if ($current == '-' . $link)
        return '-';
    return '';
}

?>
