<?php

import_lib('Pager');
import_lib('Table');
import_lib('Search');

$s['head'] = 'Maps';
$s['description'] = 'Racesow record demo listing for all recorded maps.';

$table = new Table(1);
$table->addColumn(array('name' => 'name', 'title' => 'Map', 'size' => 'large'));
$table->addColumn(array('name' => 'player', 'title' => 'Record holder', 'table' => 'P', 'column' => 'name_raw', 'size' => 'large'));
$table->addColumn(array('name' => 'record', 'title' => 'Record', 'align' => 'right', 'size' => 'small'));
$table->addColumn(array('name' => 'timestamp', 'title' => 'Date', 'align' => 'right', 'size' => 'medium'));

$table->processOrder('name');

$search = new Search(3);
$like = $search->get();

$pager = new Pager(2, $s['max_rows'], "P.`id`, M.`name`, `record`, P.`name` AS `record_holder`, UNIX_TIMESTAMP(`timestamp`) AS `timestamp` FROM `map` M, `player` P WHERE P.`id`=M.`player` AND (M.`name` LIKE '%$like%' OR P.`name_raw` LIKE '%$like%')" . $table->getOrder());

$search->redirect($pager);

$pager->query();
$rows = $pager->getRows();
foreach ($rows as $row) {
    $table->addField(format_map($row['name']));
    $table->addField(format_player($row['record_holder'], $row['id'], -1));
    $table->addField(format_time($row['record'], $row['name']));
    $table->addField(format_date($row['timestamp']));
}
$table->setPager($pager);
$table->setSearch($search);

$s['table'] = $table;

?>
