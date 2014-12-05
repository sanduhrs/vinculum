<?php
/**
 * Root directory of Drupal installation.
 */
define('DRUPAL_ROOT', '/var/www/vhosts/dev');

require_once DRUPAL_ROOT . '/includes/bootstrap.inc';
drupal_bootstrap(DRUPAL_BOOTSTRAP_DATABASE);

$examples = array();
$examples[] = array(
  'nid'       => 26,
  'url'       => 'http://example.com/phpnw_commentary',
  'title'     => 'Random musings on PHP NW',
  'excerpt'   => NULL,
  'handler'   => 'pingback',
  'origin_ip' => '192.168.10.1',
  'created' => time() - rand(0, 86400),
);
$examples[] = array(
  'nid'       => 26,
  'url'       => 'http://bar.example.com/php_nw_was_good',
  'title'     => 'Another great PHP NW',
  'excerpt'   => NULL,
  'handler'   => 'pingback',
  'origin_ip' => '192.168.10.2',
  'created' => time() - rand(0, 86400),
);
$examples[] = array(
  'nid'       => 21,
  'url'       => 'http://zeus.example.com/zeus_all_bad',
  'title'     => 'Is Zeus all bad?',
  'excerpt'   => NULL,
  'handler'   => 'pingback',
  'origin_ip' => '192.168.10.3',
  'created' => time() - rand(0, 86400),
);
$examples[] = array(
  'nid'       => 16,
  'url'       => 'http://clamav.example.com/clamming_it_up',
  'title'     => 'ClamAV for drupal',
  'excerpt'   => NULL,
  'handler'   => 'pingback',
  'origin_ip' => '192.168.10.4',
  'created' => time() - rand(0, 86400),
);


foreach ($examples as $example) {
  db_merge('node_vinculum_received')
    ->key(array(
      'nid' => $example['nid'],
      'url' => $example['url'],
      ))
    ->fields(array(
      'title'     => $example['title'],
      'excerpt'   => $example['excerpt'],
      'handler'   => $example['handler'],
      'origin_ip' => $example['origin_ip'],
      'created' => $example['created'],
    ))
    ->execute();
}
