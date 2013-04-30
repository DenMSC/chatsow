<?php

import_lib('Pager');

$id = (int)$hierarchy[1];
$like = $db->real_escape_string($hierarchy[3]);

$pager = new Pager($hierarchy[2] - 1, $shared['max_rows'], "`name`, `record`, UNIX_TIMESTAMP(`timestamp`) AS `timestamp` FROM `map` WHERE `player`=$id AND `name` LIKE '%$like%' ORDER BY `name`");

$maps = '';
$rows = $pager->getRows();
foreach ($rows as $row) {
    $maps .= '<tr><td>' . format_map($row['name']) . '</td><td class="right">' . format_time($row['record'], $row['name']) . '</td><td class="right">' . exact_time($row['timestamp']) . '</td></tr>';
}

?>
<p>
Records below are the best runs recorded by the bot, not necessarily actual records.
</p>
<p>
<form action="<?= url('players/' . $hierarchy[1]); ?>" method="POST">
<input type="text" name="name" value="<?= $like; ?>" />
<input type="submit" name="submit" value="Search">
</form>
</p>
<?= format_pages(2, $pager); ?>
<table>
    <tr><th>Map</th><th class="right">Record</th><th class="right">Date</th></tr>
    <?= $maps; ?>
</table>
