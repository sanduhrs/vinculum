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
    'description' => "Clear all received Vinculums immediately.",
    'examples' => array(
      'drush vinculum-clear-all-received' => "Clear all received Vinculums immediately.",
    ),
  );
  return $items;
}

/**
 * Command callback.
 */
function vinculum__drush_clear_all_received() {
  $results = entity_load('vinculum_received');
  $entity_ids = array_keys($results);
  entity_delete_multiple('vinculum_received', $entity_ids);
  print drush_log(dt('vinculums_received deleted'), 'ok', $type = 'notice');
  watchdog('vinculum', 'vinculums_received all deleted by drush', array());
}
