<?php
/**
 * @file
 * Drush integration for vinculum.
 */

/**
 * Implements hook_drush_command().
 */
function vinculum_drush_command() {
  $items = array();
  $items['vinculum-clear-all-received'] = array(
    'callback' => 'vinculum__drush_clear_all_received',
    'description' => "Clear all received vinculums immediately.",
    //'arguments' => array(
    //  'nid' => "The node ID that the pingback comes from.",
    //  'target' => "The URL to send the pingback to.",
    //),
    'examples' => array(
      'drush vinculum-clear-all-received' => "Send a pingback from node 45 to http://example.com/blog/2011/11/09/foo.",
    ),
  );

  return $items;
}

function vinculum__drush_clear_all_received() {
	$results = entity_load('vinculum_received');
	$entity_ids = array_keys($results);
	entity_delete_multiple('vinculum_received', $entity_ids);
	print drush_log(dt('vinculums_received deleted'), 'ok', $type = 'notice');
	watchdog('vinculum', 'vinculums_received all deleted by drush', array()); 
}