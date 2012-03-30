<?php
/**
 * @file
 */

/**
 * Implements hook_views_data_alter().
 */
function vinculum_views_data_alter(&$data) {
  // Switch the views-handler from the standard views_handler_field to a custom
  // handler which links the title/excerpt to the referring URL.
  if (!empty($data['node_vinculum_received']['title']['field'])) {
    $data['node_vinculum_received']['title']['field']['handler'] = 'vinculum_views_handler_field_refback_text';
  }
  if (!empty($data['node_vinculum_received']['excerpt']['field'])) {
    $data['node_vinculum_received']['excerpt']['field']['handler'] = 'vinculum_views_handler_field_refback_text';
  }
}
